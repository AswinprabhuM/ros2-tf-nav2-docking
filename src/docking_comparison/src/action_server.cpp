// STUB — Task 3: action_server will be implemented here.
#include "rclcpp/rclcpp.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  RCLCPP_INFO(rclcpp::get_logger("action_server"), "action_server stub (TODO: implement)");
  rclcpp::shutdown();
  return 0;
}
