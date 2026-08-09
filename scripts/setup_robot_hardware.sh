#!/bin/bash
# ==============================================================================
# Robot Hardware & Udev Setup Script (Runs on Robot)
# Configures serial port permissions and /dev/rrc udev rule for STM32 board
# ==============================================================================

set -e

echo "=== 1. Adding user $USER to dialout group ==="
sudo usermod -aG dialout "$USER"

echo "=== 2. Creating udev rule for STM32 board (/dev/rrc) ==="
cat << 'EOF' | sudo tee /etc/udev/rules.d/99-ros_robot_controller.rules > /dev/null
# Symlink /dev/ttyACM* to /dev/rrc with read/write permissions
KERNEL=="ttyACM*", MODE="0666", SYMLINK+="rrc"
EOF

echo "=== 3. Reloading udev rules ==="
sudo udevadm control --reload-rules
sudo udevadm trigger

echo "===================================================="
echo "✔ Hardware permissions & /dev/rrc udev rule configured!"
echo "===================================================="
