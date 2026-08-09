#include "vehicle_core/failsafe_manager.hpp"
#include <algorithm>

using namespace vehicle_core;

FailsafeManager::FailsafeManager(LifecycleNodeSharedPtr node)
  : node_(node)
{
}

// ─── Event injection ────────────────────────────────────────────────

void FailsafeManager::on_battery_low(float remaining_percent)
{
  (void)remaining_percent;
  std::lock_guard<std::mutex> lock(mutex_);
  battery_low_ = true;
  safety_flags_ |= 0x00000004;  // SAFETY_LOW_BATTERY
}

void FailsafeManager::on_battery_critical(float remaining_percent)
{
  (void)remaining_percent;
  std::lock_guard<std::mutex> lock(mutex_);
  battery_critical_ = true;
  battery_low_ = true;
  safety_flags_ |= 0x00000200;  // SAFETY_BATTERY_CRITICAL
  safety_flags_ |= 0x00000004;  // SAFETY_LOW_BATTERY
}

void FailsafeManager::on_link_timeout()
{
  std::lock_guard<std::mutex> lock(mutex_);
  link_timeout_ = true;
  safety_flags_ |= 0x00000008;  // SAFETY_GCS_TIMEOUT
}

void FailsafeManager::on_link_restored()
{
  std::lock_guard<std::mutex> lock(mutex_);
  link_timeout_ = false;
  safety_flags_ &= ~0x00000008;
}

void FailsafeManager::on_geofence_breach()
{
  std::lock_guard<std::mutex> lock(mutex_);
  geofence_breach_ = true;
  safety_flags_ |= 0x00000001;  // SAFETY_GEOFENCE_BREACH
}

void FailsafeManager::on_motor_fault()
{
  std::lock_guard<std::mutex> lock(mutex_);
  motor_fault_ = true;
  safety_flags_ |= 0x00000010;  // SAFETY_MOTOR_FAULT
}

void FailsafeManager::on_gps_lost()
{
  std::lock_guard<std::mutex> lock(mutex_);
  gps_lost_ = true;
  safety_flags_ |= 0x00000020;  // SAFETY_GPS_LOST
}

void FailsafeManager::on_gps_restored()
{
  std::lock_guard<std::mutex> lock(mutex_);
  gps_lost_ = false;
  safety_flags_ &= ~0x00000020;
}

void FailsafeManager::on_imu_anomaly()
{
  std::lock_guard<std::mutex> lock(mutex_);
  imu_anomaly_ = true;
  safety_flags_ |= 0x00000800;  // SAFETY_IMU_ANOMALY
}

void FailsafeManager::reset_all()
{
  std::lock_guard<std::mutex> lock(mutex_);
  battery_low_       = false;
  battery_critical_  = false;
  link_timeout_      = false;
  geofence_breach_   = false;
  motor_fault_       = false;
  gps_lost_          = false;
  imu_anomaly_       = false;
  safety_flags_      = 0;
}

// ─── State query ────────────────────────────────────────────────────

FailsafeState FailsafeManager::get_state() const
{
  return current_state_.load();
}

uint32_t FailsafeManager::get_safety_flags() const
{
  std::lock_guard<std::mutex> lock(mutex_);
  return safety_flags_;
}

std::string FailsafeManager::get_state_string() const
{
  return failsafe_state_to_string(current_state_.load());
}

std::vector<TransitionRecord> FailsafeManager::get_transition_history() const
{
  std::lock_guard<std::mutex> lock(mutex_);
  return transition_history_;
}

// ─── Threshold configuration ────────────────────────────────────────

void FailsafeManager::set_battery_low_threshold(float pct)
{
  std::lock_guard<std::mutex> lock(mutex_);
  battery_low_threshold_ = pct;
}

void FailsafeManager::set_battery_critical_threshold(float pct)
{
  std::lock_guard<std::mutex> lock(mutex_);
  battery_critical_threshold_ = pct;
}

void FailsafeManager::set_link_timeout_ms(int64_t ms)
{
  std::lock_guard<std::mutex> lock(mutex_);
  link_timeout_ms_ = ms;
}

// ─── Evaluation cycle ───────────────────────────────────────────────

FailsafeState FailsafeManager::evaluate()
{
  std::lock_guard<std::mutex> lock(mutex_);

  FailsafeState new_state = FailsafeState::NORMAL;

  // Determine worst-case state from all active conditions
  if (battery_critical_) {
    new_state = FailsafeState::EMERGENCY;
  } else if (motor_fault_ || geofence_breach_) {
    new_state = std::max(new_state, FailsafeState::HAZARD);
  }

  if (link_timeout_ && (new_state <= FailsafeState::WARNING)) {
    new_state = FailsafeState::HAZARD;
  }

  if (gps_lost_ || imu_anomaly_) {
    new_state = std::max(new_state, FailsafeState::WARNING);
  }

  if (battery_low_ && (new_state == FailsafeState::NORMAL)) {
    new_state = FailsafeState::WARNING;
  }

  // Transition if state changed
  auto old_state = current_state_.load();
  if (new_state != old_state) {
    std::string reason;
    if (new_state == FailsafeState::EMERGENCY) {
      reason = "battery_critical";
    } else if (new_state == FailsafeState::HAZARD) {
      reason = geofence_breach_ ? "geofence_breach"
             : motor_fault_ ? "motor_fault"
             : "link_timeout";
    } else if (new_state == FailsafeState::WARNING) {
      reason = gps_lost_ ? "gps_lost"
             : imu_anomaly_ ? "imu_anomaly"
             : "battery_low";
    } else {
      reason = "all_clear";
    }

    transition_history_.push_back({
      std::chrono::steady_clock::now(),
      old_state,
      new_state,
      reason
    });

    current_state_.store(new_state);

    if (node_) {
      RCLCPP_WARN(node_->get_logger(),
                  "Failsafe transition: %s -> %s (reason: %s)",
                  failsafe_state_to_string(old_state).c_str(),
                  failsafe_state_to_string(new_state).c_str(),
                  reason.c_str());
    }
  }

  return new_state;
}

// ─── Private helper ─────────────────────────────────────────────────

void FailsafeManager::transition_to(FailsafeState new_state, const std::string & reason)
{
  auto old_state = current_state_.load();
  current_state_.store(new_state);

  transition_history_.push_back({
    std::chrono::steady_clock::now(),
    old_state,
    new_state,
    reason
  });
}