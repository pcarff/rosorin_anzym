#!/bin/bash
# ==============================================================================
# Main Robot Bringup Launcher (Runs on Robot)
# Launches STM32 hardware driver, motor & steering drive, wireless joystick, and OLED display
# ==============================================================================

set -e
source /opt/ros/jazzy/setup.bash

ROBOT_WS="${HOME:-/home/pcarff}/anzym_robot_ws"
if [ -f "${ROBOT_WS}/install/setup.bash" ]; then
    source "${ROBOT_WS}/install/setup.bash"
elif [ -f "/home/pcarff/anzym_robot_ws/install/setup.bash" ]; then
    source "/home/pcarff/anzym_robot_ws/install/setup.bash"
else
    echo "Error: Workspace not built yet. Please run ~/anzym_robot_ws/scripts/build.sh first."
    exit 1
fi

export MACHINE_TYPE="ROSOrin_Acker"
export FASTRTPS_DEFAULT_PROFILES_FILE="$HOME/anzym_robot_ws/fastdds_wifi.xml"

echo "Starting ROSOrin Ackermann Robot (Hardware, Motors, Steering, Joystick, OLED)..."
ros2 launch bringup joystick_teleop.launch.py
