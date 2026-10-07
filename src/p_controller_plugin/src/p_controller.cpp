// STUB — Task 2 will be implemented here.
#include "p_controller_plugin/p_controller.hpp"

#include "pluginlib/class_list_macros.hpp"

namespace p_controller_plugin
{

void PController::configure(
  const rclcpp_lifecycle::LifecycleNode::WeakPtr &, std::string,
  std::shared_ptr<tf2_ros::Buffer>, std::shared_ptr<nav2_costmap_2d::Costmap2DROS>)
{
}

void PController::cleanup() {}
void PController::activate() {}
void PController::deactivate() {}

geometry_msgs::msg::TwistStamped PController::computeVelocityCommands(
  const geometry_msgs::msg::PoseStamped &, const geometry_msgs::msg::Twist &,
  nav2_core::GoalChecker *)
{
  return geometry_msgs::msg::TwistStamped();  // zero velocity
}

void PController::setPlan(const nav_msgs::msg::Path &) {}
void PController::setSpeedLimit(const double &, const bool &) {}

}  // namespace p_controller_plugin

PLUGINLIB_EXPORT_CLASS(p_controller_plugin::PController, nav2_core::Controller)
