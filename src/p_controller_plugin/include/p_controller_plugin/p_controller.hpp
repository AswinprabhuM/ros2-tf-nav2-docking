// Simple proportional (P) heading controller, written as a Nav2 controller plugin.
//
// Strategy: drive at a constant linear speed towards the nearest unvisited waypoint
// of the plan and steer with  angular_vel = kp * heading_error  (clamped).
// When every waypoint has been visited the controller outputs zero velocity.
#ifndef P_CONTROLLER_PLUGIN__P_CONTROLLER_HPP_
#define P_CONTROLLER_PLUGIN__P_CONTROLLER_HPP_

#include <cstddef>
#include <memory>
#include <string>

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "nav2_core/controller.hpp"
#include "nav_msgs/msg/path.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"
#include "tf2_ros/buffer.h"

namespace p_controller_plugin
{

class PController : public nav2_core::Controller
{
public:
  PController() = default;
  ~PController() override = default;

  void configure(
    const rclcpp_lifecycle::LifecycleNode::WeakPtr & parent,
    std::string name, std::shared_ptr<tf2_ros::Buffer> tf,
    std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros) override;
  void cleanup() override;
  void activate() override;
  void deactivate() override;

  geometry_msgs::msg::TwistStamped computeVelocityCommands(
    const geometry_msgs::msg::PoseStamped & pose,
    const geometry_msgs::msg::Twist & velocity,
    nav2_core::GoalChecker * goal_checker) override;

  void setPlan(const nav_msgs::msg::Path & path) override;
  void setSpeedLimit(const double & speed_limit, const bool & percentage) override;

private:
  // Returns `in` expressed in `frame`; throws nav2_core::ControllerTFError on failure.
  geometry_msgs::msg::PoseStamped toFrame(
    const geometry_msgs::msg::PoseStamped & in, const std::string & frame) const;

  // weak_ptr because the controller server owns the node, we shouldn't keep it alive.
  rclcpp_lifecycle::LifecycleNode::WeakPtr node_;
  rclcpp::Logger logger_{rclcpp::get_logger("PController")};
  rclcpp::Clock::SharedPtr clock_;
  std::shared_ptr<tf2_ros::Buffer> tf_;
  std::string plugin_name_;

  nav_msgs::msg::Path plan_;
  std::size_t target_index_{0};  // first waypoint that has not been visited yet

  // Parameters (all under <plugin_name>.)
  double kp_{1.0};
  double linear_vel_{0.2};
  double max_angular_vel_{1.0};
  double waypoint_tolerance_{0.15};

  double speed_scale_{1.0};  // set by setSpeedLimit(), multiplies linear_vel_
};

}  // namespace p_controller_plugin

#endif  // P_CONTROLLER_PLUGIN__P_CONTROLLER_HPP_
