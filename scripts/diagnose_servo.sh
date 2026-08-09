#!/bin/bash
# ==============================================================================
# ROS Servo Topic Diagnostic (Run WHILE bringup is active in another terminal)
# ==============================================================================

source /opt/ros/jazzy/setup.bash
source "$HOME/anzym_robot_ws/install/setup.bash"

echo "=== ROS Servo Topic Diagnostic ==="
echo ""

echo "[1] Active nodes:"
ros2 node list 2>/dev/null
echo ""

echo "[2] Looking for PWM servo topics:"
ros2 topic list 2>/dev/null | grep -i pwm || echo "  (none found!)"
echo ""

echo "[3] Full topic list:"
ros2 topic list 2>/dev/null
echo ""

echo "[4] Topic info for /ros_robot_controller/pwm_servo/set_state:"
ros2 topic info /ros_robot_controller/pwm_servo/set_state 2>/dev/null || echo "  Topic does NOT exist!"
echo ""

echo "[5] Checking all subscribers on ros_robot_controller node:"
ros2 node info /ros_robot_controller 2>/dev/null || echo "  Node NOT found!"
echo ""

echo "[6] Sending test servo command via ROS topic..."
echo "    (Watch the front wheels!)"
ros2 topic pub --once /ros_robot_controller/pwm_servo/set_state \
  ros_robot_controller_msgs/msg/SetPWMServoState \
  "{state: [{id: [1], position: [1200]}], duration: 0.5}" 2>&1
sleep 2

echo "[7] Returning servo to center..."
ros2 topic pub --once /ros_robot_controller/pwm_servo/set_state \
  ros_robot_controller_msgs/msg/SetPWMServoState \
  "{state: [{id: [1], position: [1500]}], duration: 0.5}" 2>&1
echo ""

echo "=== Diagnostic Complete ==="
