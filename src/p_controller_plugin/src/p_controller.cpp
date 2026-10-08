#include "p_controller_plugin/p_controller.hpp"

#include <algorithm>
#include <cmath>

#include "nav2_core/controller_exceptions.hpp"
#include "nav2_costmap_2d/costmap_2d_ros.hpp"
#include "nav2_util/node_utils.hpp"
#include "pluginlib/class_list_macros.hpp"
#include "tf2/utils.h"  // tf2::getYaw

namespace p_controller_plugin
{

void PController::configure(
  const rclcpp_lifecycle::LifecycleNode::WeakPtr & parent,
  std::string name, std::shared_ptr<tf2_ros::Buffer> tf,
  std::shared_ptr<nav2_costmap_2d::Costmap2DROS>)
{
  node_ = parent;
  tf_ = tf;
  auto node = node_.lock();
  if (!node) {
    throw std::runtime_error("PController: unable to lock the parent lifecycle node");
  }

  plugin_name_ = name;
  logger_ = node->get_logger();
  clock_ = node->get_clock();

  // Declare parameters namespaced by the plugin name, e.g. FollowPath.kp
  nav2_util::declare_parameter_if_not_declared(
    node, plugin_name_ + ".kp", rclcpp::ParameterValue(1.0));
  nav2_util::declare_parameter_if_not_declared(
    node, plugin_name_ + ".linear_vel", rclcpp::ParameterValue(0.2));
  nav2_util::declare_parameter_if_not_declared(
    node, plugin_name_ + ".max_angular_vel", rclcpp::ParameterValue(1.0));
  nav2_util::declare_parameter_if_not_declared(
    node, plugin_name_ + ".waypoint_tolerance", rclcpp::ParameterValue(0.15));

  node->get_parameter(plugin_name_ + ".kp", kp_);
  node->get_parameter(plugin_name_ + ".linear_vel", linear_vel_);
  node->get_parameter(plugin_name_ + ".max_angular_vel", max_angular_vel_);
  node->get_parameter(plugin_name_ + ".waypoint_tolerance", waypoint_tolerance_);

  RCLCPP_INFO(
    logger_, "%s configured: kp=%.2f linear_vel=%.2f max_angular_vel=%.2f tol=%.2f",
    plugin_name_.c_str(), kp_, linear_vel_, max_angular_vel_, waypoint_tolerance_);
}

void PController::cleanup()
{
  RCLCPP_INFO(logger_, "%s cleaning up", plugin_name_.c_str());
  tf_.reset();
  plan_ = nav_msgs::msg::Path();
  target_index_ = 0;
}

void PController::activate()
{
  RCLCPP_INFO(logger_, "%s activating", plugin_name_.c_str());
}

void PController::deactivate()
{
  RCLCPP_INFO(logger_, "%s deactivating", plugin_name_.c_str());
}

void PController::setPlan(const nav_msgs::msg::Path & path)
{
  plan_ = path;
  target_index_ = 0;  // new plan: start again from its first waypoint
}

void PController::setSpeedLimit(const double & speed_limit, const bool & percentage)
{
  // 0.0 (nav2_costmap_2d::NO_SPEED_LIMIT) means "no limit"
  if (speed_limit <= 0.0) {
    speed_scale_ = 1.0;
  } else if (percentage) {
    speed_scale_ = std::min(speed_limit / 100.0, 1.0);
  } else {
    speed_scale_ = std::min(speed_limit / linear_vel_, 1.0);
  }
}

geometry_msgs::msg::TwistStamped PController::computeVelocityCommands(
  const geometry_msgs::msg::PoseStamped & pose, const geometry_msgs::msg::Twist &,
  nav2_core::GoalChecker *)
{
  geometry_msgs::msg::TwistStamped cmd;  // all zeros by default
  cmd.header.frame_id = pose.header.frame_id;
  cmd.header.stamp = clock_->now();

  // Skip waypoints we are already close to, the first one left is the target.
  // Waypoints are converted to the pose's frame first (see toFrame).
  const double x = pose.pose.position.x;
  const double y = pose.pose.position.y;
  geometry_msgs::msg::PoseStamped target;
  while (target_index_ < plan_.poses.size()) {
    target = toFrame(plan_.poses[target_index_], pose.header.frame_id);
    if (std::hypot(target.pose.position.x - x, target.pose.position.y - y) >
      waypoint_tolerance_)
    {
      break;
    }
    ++target_index_;
  }

  // Plan finished (or empty): stop.
  if (target_index_ >= plan_.poses.size()) {
    return cmd;
  }

  // Heading error = bearing to the target - current yaw, wrapped to [-pi, pi].
  const double bearing = std::atan2(
    target.pose.position.y - y, target.pose.position.x - x);
  const double yaw = tf2::getYaw(pose.pose.orientation);
  const double error = std::atan2(std::sin(bearing - yaw), std::cos(bearing - yaw));

  cmd.twist.linear.x = linear_vel_ * speed_scale_;
  cmd.twist.angular.z = std::clamp(kp_ * error, -max_angular_vel_, max_angular_vel_);
  return cmd;
}

geometry_msgs::msg::PoseStamped PController::toFrame(
  const geometry_msgs::msg::PoseStamped & in, const std::string & frame) const
{
  if (in.header.frame_id == frame) {
    return in;
  }
  // plan is in map, the robot pose is in odom, so ask tf for the waypoint in the pose's frame
  geometry_msgs::msg::PoseStamped query = in;
  query.header.stamp = rclcpp::Time(0);  // 0 = latest available transform
  geometry_msgs::msg::PoseStamped out;
  try {
    tf_->transform(query, out, frame, tf2::durationFromSec(0.1));
  } catch (const tf2::TransformException & ex) {
    throw nav2_core::ControllerTFError(
      "PController: cannot transform " + in.header.frame_id + " -> " + frame + ": " + ex.what());
  }
  return out;
}

}  // namespace p_controller_plugin

PLUGINLIB_EXPORT_CLASS(p_controller_plugin::PController, nav2_core::Controller)
