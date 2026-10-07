// STUB — Task 2. Signatures written for Jazzy; verify against
// /opt/ros/jazzy/include/nav2_core/nav2_core/controller.hpp once Nav2 is installed.
#ifndef P_CONTROLLER_PLUGIN__P_CONTROLLER_HPP_
#define P_CONTROLLER_PLUGIN__P_CONTROLLER_HPP_

#include <memory>
#include <string>

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "nav2_core/controller.hpp"
#include "nav_msgs/msg/path.hpp"

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
};

}  // namespace p_controller_plugin

#endif  // P_CONTROLLER_PLUGIN__P_CONTROLLER_HPP_
