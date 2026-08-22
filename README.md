# ROSOrin Ackermann Robot (`rosorin_anzym`)

A modern ROS 2 (Jazzy Jalisco) control stack for the **ROSOrin Ackermann Robot** running on Ubuntu 24.04 LTS (NVIDIA Jetson Orin Nano / Orin NX) with integrated STM32 controller, LD19 2D LIDAR, Deptrum Aurora 930 3D Depth Camera, and wireless gamepad teleoperation.

> [!IMPORTANT]
> 🚀 **Ubuntu 24.04 & ROS 2 Jazzy Refactor**: This repository is a complete refactor and upgrade of the original ROSOrin robot stack to **Ubuntu 24.04 LTS** and **ROS 2 Jazzy Jalisco**. All packages, drivers, launch files, udev rules, and scripts have been modernized and tested for zero-latency control and robust hardware execution.

> [!TIP]
> 🎓 **Students & Beginners**: Check out the [Student & Beginner Engineering Guide](docs/STUDENT_GUIDE.md) for a middle-school friendly walkthrough of what the robot is, how to turn it on and start it, how to drive with the wireless gamepad, and fun hands-on STEM science & engineering experiments!

---

## 🚗 Platform & Hardware Overview

* **Compute Unit:** NVIDIA Jetson Orin Nano / Orin NX (Ubuntu 24.04 LTS / ROS 2 Jazzy Jalisco)
* **Microcontroller:** HiWonder STM32 Robot Controller (`/dev/ttyACM0` serial interface)
* **Drive Kinematics:** Ackermann chassis (Rear-wheel DC drive motors + Front-wheel PWM steering servo ID 1)
* **LIDAR Sensor:** LDROBOT LD19 2D DToF LIDAR (`/dev/ttyCH341USB0` / `/dev/lidar` @ 230400 baud, 360° `/scan` topic)
* **3D Perception:** Deptrum Aurora 930 3D Depth Camera (RGB feed, 16-bit Depth, 3D PointCloud2)
* **Human Interface:** Wireless Gamepad (USB Joy), SSD1306 OLED Display, 9-DOF IMU, Battery Monitor

---

## 📁 Repository Structure

```text
├── docs/
│   └── STUDENT_GUIDE.md           # Middle school & student STEM guide
├── scripts/
│   ├── bringup_robot.sh           # Robot hardware bringup script
│   ├── install_ros2_robot.sh      # One-command robot environment installer
│   ├── launch_workstation_viz.sh  # Workstation one-command RViz2 visualization launcher
│   ├── setup_autostart.sh         # Systemd service installer (boot autostart)
│   ├── 99-lidar.rules             # Udev rules for LD19 LIDAR (/dev/lidar)
│   └── 99-ros_robot_controller.rules # Udev rules for STM32 board
├── src/
│   ├── bringup/                   # System bringup launch files & OLED info node
│   │   └── launch/
│   │       ├── joystick_teleop.launch.py # Main robot bringup (Drivers, Odom, Joy, LIDAR, Camera)
│   │       └── view_lidar.launch.py      # Workstation RViz2 visualization launcher
│   ├── driver/
│   │   ├── controller/            # Ackermann chassis kinematics & odometry publisher
│   │   ├── deptrum-ros-driver-aurora930/ # 3D Depth Camera C++ ROS 2 driver
│   │   ├── ldlidar_stl_ros2/      # LD19 2D DToF LIDAR C++ ROS 2 driver
│   │   ├── ros_robot_controller/  # STM32 serial communication & SDK
│   │   ├── ros_robot_controller_msgs/ # Custom STM32 message definitions
│   │   ├── servo_controller/      # Bus & PWM servo trajectory controllers
│   │   └── servo_controller_msgs/ # Servo state & action message definitions
│   ├── gcs_interfaces/            # Ground station control interfaces
│   ├── gcs_ui/                    # Ground control station web interface
│   ├── interfaces/                # Robot state message definitions
│   ├── peripherals/               # Joystick teleoperation & sensor configurations
│   └── robot_nodes/               # Higher-level robot control nodes
└── manage_workspace.sh            # Universal workspace manager (build | deploy | bringup | status | clean)
```

