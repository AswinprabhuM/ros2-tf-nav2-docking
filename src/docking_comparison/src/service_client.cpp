// Version A: service client with a 5 s timeout.
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>

#include "docking_interfaces/srv/go_to_charger.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;
using GoToCharger = docking_interfaces::srv::GoToCharger;

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  std::cout << std::fixed << std::setprecision(1);
  auto node = std::make_shared<rclcpp::Node>("docking_service_client");
  auto client = node->create_client<GoToCharger>("go_to_charger");

  while (!client->wait_for_service(1s)) {
    if (!rclcpp::ok()) {
      return 1;
    }
    std::cout << "[client] Waiting for the service..." << std::endl;
  }

  auto request = std::make_shared<GoToCharger::Request>();
  request->charger_id = "charger_1";

  std::cout << "[client] Requesting charging station trip via SERVICE..." << std::endl;
  auto pending = client->async_send_request(request);

  std::cout << "[client] Waiting for response (5s timeout)..." << std::endl;
  auto status = rclcpp::spin_until_future_complete(node, pending.future, 5s);

  if (status == rclcpp::FutureReturnCode::SUCCESS) {
    auto response = pending.future.get();
    std::cout << "[client] Result: " << response->message
              << " Travel time: " << response->travel_time << "s" << std::endl;
  } else {
    std::cout << "[client] TIMED OUT - service did not respond within 5 seconds!" << std::endl;
    // We stop waiting, but the server doesn't know: it keeps driving and its
    // late response is dropped because nobody is waiting for it anymore.
    client->remove_pending_request(pending.request_id);
  }

  rclcpp::shutdown();
  return 0;
}
