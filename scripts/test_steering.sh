#!/bin/bash
# ==============================================================================
# Quick Steering Servo Test (Runs on Robot)
# Sends direct PWM servo commands to verify the steering servo responds.
# The robot bringup (ros_robot_controller_node) must already be running.
# ==============================================================================

set -e
source /opt/ros/jazzy/setup.bash
source "$HOME/anzym_robot_ws/install/setup.bash"

echo "=== Steering Servo Test ==="
echo "This test sends 3 direct PWM servo commands to servo ID 1."
echo "Watch the front wheels for movement."
echo ""

echo "[1/3] Steering LEFT (servo position 1200)..."
ros2 topic pub --once /ros_robot_controller/pwm_servo/set_state \
  ros_robot_controller_msgs/msg/SetPWMServoState \
  "{state: [{id: [1], position: [1200]}], duration: 0.5}"
sleep 2

echo "[2/3] Steering RIGHT (servo position 1800)..."
ros2 topic pub --once /ros_robot_controller/pwm_servo/set_state \
  ros_robot_controller_msgs/msg/SetPWMServoState \
  "{state: [{id: [1], position: [1800]}], duration: 0.5}"
sleep 2

echo "[3/3] Steering CENTER (servo position 1500)..."
ros2 topic pub --once /ros_robot_controller/pwm_servo/set_state \
  ros_robot_controller_msgs/msg/SetPWMServoState \
  "{state: [{id: [1], position: [1500]}], duration: 0.5}"
sleep 1

echo ""
echo "=== Test Complete ==="
echo "Did the front wheels move left, then right, then center?"
echo "If NO movement: hardware/wiring issue or wrong servo ID."
echo "If YES movement: the joystick->servo software path has a bug."
