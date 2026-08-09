#!/bin/bash
# ==============================================================================
# Robot OLED Display Launcher (Runs on Robot)
# Sources environment and starts hardware driver + OLED info display
# ==============================================================================

set -e
source /opt/ros/jazzy/setup.bash

if [ -f "$HOME/anzym_robot_ws/install/setup.bash" ]; then
    source "$HOME/anzym_robot_ws/install/setup.bash"
else
    echo "Error: Workspace not built yet. Please run ~/anzym_robot_ws/scripts/build.sh first."
    exit 1
fi

echo "Starting ROS2 hardware driver and OLED display manager..."
ros2 launch bringup oled_display.launch.py
