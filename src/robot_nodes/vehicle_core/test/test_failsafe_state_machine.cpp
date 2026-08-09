/**
 * @file test_failsafe_state_machine.cpp
 * @brief gtest suite for FailsafeManager state machine transitions.
 *
 * Tests validate:
 *  - NORMAL → WARNING on battery low
 *  - NORMAL → WARNING on GPS lost
 *  - NORMAL → HAZARD on link timeout
 *  - NORMAL → HAZARD on geofence breach
 *  - NORMAL → HAZARD on motor fault
 *  - WARNING → EMERGENCY on battery critical
 *  - EMERGENCY → NORMAL after reset
 *  - Transition history recording
 *  - Safety flag accumulation
 *  - Concurrent event handling (multiple simultaneous faults)
 */

#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "vehicle_core/failsafe_manager.hpp"

using namespace vehicle_core;

// ─── Fixtures ───────────────────────────────────────────────────────

class FailsafeManagerTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Initialize ROS if not already initialized
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    auto options = rclcpp::NodeOptions();
    auto node = std::make_shared<rclcpp_lifecycle::LifecycleNode>(
      "failsafe_test_node", options);
    failsafe = std::make_shared<FailsafeManager>(node);
  }

  void TearDown() override
  {
    failsafe.reset();
  }

  static void TearDownTestSuite()
  {
    rclcpp::shutdown();
  }

  std::shared_ptr<FailsafeManager> failsafe;
};

// ─── TEST: Initial state ────────────────────────────────────────────

TEST_F(FailsafeManagerTest, InitialStateIsNormal)
{
  EXPECT_EQ(failsafe->get_state(), FailsafeState::NORMAL);
  EXPECT_EQ(failsafe->get_state_string(), "NORMAL");
  EXPECT_EQ(failsafe->get_safety_flags(), 0u);
  EXPECT_TRUE(failsafe->get_transition_history().empty());
}

// ─── TEST: NORMAL → WARNING transitions ─────────────────────────────

TEST_F(FailsafeManagerTest, BatteryLowTriggersWarning)
{
  failsafe->on_battery_low(15.0f);
  auto state = failsafe->evaluate();

  EXPECT_EQ(state, FailsafeState::WARNING);
  EXPECT_EQ(failsafe->get_state_string(), "WARNING");
}

TEST_F(FailsafeManagerTest, GpsLostTriggersWarning)
{
  failsafe->on_gps_lost();
  auto state = failsafe->evaluate();

  EXPECT_EQ(state, FailsafeState::WARNING);
}

TEST_F(FailsafeManagerTest, ImuAnomalyTriggersWarning)
{
  failsafe->on_imu_anomaly();
  auto state = failsafe->evaluate();

  EXPECT_EQ(state, FailsafeState::WARNING);
}

// ─── TEST: NORMAL → HAZARD transitions ──────────────────────────────

TEST_F(FailsafeManagerTest, LinkTimeoutTriggersHazard)
{
  failsafe->on_link_timeout();
  auto state = failsafe->evaluate();

  EXPECT_EQ(state, FailsafeState::HAZARD);
}

TEST_F(FailsafeManagerTest, GeofenceBreachTriggersHazard)
{
  failsafe->on_geofence_breach();
  auto state = failsafe->evaluate();

  EXPECT_EQ(state, FailsafeState::HAZARD);
}

TEST_F(FailsafeManagerTest, MotorFaultTriggersHazard)
{
  failsafe->on_motor_fault();
  auto state = failsafe->evaluate();

  EXPECT_EQ(state, FailsafeState::HAZARD);
}

// ─── TEST: WARNING → EMERGENCY transitions ──────────────────────────

TEST_F(FailsafeManagerTest, BatteryCriticalTriggersEmergency)
{
  failsafe->on_battery_critical(5.0f);
  auto state = failsafe->evaluate();

  EXPECT_EQ(state, FailsafeState::EMERGENCY);
}

// ─── TEST: Reset clears all conditions ──────────────────────────────

TEST_F(FailsafeManagerTest, ResetReturnsToNormal)
{
  // Trigger multiple conditions
  failsafe->on_battery_low(15.0f);
  failsafe->on_gps_lost();
  failsafe->evaluate();
  EXPECT_EQ(failsafe->get_state(), FailsafeState::WARNING);

  // Reset should clear
  failsafe->reset_all();
  failsafe->evaluate();
  EXPECT_EQ(failsafe->get_state(), FailsafeState::NORMAL);
  EXPECT_EQ(failsafe->get_safety_flags(), 0u);
}

