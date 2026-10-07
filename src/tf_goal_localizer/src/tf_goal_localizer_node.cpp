// STUB — Task 1 will be implemented here.
#include <memory>

#include "rclcpp/rclcpp.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rclcpp::Node>("tf_goal_localizer");
  RCLCPP_INFO(node->get_logger(), "tf_goal_localizer stub started (TODO: implement)");
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
