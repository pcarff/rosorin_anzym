# 🤖 Student & Beginner Engineering Guide: AnZym ROSOrin Robot

Here you will learn how the robot works, how its computer "brain" communicates with its "muscles" and "eyes", how to start it up, and how to drive it using a wireless gamepad!

> [!NOTE]
> 🚀 **Ubuntu 24.04 & ROS 2 Jazzy Refactor**: This robot software stack has been fully updated and refactored for **Ubuntu 24.04 LTS** and **ROS 2 Jazzy Jalisco**. It includes high-speed 2D DToF LIDAR laser scanning, 3D depth perception, real-time wireless gamepad control, and workstation 3D visualization.

---

## 🧭 Table of Contents
1. [What Is This Robot?](#1-what-is-this-robot)
2. [How the Robot Works (Hardware & Brain)](#2-how-the-robot-works-hardware--brain)
3. [How to Turn On and Start the Robot](#3-how-to-turn-on-and-start-the-robot)
4. [How to Drive with the Gamepad](#4-how-to-drive-with-the-gamepad)
5. [Current Features & Capabilities](#5-current-features--capabilities)
6. [Fun STEM Science & Engineering Experiments](#6-fun-stem-science--engineering-experiments)

---

## 1. What Is This Robot?

The **AnZym ROSOrin** is a smart, autonomous car-style robot designed for robotics learning, obstacle avoidance, mapping, and artificial intelligence (AI). 

Unlike a simple remote-controlled toy car, this robot contains a full **supercomputer** inside it! It uses real automotive steering (called **Ackermann steering**, just like a tesla or electric vehicle) and runs **ROS 2 (Robot Operating System)** on **Ubuntu Linux 24.04**.

```text
       +-------------------------------------------------------+
       |                  3D Depth Camera                      |
       |             (Sees obstacles in 3D)                    |
       +---------------------------+---------------------------+
                                   |
       +---------------------------v---------------------------+
       |                NVIDIA Jetson Supercomputer             |
       |         (Runs ROS 2 Jazzy & Autonomous Brain)         |
       +---------------------------+---------------------------+
                                   | Serial Communication
       +---------------------------v---------------------------+
       |             STM32 Motor Controller Board              |
       |    (Controls Steering Servo, Wheel Motors & OLED)     |
       +-------------------+---------------+-------------------+
                           |               |
             +-------------v----+     +----v-------------------+
             | Front Steering   |     | Rear Wheel Motors      |
             | Servo (Left/Right|     | (Drive Forward/Reverse)|
             +------------------+     +------------------------+
```

---

## 2. How the Robot Works (Hardware & Brain)

The robot is divided into four main systems:

### 🧠 1. The Brain (NVIDIA Jetson Computer)
- **What it is**: An advanced computer board running **Ubuntu Linux 24.04 LTS** and **ROS 2 Jazzy**.
- **What it does**: Thinks, makes decisions, calculates speeds, processes 3D camera images, and listens to gamepad signals over Wi-Fi.

### 💪 2. The Muscles (STM32 Microcontroller & Drive Motors)
- **What it is**: A high-speed microcontroller board connected to electric motors.
- **What it does**: Takes instructions from ROS 2 and turns them into precise electrical signals to turn the front steering servo and spin the rear drive wheels.

### 👁️ 3. The Eyes & Sensors (3D Depth Camera & IMU)
- **Deptrum Aurora 3D Camera**: Projects invisible infrared light dots to measure exact distances to objects, creating a 3D point cloud map of the room.
- **9-DOF IMU Sensor**: Measures tilt, rotation, and acceleration so the robot knows if it is turning or driving on a slope.

### 📺 4. The Dashboard (SSD1306 OLED Display)
- Mounted on the front of the robot.
- Displays live status: Robot IP address, hostname, battery voltage, and CPU temperature.

---

## 3. How to Turn On and Start the Robot

### Step 1: Power Up the Robot
1. Turn on the main battery power switch on the robot chassis.
2. Wait about **20 seconds** for the onboard computer to boot up.
3. Look at the small **OLED screen** on the front of the robot. When it displays `AnZym / ROSOrin: Online` and shows its Wi-Fi IP address, the robot is ready!

### Step 2: Automatic Boot Service vs. Manual Start
The robot has an **automatic startup service** (`rosorin_bringup.service`) that turns on all hardware drivers, steering controls, and camera systems as soon as power is applied.

If you ever want to check its status or start it manually from your workstation laptop:

```bash
# Check if the robot is online and ready
./manage_workspace.sh status

# Manually start the joystick teleop bringup
./manage_workspace.sh bringup
```

---

## 4. How to Drive with the Gamepad

Make sure the wireless USB gamepad receiver is plugged into the robot and the controller switch is turned ON.

```text
               +-----------------------------------+
               |          [ Wireless Gamepad ]     |
               +-----------------------------------+
                  /                             \
                 /    (LY) Driving Joystick      \
                |       ▲                         |
                |   ◄───────► Forward / Reverse   |
                |       ▼                         |
                |                                 |
                |     (RX) Steering Joystick      |
                |       ▲                         |
                |   ◄───────► Turn Left / Right   |
                |       ▼                         |
                |                                 |
                |     [START] Horn Buzzer         |
                 \                               /
                  \                             /
                   +---------------------------+
```

### Gamepad Controls:
- **Left Stick (Up / Down)**: Controls driving speed ($v_x$). Push forward to drive forward, pull back to reverse.
- **Right Stick (Left / Right)**: Controls front wheel steering angle ($\theta$). Push left to steer left, push right to steer right.
- **START Button**: Press to sound the onboard buzzer horn!

---

## 5. Current Features & Capabilities

Here is what the robot can do right now:

| Feature | Description | How It Works |
|---|---|---|
| 🚗 **Ackermann Driving** | Car-style steering and driving kinematics | Front PWM Servo ID 1 controls steering angle while rear DC motors drive speed |
| 🕹️ **Wireless Gamepad Control** | Drive using a wireless controller | `joystick_control` node converts joystick axes into ROS 2 `geometry_msgs/msg/Twist` velocity commands |
| 🗺️ **Autonomous Navigation (Nav2)** | Drive to goal positions automatically | Nav2 plans paths around obstacles using LiDAR and drives the car autonomously |
| 🎯 **Point & Click Map Targeting** | Click anywhere on GCS to navigate | Web Ground Control Station sends `/goal_pose` and Nav2 navigates to the target |
| 📺 **Live OLED Telemetry** | Displays status on front screen | `oled_info_node` reads system health, battery voltage, and IP address |
| 📷 **3D Depth Vision** | Sees objects in 3D | `deptrum-ros-driver` Streams 16-bit depth maps and 3D PointCloud2 data |
| ⚡ **Automatic Power-On Boot** | Starts automatically on power up | Systemd service `rosorin_bringup.service` starts bringup scripts on boot |

---

## 6. Fun STEM Science & Engineering Experiments

Want to test your science and engineering skills? Try these hands-on activities!

### 🔬 Experiment 1: Measure Turning Radius (Geometry & Math)
- **Goal**: Find out how sharp the robot can turn.
- **Action**: Drive the robot in a tight full circle at low speed on the floor. Mark the outer wheel path with tape and measure the diameter with a tape measure.
- **Science Concept**: Car steering geometry follows the Ackermann formula:
  $$R = \frac{L}{\tan(\theta)}$$
  Where $R$ is turning radius, $L$ is wheelbase length, and $\theta$ is steering angle!

### 👁️ Experiment 2: Explore 3D Depth Perception in RViz2
- **Goal**: See what the 3D camera sees in real-time.
- **Action**: Open RViz2 on your workstation:
  ```bash
  source /opt/ros/jazzy/setup.bash
  ros2 run rviz2 rviz2
  ```
  Add a **PointCloud2** display subscribing to `/depth_cam/depth0/points`. Stand in front of the camera and watch the 3D color-coded dots outline your shape in 3D space!

### ⚙️ Experiment 3: Adjust Driving Speed Limits (Computer Science)
- **Goal**: Learn how software parameters change robot behavior.
- **Action**: Open [src/peripherals/peripherals/joystick_control.py](file:///media/pcarff/Workspaces/AnZym_Robot_System/anzym_rosorin/src/peripherals/peripherals/joystick_control.py) and locate:
  ```python
  self.declare_parameter('max_linear', 0.2) # meters per second
  ```
  Try changing `0.2` (slow) to `0.4` (faster) or `0.1` (super safe mode). Deploy changes with `./manage_workspace.sh deploy` and feel the difference on the gamepad!

### 🚀 Experiment 4: Autonomous "Return Home" and Obstacle Avoidance
- **Goal**: Test how the robot's artificial intelligence navigates back to its starting spot without hitting objects.
- **Action**: 
  1. Open the Ground Control Station at `http://localhost:5173`.
  2. Drive the robot forward into the room with the gamepad.
  3. Place a cardboard box in front of the robot.
  4. Click **"Return Home"** on the screen.
  5. Watch the robot's LiDAR detect the box, plan a curved path around it, and steer smoothly back to `(0, 0)`!
  6. Click **"Cancel Goal"** at any time to instantly stop the robot.

---

## 🎓 Happy Engineering!
You are now ready to explore, drive, and program the AnZym ROSOrin Robot! If you have questions or want to add new sensors, check the package documentation in `src/`.
