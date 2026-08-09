#!/bin/bash
# ==============================================================================
# ROS2 Jazzy Installer for Ubuntu 24.04 LTS
# Run this script on the robot via SSH:
#   ssh -t rosorin "bash ~/anzym_robot_ws/scripts/install_ros2_robot.sh"
# ==============================================================================

set -e
export DEBIAN_FRONTEND=noninteractive

echo "=== 1. Checking & Setting Locale ==="
sudo apt-get update
sudo apt-get install -y locales
sudo locale-gen en_US en_US.UTF-8
sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
export LANG=en_US.UTF-8

echo "=== 2. Enabling Ubuntu Universe Repository ==="
sudo apt-get install -y software-properties-common
sudo add-apt-repository -y universe

echo "=== 3. Adding ROS2 Jazzy GPG Key & Apt Repository ==="
sudo apt-get install -y curl
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null

echo "=== 4. Updating Package Lists & Installing ROS2 Jazzy Base ==="
sudo apt-get update
sudo apt-get install -y ros-jazzy-ros-base ros-dev-tools python3-colcon-common-extensions python3-rosdep python3-pip python3-psutil python3-serial python3-yaml python3-transforms3d

echo "=== 5. Setting Up Udev Rules for Robot Hardware ==="
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [ -f "$SCRIPT_DIR/99-ros_robot_controller.rules" ]; then
    sudo cp "$SCRIPT_DIR/99-ros_robot_controller.rules" /etc/udev/rules.d/
fi
if [ -f "$SCRIPT_DIR/99-lidar.rules" ]; then
    sudo cp "$SCRIPT_DIR/99-lidar.rules" /etc/udev/rules.d/
fi
sudo udevadm control --reload-rules && sudo udevadm trigger

echo "=== 6. Setting Up Sourcing in ~/.bashrc ==="
if ! grep -q "source /opt/ros/jazzy/setup.bash" ~/.bashrc; then
    echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc
fi

echo "===================================================="
echo "✔ ROS2 Jazzy Installation & Hardware Rules Complete!"
echo "===================================================="
