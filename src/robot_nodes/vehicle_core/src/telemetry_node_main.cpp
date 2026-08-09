#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "lifecycle_msgs/msg/state.hpp"
#include "vehicle_core/telemetry_publisher.hpp"
#include "vehicle_core/failsafe_manager.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  // Create lifecycle node with AllowUndeclaredParameters callbacks
  auto options = rclcpp::NodeOptions()
    .allow_undeclared_parameters(true)
    .automatically_declare_parameters_from_overrides(true);

  auto node = std::make_shared<vehicle_core::TelemetryPublisher>(
    "telemetry_publisher", options);

  // Create failsafe manager
  auto failsafe = std::make_shared<vehicle_core::FailsafeManager>(node);

  // Trigger lifecycle transitions: CONFIGURE -> ACTIVATE
  auto graph = node->get_transition_graph();
  (void)node->trigger_transition(graph[0]);

  auto result = node->trigger_transition(graph[1]);

  auto current_state = node->get_current_state();
  if (current_state.id() != lifecycle_msgs::msg::State::PRIMARY_STATE_ACTIVE) {
    RCLCPP_ERROR(node->get_logger(), "Failed to activate node. State: %s",
                 current_state.label().c_str());
    rclcpp::shutdown();
    return 1;
  }

  RCLCPP_INFO(node->get_logger(), "Telemetry node running");

  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node->get_node_base_interface());

  // Timer for periodic failsafe evaluation
  node->create_wall_timer(
    std::chrono::milliseconds(500),
    [failsafe]() { failsafe->evaluate(); }
  );

  // Timer for periodic telemetry publish
  node->create_wall_timer(
    std::chrono::milliseconds(node->get_publish_period_ms()),
    [node]() { node->publish(); }
  );

  executor.spin();

  rclcpp::shutdown();
  return 0;
}