// Version B: action server. 30 s trip, feedback every second, cancel supported.
#include <chrono>
#include <functional>
#include <memory>
#include <thread>

#include "docking_interfaces/action/go_to_charger.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

using GoToCharger = docking_interfaces::action::GoToCharger;
using GoalHandle = rclcpp_action::ServerGoalHandle<GoToCharger>;

class DockingActionServer : public rclcpp::Node
{
public:
  DockingActionServer()
  : Node("docking_action_server")
  {
    using namespace std::placeholders;
    server_ = rclcpp_action::create_server<GoToCharger>(
      this, "go_to_charger",
      std::bind(&DockingActionServer::handleGoal, this, _1, _2),
      std::bind(&DockingActionServer::handleCancel, this, _1),
      std::bind(&DockingActionServer::handleAccepted, this, _1));
    RCLCPP_INFO(get_logger(), "Action server ready");
  }

private:
  // 60 m at 2 m/s = 30 s
  static constexpr int kTravelSeconds = 30;
  static constexpr float kSpeed = 2.0;
  static constexpr float kStartDistance = kSpeed * kTravelSeconds;

  rclcpp_action::GoalResponse handleGoal(
    const rclcpp_action::GoalUUID &, std::shared_ptr<const GoToCharger::Goal> goal)
  {
    RCLCPP_INFO(get_logger(), "Goal received: dock at '%s'", goal->charger_id.c_str());
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  rclcpp_action::CancelResponse handleCancel(const std::shared_ptr<GoalHandle>)
  {
    RCLCPP_INFO(get_logger(), "Cancel requested");
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  void handleAccepted(const std::shared_ptr<GoalHandle> goal_handle)
  {
    // execute() sleeps, so run it in its own thread and keep the executor free
    std::thread{std::bind(&DockingActionServer::execute, this, goal_handle)}.detach();
  }

  void execute(const std::shared_ptr<GoalHandle> goal_handle)
  {
    auto feedback = std::make_shared<GoToCharger::Feedback>();
    auto result = std::make_shared<GoToCharger::Result>();

    for (int second = 1; second <= kTravelSeconds; ++second) {
      if (goal_handle->is_canceling()) {
        result->success = false;
        result->message = "Docking canceled";
        result->travel_time = static_cast<float>(second - 1);
        goal_handle->canceled(result);
        RCLCPP_INFO(get_logger(), "Goal canceled after %d s", second - 1);
        return;
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
      feedback->distance_remaining = kStartDistance - kSpeed * second;
      goal_handle->publish_feedback(feedback);
    }

    result->success = true;
    result->message = "Arrived at charging station!";
    result->travel_time = static_cast<float>(kTravelSeconds);
    goal_handle->succeed(result);
    RCLCPP_INFO(get_logger(), "Goal succeeded");
  }

  rclcpp_action::Server<GoToCharger>::SharedPtr server_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DockingActionServer>());
  rclcpp::shutdown();
  return 0;
}
