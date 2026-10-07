// Task 1: broadcast map -> odom -> base_link and move base_link to the 2D goal
// by changing only map -> odom.
//   T_map_base = T_map_odom * T_odom_base, and we want T_map_base = goal
//   so T_map_odom = T_map_goal * inverse(T_odom_base)
#include <chrono>
#include <memory>
#include <string>

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "rclcpp/rclcpp.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2/LinearMath/Transform.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"
#include "tf2_ros/transform_broadcaster.h"

using namespace std::chrono_literals;

class TfGoalLocalizer : public rclcpp::Node
{
public:
  TfGoalLocalizer()
  : Node("tf_goal_localizer")
  {
    // odom -> base_link never changes. Default is identity, the demo params use an offset
    const double x = declare_parameter("odom_base_x", 0.0);
    const double y = declare_parameter("odom_base_y", 0.0);
    const double yaw = declare_parameter("odom_base_yaw", 0.0);

    tf2::Quaternion q;
    q.setRPY(0.0, 0.0, yaw);
    t_odom_base_ = tf2::Transform(q, tf2::Vector3(x, y, 0.0));

    // no goal yet, so map and odom sit on top of each other
    t_map_odom_ = tf2::Transform::getIdentity();

    broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    goal_sub_ = create_subscription<geometry_msgs::msg::PoseStamped>(
      "/goal_pose", 10,
      std::bind(&TfGoalLocalizer::on_goal, this, std::placeholders::_1));

    timer_ = create_wall_timer(100ms, std::bind(&TfGoalLocalizer::on_timer, this));

    RCLCPP_INFO(
      get_logger(), "Publishing tf at 10 Hz, odom->base_link = (%.2f, %.2f, yaw %.2f)",
      x, y, yaw);
  }

private:
  void on_goal(const geometry_msgs::msg::PoseStamped::SharedPtr msg)
  {
    // we only know how to place a goal that is given in the map frame
    if (msg->header.frame_id != "map") {
      RCLCPP_WARN(
        get_logger(), "Goal frame_id is '%s', expected 'map'. Ignoring it.",
        msg->header.frame_id.c_str());
      return;
    }

    tf2::Transform t_map_goal;
    tf2::fromMsg(msg->pose, t_map_goal);

    // odom -> base_link stays as it is, only map -> odom moves
    t_map_odom_ = t_map_goal * t_odom_base_.inverse();

    RCLCPP_INFO(
      get_logger(), "New goal (%.2f, %.2f), map->odom updated",
      msg->pose.position.x, msg->pose.position.y);
  }

  void on_timer()
  {
    const auto stamp = now();
    broadcaster_->sendTransform(
      {to_msg(t_map_odom_, "map", "odom", stamp),
        to_msg(t_odom_base_, "odom", "base_link", stamp)});
  }

  // frame_id is the parent, child_frame_id is the child
  static geometry_msgs::msg::TransformStamped to_msg(
    const tf2::Transform & t, const std::string & parent, const std::string & child,
    const rclcpp::Time & stamp)
  {
    geometry_msgs::msg::TransformStamped msg;
    msg.header.stamp = stamp;
    msg.header.frame_id = parent;
    msg.child_frame_id = child;
    msg.transform = tf2::toMsg(t);
    return msg;
  }

  tf2::Transform t_map_odom_;
  tf2::Transform t_odom_base_;

  std::unique_ptr<tf2_ros::TransformBroadcaster> broadcaster_;
  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr goal_sub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TfGoalLocalizer>());
  rclcpp::shutdown();
  return 0;
}
