#include "vehicle_core/telemetry_publisher.hpp"
#include <algorithm>

using namespace vehicle_core;
using namespace std::placeholders;
using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

TelemetryPublisher::TelemetryPublisher(const std::string & node_name,
                                         const rclcpp::NodeOptions & options)
  : LifecycleNode(node_name, options)
  , last_publish_time_(std::chrono::steady_clock::now())
{
  declare_parameter<int>("publish_period_ms", 1000);
  declare_parameter<uint8_t>("drone_id", 0);
  declare_parameter<std::string>("vehicle_type", "unknown");
}

CallbackReturn TelemetryPublisher::on_configure(const rclcpp_lifecycle::State &)
{
  status_pub_ = create_publisher<gcs_interfaces::msg::VehicleBaselineStatus>(
    "/vehicle/baseline_status", rclcpp::QoS(10));

  if (has_parameter("publish_period_ms")) {
    set_publish_period_ms(get_parameter("publish_period_ms").as_int());
  }
  if (has_parameter("drone_id")) {
    set_drone_id(static_cast<uint8_t>(get_parameter("drone_id").as_int()));
  }
  if (has_parameter("vehicle_type")) {
    set_vehicle_type(get_parameter("vehicle_type").as_string());
  }

  RCLCPP_INFO(get_logger(), "TelemetryPublisher configured");
  return CallbackReturn::SUCCESS;
}

CallbackReturn TelemetryPublisher::on_activate(const rclcpp_lifecycle::State &)
{
  status_pub_->on_activate();
  RCLCPP_INFO(get_logger(), "TelemetryPublisher activated");
  return CallbackReturn::SUCCESS;
}

CallbackReturn TelemetryPublisher::on_deactivate(const rclcpp_lifecycle::State &)
{
  status_pub_->on_deactivate();
  RCLCPP_INFO(get_logger(), "TelemetryPublisher deactivated");
  return CallbackReturn::SUCCESS;
}

CallbackReturn TelemetryPublisher::on_cleanup(const rclcpp_lifecycle::State &)
{
  status_pub_.reset();
  RCLCPP_INFO(get_logger(), "TelemetryPublisher cleaned up");
  return CallbackReturn::SUCCESS;
}

CallbackReturn TelemetryPublisher::on_error(const rclcpp_lifecycle::State &)
{
  RCLCPP_ERROR(get_logger(), "TelemetryPublisher in error state");
  return CallbackReturn::FAILURE;
}

/* ─── Setters ─────────────────────────────────────────────────────── */

void TelemetryPublisher::set_battery_voltage(float v)           { battery_.voltage = v; }
void TelemetryPublisher::set_battery_current(float a)           { battery_.current = a; }
void TelemetryPublisher::set_battery_remaining(float pct)
{
  battery_.remaining_percent = std::clamp(pct, 0.0f, 100.0f);
}
void TelemetryPublisher::set_battery_temperature(float c)       { battery_.cell_temperature = c; }
void TelemetryPublisher::set_battery_health_flags(uint16_t f)   { battery_.health_flags = f; }

void TelemetryPublisher::set_link_rssi(int8_t r)               { link_.rssi = r; }
void TelemetryPublisher::set_link_packet_loss(float r)         { link_.packet_loss_rate = r; }
void TelemetryPublisher::set_link_bytes_sent(uint32_t b)       { link_.bytes_sent = b; }
void TelemetryPublisher::set_link_bytes_received(uint32_t b)   { link_.bytes_received = b; }
void TelemetryPublisher::set_link_jitter(float ms)             { link_.jitter = ms; }

void TelemetryPublisher::set_drone_id(uint8_t id)              { drone_id_ = id; }
void TelemetryPublisher::set_vehicle_type(const std::string & t){ vehicle_type_ = t; }
void TelemetryPublisher::set_flight_mode(uint8_t mode)         { flight_mode_ = mode; }
void TelemetryPublisher::set_rtk_locked(bool locked)           { rtk_locked_ = locked; }

void TelemetryPublisher::add_safety_flag(uint32_t flag)        { safety_flags_ |= flag; }
void TelemetryPublisher::clear_safety_flag(uint32_t flag)      { safety_flags_ &= ~flag; }
void TelemetryPublisher::clear_all_safety_flags()              { safety_flags_ = 0; }
uint32_t TelemetryPublisher::get_safety_flags() const          { return safety_flags_; }

void TelemetryPublisher::set_publish_period_ms(int ms)
{
  publish_period_ms_ = std::max(ms, 100);  // minimum 100 ms
}

/* ─── Message builder ─────────────────────────────────────────────── */

gcs_interfaces::msg::VehicleBaselineStatus
TelemetryPublisher::build_status_message() const
{
  gcs_interfaces::msg::VehicleBaselineStatus msg;

  msg.stamp          = this->now();
  msg.drone_id       = drone_id_;
  msg.vehicle_type   = vehicle_type_;
  msg.battery        = battery_;
  msg.network        = link_;
  msg.flight_mode    = flight_mode_;
  msg.safety_flags   = safety_flags_;
  msg.rtk_locked     = rtk_locked_;

  return msg;
}

/* ─── Publish ─────────────────────────────────────────────────────── */

void TelemetryPublisher::publish()
{
  if (!status_pub_) return;

  auto msg = build_status_message();
  status_pub_->publish(msg);
  publish_count_++;
  last_publish_time_ = std::chrono::steady_clock::now();
}