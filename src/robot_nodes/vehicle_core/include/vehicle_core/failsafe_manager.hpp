#ifndef VEHICLE_CORE__FAILSAFE_MANAGER_HPP_
#define VEHICLE_CORE__FAILSAFE_MANAGER_HPP_

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "gcs_interfaces/msg/vehicle_baseline_status.hpp"

using LifecycleNodeSharedPtr = std::shared_ptr<rclcpp_lifecycle::LifecycleNode>;

namespace vehicle_core
{
  // ─── Failsafe states ──────────────────────────────────────────────
  enum class FailsafeState : uint8_t {
    NORMAL     = 0,
    WARNING    = 1,
    HAZARD     = 2,
    EMERGENCY  = 3,
    TERMINATED = 4
  };

  inline std::string failsafe_state_to_string(FailsafeState state)
  {
    switch (state) {
      case FailsafeState::NORMAL:     return "NORMAL";
      case FailsafeState::WARNING:    return "WARNING";
      case FailsafeState::HAZARD:     return "HAZARD";
      case FailsafeState::EMERGENCY:  return "EMERGENCY";
      case FailsafeState::TERMINATED: return "TERMINATED";
      default:                        return "UNKNOWN";
    }
  }

  // ─── Transition record for audit trail ────────────────────────────
  struct TransitionRecord {
    std::chrono::steady_clock::time_point timestamp;
    FailsafeState from_state;
    FailsafeState to_state;
    std::string reason;
  };

  class FailsafeManager
  {
  public:
    explicit FailsafeManager(LifecycleNodeSharedPtr node);

    // ─── Event injection (called from sensor / link callbacks) ──────
    void on_battery_low(float remaining_percent);
    void on_battery_critical(float remaining_percent);
    void on_link_timeout();
    void on_link_restored();
    void on_geofence_breach();
    void on_motor_fault();
    void on_gps_lost();
    void on_gps_restored();
    void on_imu_anomaly();
    void reset_all();

    // ─── State query ────────────────────────────────────────────────
    FailsafeState get_state() const;
    uint32_t get_safety_flags() const;
    std::string get_state_string() const;

    // ─── Run one evaluation cycle ───────────────────────────────────
    FailsafeState evaluate();

    // ─── Get transition history (for testing / diagnostics) ─────────
    std::vector<TransitionRecord> get_transition_history() const;

    // ─── Threshold configuration (for testing) ──────────────────────
    void set_battery_low_threshold(float pct);
    void set_battery_critical_threshold(float pct);
    void set_link_timeout_ms(int64_t ms);

  private:
    void transition_to(FailsafeState new_state, const std::string & reason);

    LifecycleNodeSharedPtr node_;

    mutable std::mutex mutex_;
    std::atomic<FailsafeState> current_state_{FailsafeState::NORMAL};
    uint32_t safety_flags_{0};

    // Condition flags (set by event callbacks, evaluated in evaluate())
    bool battery_low_{false};
    bool battery_critical_{false};
    bool link_timeout_{false};
    bool geofence_breach_{false};
    bool motor_fault_{false};
    bool gps_lost_{false};
    bool imu_anomaly_{false};

    // Thresholds
    float battery_low_threshold_{20.0f};
    float battery_critical_threshold_{10.0f};
    int64_t link_timeout_ms_{5000};

    // Audit trail
    std::vector<TransitionRecord> transition_history_;
  };

}  // namespace vehicle_core

#endif  // VEHICLE_CORE__FAILSAFE_MANAGER_HPP_