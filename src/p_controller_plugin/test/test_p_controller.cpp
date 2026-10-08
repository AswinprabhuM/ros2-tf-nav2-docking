// Loads PController through pluginlib (the same way controller_server does) and
// checks its output on a straight path along the x axis.
#include <gtest/gtest.h>

#include <cmath>
#include <memory>

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav2_core/controller.hpp"
#include "nav_msgs/msg/path.hpp"
#include "pluginlib/class_loader.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"
#include "tf2_ros/buffer.h"

namespace
{

geometry_msgs::msg::PoseStamped makePose(double x, double y, double yaw)
{
  geometry_msgs::msg::PoseStamped p;
  p.header.frame_id = "map";
  p.pose.position.x = x;
  p.pose.position.y = y;
  tf2::Quaternion q;
  q.setRPY(0.0, 0.0, yaw);
  p.pose.orientation = tf2::toMsg(q);
  return p;
}

const geometry_msgs::msg::Twist kNoVelocity;  // current velocity is unused by the controller

// Waypoints (0,0), (0.5,0) ... (2,0)
nav_msgs::msg::Path straightPath()
{
  nav_msgs::msg::Path path;
  path.header.frame_id = "map";
  for (int i = 0; i <= 4; ++i) {
    path.poses.push_back(makePose(0.5 * i, 0.0, 0.0));
  }
  return path;
}

// tf buffer with a fixed map -> odom transform (odom origin is at x = 10 in map)
std::shared_ptr<tf2_ros::Buffer> makeTfBuffer(rclcpp::Clock::SharedPtr clock)
{
  auto buffer = std::make_shared<tf2_ros::Buffer>(clock);
  geometry_msgs::msg::TransformStamped map_to_odom;
  map_to_odom.header.frame_id = "map";
  map_to_odom.child_frame_id = "odom";
  map_to_odom.transform.translation.x = 10.0;
  map_to_odom.transform.rotation.w = 1.0;
  buffer->setTransform(map_to_odom, "test", true);
  return buffer;
}

}  // namespace

class PControllerTest : public ::testing::Test
{
protected:
  static void SetUpTestSuite() {rclcpp::init(0, nullptr);}
  static void TearDownTestSuite() {rclcpp::shutdown();}

  void SetUp() override
  {
    node_ = std::make_shared<rclcpp_lifecycle::LifecycleNode>("test_node");
    node_->declare_parameter("FollowPath.kp", 2.0);
    node_->declare_parameter("FollowPath.linear_vel", 0.3);

    loader_ = std::make_unique<pluginlib::ClassLoader<nav2_core::Controller>>(
      "nav2_core", "nav2_core::Controller");
    controller_ = loader_->createUniqueInstance("p_controller_plugin::PController");

    tf_ = makeTfBuffer(node_->get_clock());
    controller_->configure(node_, "FollowPath", tf_, nullptr);
    controller_->activate();
  }

  void TearDown() override
  {
    controller_->deactivate();
    controller_->cleanup();
    controller_.reset();  // destroy the plugin before the loader unloads its library
  }

  rclcpp_lifecycle::LifecycleNode::SharedPtr node_;
  std::shared_ptr<tf2_ros::Buffer> tf_;
  std::unique_ptr<pluginlib::ClassLoader<nav2_core::Controller>> loader_;
  pluginlib::UniquePtr<nav2_core::Controller> controller_;
};

TEST_F(PControllerTest, DrivesStraightWhenAligned)
{
  controller_->setPlan(straightPath());
  auto cmd = controller_->computeVelocityCommands(makePose(0.1, 0.0, 0.0), kNoVelocity, nullptr);
  EXPECT_NEAR(cmd.twist.linear.x, 0.3, 1e-9);
  EXPECT_NEAR(cmd.twist.angular.z, 0.0, 1e-9);
}

TEST_F(PControllerTest, SteersBackTowardsPath)
{
  controller_->setPlan(straightPath());
  // Robot is on the path but yawed +0.2 rad: it must turn right (negative z) by kp*0.2.
  auto cmd = controller_->computeVelocityCommands(makePose(0.1, 0.0, 0.2), kNoVelocity, nullptr);
  EXPECT_NEAR(cmd.twist.angular.z, -2.0 * 0.2, 1e-6);
}

TEST_F(PControllerTest, AngularVelocityIsClamped)
{
  controller_->setPlan(straightPath());
  // Facing away from the target: error ~ pi, kp*error = ~6.3, clamp is 1.0 (default).
  auto cmd = controller_->computeVelocityCommands(makePose(0.1, 0.0, M_PI - 0.1), kNoVelocity,
    nullptr);
  EXPECT_NEAR(std::abs(cmd.twist.angular.z), 1.0, 1e-9);
}

TEST_F(PControllerTest, SkipsVisitedWaypoints)
{
  controller_->setPlan(straightPath());
  // Drive along the path so waypoints (0,0) and (0.5,0) get visited.
  controller_->computeVelocityCommands(makePose(0.0, 0.0, 0.0), kNoVelocity, nullptr);
  controller_->computeVelocityCommands(makePose(0.5, 0.0, 0.0), kNoVelocity, nullptr);
  // Now the target is (1.0, 0). Robot is 0.2 above the path, facing +x: turn right.
  auto cmd = controller_->computeVelocityCommands(makePose(0.7, 0.2, 0.0), kNoVelocity, nullptr);
  EXPECT_LT(cmd.twist.angular.z, 0.0);
}

TEST_F(PControllerTest, StopsWhenPlanFinished)
{
  controller_->setPlan(straightPath());
  geometry_msgs::msg::TwistStamped cmd;
  for (int i = 0; i <= 4; ++i) {  // visit every waypoint in order
    cmd = controller_->computeVelocityCommands(
      makePose(0.5 * i, 0.0, 0.0), kNoVelocity, nullptr);
  }
  EXPECT_EQ(cmd.twist.linear.x, 0.0);
  EXPECT_EQ(cmd.twist.angular.z, 0.0);
}

TEST_F(PControllerTest, SpeedLimitScalesLinearVelocity)
{
  controller_->setPlan(straightPath());
  controller_->setSpeedLimit(50.0, true);  // 50 %
  auto cmd = controller_->computeVelocityCommands(makePose(0.1, 0.0, 0.0), kNoVelocity, nullptr);
  EXPECT_NEAR(cmd.twist.linear.x, 0.15, 1e-9);
}

// Regression test: the controller server gives the pose in odom while the plan is in map.
TEST_F(PControllerTest, TransformsPlanIntoPoseFrame)
{
  controller_->setPlan(straightPath());  // map frame, x = 0 ... 2
  // The map->odom offset above puts the first waypoint (map 0,0) at odom (-10, 0).
  // Robot is 1 m above it, facing +x, so it has to turn right: clamped to -1.0.
  auto pose = makePose(-10.0, 1.0, 0.0);
  pose.header.frame_id = "odom";
  auto cmd = controller_->computeVelocityCommands(pose, kNoVelocity, nullptr);
  EXPECT_NEAR(cmd.twist.angular.z, -1.0, 1e-9);
  EXPECT_EQ(cmd.header.frame_id, "odom");
}
