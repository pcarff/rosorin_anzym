#!/bin/bash
# Direct hardware servo test - bypasses ROS, talks to STM32 directly.
# Run this WHILE bringup is NOT running.

source /opt/ros/jazzy/setup.bash
source "$HOME/anzym_robot_ws/install/setup.bash"

python3 - << 'PYEOF'
import sys, time
from ros_robot_controller.ros_robot_controller_sdk import Board

print("=== Direct PWM Servo Hardware Test ===")
print("Opening /dev/rrc serial port...")

try:
    board = Board()
    board.enable_reception()
    print("Board connected!\n")
except Exception as e:
    print(f"ERROR: Could not connect to board: {e}")
    sys.exit(1)

for servo_id in range(1, 5):
    print(f"--- Testing PWM Servo ID {servo_id} ---")
    print(f"  Position 1200 (left)...")
    board.pwm_servo_set_position(0.5, [[servo_id, 1200]])
    time.sleep(1.5)
    print(f"  Position 1800 (right)...")
    board.pwm_servo_set_position(0.5, [[servo_id, 1800]])
    time.sleep(1.5)
    print(f"  Position 1500 (center)...")
    board.pwm_servo_set_position(0.5, [[servo_id, 1500]])
    time.sleep(1.0)
    print(f"  Did servo {servo_id} move?\n")

print("=== Test Complete ===")
print("If NO servo moved: board/firmware issue.")
print("If a servo moved on a DIFFERENT ID: wrong connector.")
PYEOF
