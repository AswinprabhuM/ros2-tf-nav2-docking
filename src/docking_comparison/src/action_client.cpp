// Version B: action client. Prints all feedback and the final result.
// Run with the argument "cancel" to cancel the goal after 5 s.
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

#include "docking_interfaces/action/go_to_charger.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

using namespace std::chrono_literals;
using GoToCharger = docking_interfaces::action::GoToCharger;
using GoalHandle = rclcpp_action::ClientGoalHandle<GoToCharger>;

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  std::cout << std::fixed << std::setprecision(1);
  auto node = std::make_shared<rclcpp::Node>("docking_action_client");
  auto client = rclcpp_action::create_client<GoToCharger>(node, "go_to_charger");
  const bool cancel_after_5s = (argc > 1 && std::string(argv[1]) == "cancel");

  if (!client->wait_for_action_server(10s)) {
    std::cout << "[client] Action server not available" << std::endl;
    return 1;
  }

  auto goal = GoToCharger::Goal();
  goal.charger_id = "charger_1";

  auto options = rclcpp_action::Client<GoToCharger>::SendGoalOptions();
  options.feedback_callback =
    [](GoalHandle::SharedPtr, const std::shared_ptr<const GoToCharger::Feedback> feedback) {
      std::cout << "[feedback] Distance remaining: " << feedback->distance_remaining << " m"
                << std::endl;
    };

  std::cout << "[client] Requesting charging station trip via ACTION..." << std::endl;
  auto goal_future = client->async_send_goal(goal, options);
  if (rclcpp::spin_until_future_complete(node, goal_future) != rclcpp::FutureReturnCode::SUCCESS) {
    std::cout << "[client] Failed to send goal" << std::endl;
    return 1;
  }
  auto goal_handle = goal_future.get();
  if (!goal_handle) {
    std::cout << "[client] Goal was rejected" << std::endl;
    return 1;
  }
  std::cout << "[client] Goal accepted - receiving feedback..." << std::endl;

  auto result_future = client->async_get_result(goal_handle);

  if (cancel_after_5s) {
    // wait 5 s for the result; if it isn't there yet, cancel the goal
    if (rclcpp::spin_until_future_complete(node, result_future, 5s) !=
      rclcpp::FutureReturnCode::SUCCESS)
    {
      std::cout << "[client] Canceling the goal..." << std::endl;
      client->async_cancel_goal(goal_handle);
    }
  }
  rclcpp::spin_until_future_complete(node, result_future);

  auto wrapped = result_future.get();
  if (wrapped.code == rclcpp_action::ResultCode::SUCCEEDED) {
    std::cout << "[client] Result: " << wrapped.result->message
              << " Travel time: " << wrapped.result->travel_time << "s" << std::endl;
  } else if (wrapped.code == rclcpp_action::ResultCode::CANCELED) {
    std::cout << "[client] Goal canceled after " << wrapped.result->travel_time << "s"
              << std::endl;
  } else {
    std::cout << "[client] Goal failed" << std::endl;
  }

  rclcpp::shutdown();
  return 0;
}
