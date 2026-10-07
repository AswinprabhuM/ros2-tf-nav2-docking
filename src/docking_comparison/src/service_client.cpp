// STUB — Task 3: service_client will be implemented here.
#include "rclcpp/rclcpp.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  RCLCPP_INFO(rclcpp::get_logger("service_client"), "service_client stub (TODO: implement)");
  rclcpp::shutdown();
  return 0;
}
