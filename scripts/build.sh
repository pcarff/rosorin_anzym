#!/bin/bash
# ==============================================================================
# Robot Compilation Script (Runs on Robot)
# Sources ROS2 Jazzy and compiles the workspace using colcon
# ==============================================================================

set -e
source /opt/ros/jazzy/setup.bash
cd "$HOME/anzym_robot_ws"

echo "Building ROS2 workspace on robot..."
colcon build
echo "Build complete! Remember to source install/setup.bash"