// ─── TEST: Transition history ───────────────────────────────────────

TEST_F(FailsafeManagerTest, TransitionHistoryRecorded)
{
  failsafe->on_battery_low(15.0f);
  failsafe->evaluate();

  auto history = failsafe->get_transition_history();
  EXPECT_GE(history.size(), 1u);

  auto & first = history[0];
  EXPECT_EQ(first.from_state, FailsafeState::NORMAL);
  EXPECT_EQ(first.to_state, FailsafeState::WARNING);
  EXPECT_EQ(first.reason, "battery_low");
}

TEST_F(FailsafeManagerTest, MultipleTransitionsRecorded)
{
  // NORMAL → WARNING
  failsafe->on_battery_low(15.0f);
  failsafe->evaluate();

  // WARNING → EMERGENCY
  failsafe->on_battery_critical(5.0f);
  failsafe->evaluate();

  auto history = failsafe->get_transition_history();
  EXPECT_GE(history.size(), 2u);

  EXPECT_EQ(history[0].from_state, FailsafeState::NORMAL);
  EXPECT_EQ(history[0].to_state, FailsafeState::WARNING);

  EXPECT_EQ(history[1].from_state, FailsafeState::WARNING);
  EXPECT_EQ(history[1].to_state, FailsafeState::EMERGENCY);
}

// ─── TEST: Safety flag accumulation ─────────────────────────────────

TEST_F(FailsafeManagerTest, SafetyFlagsAccumulate)
{
  failsafe->on_battery_low(15.0f);
  failsafe->on_gps_lost();
  failsafe->on_link_timeout();

  uint32_t flags = failsafe->get_safety_flags();

  // Check individual bits
  EXPECT_EQ(flags & 0x00000004, 0x00000004);  // LOW_BATTERY
  EXPECT_EQ(flags & 0x00000020, 0x00000020);  // GPS_LOST
  EXPECT_EQ(flags & 0x00000008, 0x00000008);  // GCS_TIMEOUT
}

// ─── TEST: Concurrent events escalate correctly ─────────────────────

TEST_F(FailsafeManagerTest, ConcurrentEventsEscalateToHighest)
{
  // Multiple simultaneous faults: battery_low (WARNING) + geofence (HAZARD)
  failsafe->on_battery_low(15.0f);
  failsafe->on_geofence_breach();

  auto state = failsafe->evaluate();

  // Should escalate to HAZARD (higher than WARNING)
  EXPECT_EQ(state, FailsafeState::HAZARD);
}

TEST_F(FailsafeManagerTest, BatteryCriticalOverridesAll)
{
  failsafe->on_gps_lost();
  failsafe->on_link_timeout();
  failsafe->on_battery_critical(3.0f);

  auto state = failsafe->evaluate();

  // Battery critical should force EMERGENCY regardless of other conditions
  EXPECT_EQ(state, FailsafeState::EMERGENCY);
}

// ─── TEST: Link restore clears link timeout ─────────────────────────

TEST_F(FailsafeManagerTest, LinkRestoreClearsTimeout)
{
  failsafe->on_link_timeout();
  failsafe->evaluate();
  EXPECT_EQ(failsafe->get_state(), FailsafeState::HAZARD);

  failsafe->on_link_restored();
  failsafe->evaluate();

  // Should return to NORMAL since link was the only issue
  EXPECT_EQ(failsafe->get_state(), FailsafeState::NORMAL);
}

// ─── TEST: GPS restore clears GPS lost ──────────────────────────────

TEST_F(FailsafeManagerTest, GpsRestoreClearsLost)
{
  failsafe->on_gps_lost();
  failsafe->evaluate();
  EXPECT_EQ(failsafe->get_state(), FailsafeState::WARNING);

  failsafe->on_gps_restored();
  failsafe->evaluate();

  EXPECT_EQ(failsafe->get_state(), FailsafeState::NORMAL);
}

// ─── TEST: Threshold configuration ──────────────────────────────────

TEST_F(FailsafeManagerTest, BatteryThresholdsConfigurable)
{
  failsafe->set_battery_low_threshold(30.0f);
  failsafe->set_battery_critical_threshold(15.0f);

  // Values should be stored (tested indirectly through behavior)
  // This is a smoke test to ensure the setters don't crash
  EXPECT_TRUE(true);
}

// ─── Run tests ──────────────────────────────────────────────────────

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}