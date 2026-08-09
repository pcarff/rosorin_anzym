#!/bin/bash
# ==============================================================================
# Robot Wireless Joystick Teleop Launcher (Runs on Robot)
# Launches hardware board driver, joystick controller, and OLED status screen
# ==============================================================================

set -e
source /opt/ros/jazzy/setup.bash

if [ -f "$HOME/anzym_robot_ws/install/setup.bash" ]; then
    source "$HOME/anzym_robot_ws/install/setup.bash"
else
    echo "Error: Workspace not built yet. Please run ~/anzym_robot_ws/scripts/build.sh first."
    exit 1
fi

export MACHINE_TYPE="Ackermann"

echo "Starting ROS2 hardware driver, wireless joystick teleop, and OLED display..."
ros2 launch bringup joystick_teleop.launch.py
