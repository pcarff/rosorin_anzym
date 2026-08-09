#!/bin/bash
# ==============================================================================
# Workstation RViz2 LIDAR & Robot Visualization Launcher
# Run this on your workstation to view live 2D LIDAR scans & robot transforms:
#   ./scripts/launch_workstation_viz.sh
# ==============================================================================

set -e

echo "🕹️ Launching Workstation RViz2 LIDAR & Robot Visualization..."

# 1. Source ROS2 environment
if [ -f /opt/ros/jazzy/setup.bash ]; then
    source /opt/ros/jazzy/setup.bash
elif [ -f /opt/ros/humble/setup.bash ]; then
    source /opt/ros/humble/setup.bash
else
    echo "❌ Error: ROS2 installation setup.bash not found in /opt/ros/"
    exit 1
fi

# 2. Source local workspace if built
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WS_DIR="$(dirname "$SCRIPT_DIR")"

if [ -f "$WS_DIR/install/setup.bash" ]; then
    source "$WS_DIR/install/setup.bash"
fi

# 3. Set ROS Domain ID (match robot default = 0)
export ROS_DOMAIN_ID=${ROS_DOMAIN_ID:-0}

echo "📡 Listening on ROS_DOMAIN_ID=$ROS_DOMAIN_ID..."
echo "🖥️ Starting RViz2 with LIDAR & TF visualization..."

ros2 run rviz2 rviz2 -d "$WS_DIR/src/peripherals/rviz/lidar_view.rviz"
