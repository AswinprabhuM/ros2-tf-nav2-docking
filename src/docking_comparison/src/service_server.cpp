// STUB — Task 3: service_server will be implemented here.
#include "rclcpp/rclcpp.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  RCLCPP_INFO(rclcpp::get_logger("service_server"), "service_server stub (TODO: implement)");
  rclcpp::shutdown();
  return 0;
}
