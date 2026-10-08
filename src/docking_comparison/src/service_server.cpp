// Version A: service server. The "trip" is a 30 s sleep inside the callback.
#include <chrono>
#include <memory>
#include <thread>

#include "docking_interfaces/srv/go_to_charger.hpp"
#include "rclcpp/rclcpp.hpp"

using GoToCharger = docking_interfaces::srv::GoToCharger;

class DockingServiceServer : public rclcpp::Node
{
public:
  DockingServiceServer()
  : Node("docking_service_server")
  {
    service_ = create_service<GoToCharger>(
      "go_to_charger",
      std::bind(&DockingServiceServer::handle, this, std::placeholders::_1, std::placeholders::_2));
    RCLCPP_INFO(get_logger(), "Service server ready");
  }

private:
  void handle(
    const std::shared_ptr<GoToCharger::Request> request,
    std::shared_ptr<GoToCharger::Response> response)
  {
    RCLCPP_INFO(get_logger(), "Request received: dock at '%s', driving for 30 s...",
      request->charger_id.c_str());

    // The callback blocks here, so this server can't do anything else for 30 s.
    std::this_thread::sleep_for(std::chrono::seconds(30));

    response->success = true;
    response->message = "Arrived at charging station!";
    response->travel_time = 30.0;
    RCLCPP_INFO(get_logger(), "Arrived, sending response (the client may have given up already)");
  }

  rclcpp::Service<GoToCharger>::SharedPtr service_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DockingServiceServer>());
  rclcpp::shutdown();
  return 0;
}
