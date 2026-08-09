#ifndef VEHICLE_CORE__TELEMETRY_PUBLISHER_HPP_
#define VEHICLE_CORE__TELEMETRY_PUBLISHER_HPP_

#include <memory>
#include <string>
#include <chrono>
#include <vector>

#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "gcs_interfaces/msg/vehicle_baseline_status.hpp"
#include "gcs_interfaces/msg/battery_status.hpp"
#include "gcs_interfaces/msg/link_quality.hpp"

namespace vehicle_core
{
  // Flight mode constants
  enum FlightMode : uint8_t {
    FLIGHT_MODE_STABILIZE  = 0,
    FLIGHT_MODE_ALT_HOLD   = 1,
    FLIGHT_MODE_AUTO       = 2,
    FLIGHT_MODE_GUIDED     = 3,
    FLIGHT_MODE_RTL        = 4,
    FLIGHT_MODE_LAND       = 5,
    FLIGHT_MODE_TAKEOFF    = 6,
    FLIGHT_MODE_OFFBOARD   = 7
  };

  // Safety flag bitmasks
  enum SafetyFlag : uint32_t {
    SAFETY_GEOFENCE_BREACH      = 0x00000001,
    SAFETY_RC_LINK_LOSS         = 0x00000002,
    SAFETY_LOW_BATTERY          = 0x00000004,
    SAFETY_GCS_TIMEOUT          = 0x00000008,
    SAFETY_MOTOR_FAULT          = 0x00000010,
    SAFETY_GPS_LOST             = 0x00000020,
    SAFETY_AIRSPEED_ANOMALY     = 0x00000040,
    SAFETY_BARO_FAULT           = 0x00000080,
    SAFETY_COMPASS_INTERFERENCE = 0x00000100,
    SAFETY_BATTERY_CRITICAL     = 0x00000200,
    SAFETY_MOTOR_OVERCURRENT    = 0x00000400,
    SAFETY_IMU_ANOMALY          = 0x00000800,
    SAFETY_MISSION_ERROR        = 0x00001000,
    SAFETY_ACTUATOR_SATURATION  = 0x00002000,
    SAFETY_PARACHUTE_ARMED      = 0x00004000,
    SAFETY_PARACHUTE_DEPLOYED   = 0x00008000
  };

  class TelemetryPublisher : public rclcpp_lifecycle::LifecycleNode
  {
  public:
    explicit TelemetryPublisher(const std::string & node_name = "telemetry_publisher",
                                 const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

    /// Build a fully-populated VehicleBaselineStatus message from current internal state.
    gcs_interfaces::msg::VehicleBaselineStatus build_status_message() const;

    /// Update battery fields (called from sensor callbacks)
    void set_battery_voltage(float v);
    void set_battery_current(float a);
    void set_battery_remaining(float pct);
    void set_battery_temperature(float c);
    void set_battery_health_flags(uint16_t flags);

    /// Update link quality fields
    void set_link_rssi(int8_t rssi);
    void set_link_packet_loss(float rate);
    void set_link_bytes_sent(uint32_t b);
    void set_link_bytes_received(uint32_t b);
    void set_link_jitter(float ms);

    /// Update vehicle metadata
    void set_drone_id(uint8_t id);
    void set_vehicle_type(const std::string & type);
    void set_flight_mode(uint8_t mode);
    void set_rtk_locked(bool locked);

    /// Manipulate safety flags
    void add_safety_flag(uint32_t flag);
    void clear_safety_flag(uint32_t flag);
    void clear_all_safety_flags();
    uint32_t get_safety_flags() const;

    /// Get the publish period in milliseconds (default 1000 = 1 Hz)
    int get_publish_period_ms() const { return publish_period_ms_; }
    void set_publish_period_ms(int ms);

    /// Get the number of messages published (for testing)
    size_t get_publish_count() const { return publish_count_; }

    /// Actually publish the message on the ROS topic
    void publish();

  private:
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    on_configure(const rclcpp_lifecycle::State &);
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    on_activate(const rclcpp_lifecycle::State &);
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    on_deactivate(const rclcpp_lifecycle::State &);
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    on_cleanup(const rclcpp_lifecycle::State &);
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    on_error(const rclcpp_lifecycle::State &);

    rclcpp_lifecycle::LifecyclePublisher<
      gcs_interfaces::msg::VehicleBaselineStatus>::SharedPtr status_pub_;

    gcs_interfaces::msg::BatteryStatus battery_;
    gcs_interfaces::msg::LinkQuality link_;
    uint8_t drone_id_            = 0;
    std::string vehicle_type_    = "unknown";
    uint8_t flight_mode_         = FLIGHT_MODE_STABILIZE;
    uint32_t safety_flags_       = 0;
    bool rtk_locked_             = false;
    int publish_period_ms_       = 1000;
    size_t publish_count_        = 0;

    std::chrono::steady_clock::time_point last_publish_time_;
  };  // class TelemetryPublisher

}  // namespace vehicle_core

#endif  // VEHICLE_CORE__TELEMETRY_PUBLISHER_HPP_