---

## 🚀 Operations & Bringup

### 1. Unified Workspace Manager (`manage_workspace.sh`)

Use the `manage_workspace.sh` script for all workspace build, deployment, and bringup tasks:

```bash
# Build the workspace locally
./manage_workspace.sh build

# Sync code to the robot (rosorin:~/anzym_robot_ws) and compile remotely
./manage_workspace.sh deploy

# Trigger robot bringup remotely
./manage_workspace.sh bringup

# Check systemd autostart service status on the robot
./manage_workspace.sh status

# Clean local build/install artifacts
./manage_workspace.sh clean
```

### 2. Autonomous Navigation & SLAM (Nav2 on ROS 2 Jazzy)

The ROSOrin robot features a fully configured **Nav2 (Navigation 2)** and **SLAM Toolbox** autonomous navigation stack parameterized specifically for **Ackermann steering kinematics**:

```bash
# Launch autonomous navigation with static map/odom TF
ros2 launch bringup navigation.launch.py slam:=false autostart:=true

# Launch autonomous navigation with live dynamic SLAM mapping
ros2 launch bringup navigation.launch.py slam:=true autostart:=true
```

#### Nav2 Stack Architecture:
- **Kinematic Model**: Ackermann chassis (`nav2_params_ackermann.yaml`) with Regulated Pure Pursuit / DWB controller.
- **Direct Lifecycle Management**: Managed nodes (`controller_server`, `smoother_server`, `planner_server`, `behavior_server`, `velocity_smoother`, `collision_monitor`, `bt_navigator`, `waypoint_follower`) orchestrated with custom bond timeouts.
- **Costmaps & Obstacle Avoidance**: 2D LiDAR (`/scan`) obstacle layer with footprint and inflation radii tuned for Ackermann turning constraints.
- **Action Interface & GCS Integration**: Full compatibility with `nav2_msgs/action/NavigateToPose` and `/goal_pose`, including zero-velocity cancellation via rosbridge service call to `/navigate_to_pose/_action/cancel_goal`.

### 3. Workstation RViz2 LIDAR & Robot Visualization

To view live 360° LIDAR scan data, 3D point clouds, and robot TF transforms in RViz2 from your workstation:

```bash
./scripts/launch_workstation_viz.sh
```

### 4. Automatic Power-Up Autostart (Systemd)

To enable automatic bringup every time the robot powers on:

```bash
# Enable automatic bringup on boot
ssh rosorin "bash ~/anzym_robot_ws/scripts/setup_autostart.sh install"

# Check status or disable
ssh rosorin "bash ~/anzym_robot_ws/scripts/setup_autostart.sh status"
ssh rosorin "bash ~/anzym_robot_ws/scripts/setup_autostart.sh disable"
```

---

## 📡 Published Active ROS 2 Topics

When bringup is running (`rosorin_bringup.service`), the robot publishes the following ROS 2 topics:

| Topic | Message Type | Description |
|---|---|---|
| `/scan` | `sensor_msgs/msg/LaserScan` | 360° 2D LaserScan distances & intensities (10 Hz) |
| `/depth_cam/depth0/points` | `sensor_msgs/msg/PointCloud2` | 3D Spatial Point Cloud |
| `/depth_cam/rgb0/image_raw` | `sensor_msgs/msg/Image` | Live RGB Color Camera Stream |
| `/depth_cam/depth0/image_raw` | `sensor_msgs/msg/Image` | 16-bit Depth Map Stream |
| `/odom_raw` | `nav_msgs/msg/Odometry` | Wheel odometry & pose telemetry |
| `/controller/cmd_vel` | `geometry_msgs/msg/Twist` | Velocity & steering commands |
| `/ros_robot_controller/battery` | `std_msgs/msg/Float32` | Live battery voltage level |

---

## 📄 License

Maintained for the ROSOrin Ackermann Robot platform running Ubuntu 24.04 LTS and ROS 2 Jazzy. Refer to individual package headers for specific licensing details.