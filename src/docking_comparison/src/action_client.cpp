// STUB — Task 3: action_client will be implemented here.
#include "rclcpp/rclcpp.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  RCLCPP_INFO(rclcpp::get_logger("action_client"), "action_client stub (TODO: implement)");
  rclcpp::shutdown();
  return 0;
}
