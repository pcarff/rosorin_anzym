# AI Coding Context: Cross-Compilation & Robot Deployment

## System Profile & Development Environment
- **Host Workstation Stack:** Ubuntu 24.04 LTS, VS Code IDE, ROS 2 Jazzy, Python 3.12, C++, Native Qt6 / QML Framework.
- **Target Vehicle Stack:** Ubuntu 24.04 LTS, Headless Deployment (No Display Server/GUI), ROS 2 Jazzy, Python 3.12, C++, Workspace: `~/anzym_robot_ws`.
- **Communication Architecture:** ROS2 Native (Jazzy). Remote vehicle communication utilizes Eclipse Zenoh over wireless links to minimize standard DDS discovery overhead.

## Project Directory Topology & Operational Targets
This is a unified workspace repository split explicitly by target deployment constraints. The folder architecture is mapped as follows:
- `/src/gcs_ui/` -> Ground Control Station user interface app (Qt6 / C++ / QML). **CRITICAL: NEVER DEPLOY TO ROBOT.**
- `/src/robot_nodes/` -> Vehicle drivers, flight controller interfaces, autonomy logic. **CRITICAL: DEPLOY TO ROBOT ONLY.**
- `/src/interfaces/` -> Custom ROS2 `.msg`, `.srv`, and `.action` definitions. Deployed to **BOTH** GCS and Robot.

---

## Strict AI Coding Rules

### 1. Architectural Target Separation
- **GUI Hard Boundary:** Never mix user interface elements, Qt headers (`#include <QObject>`), or QML types into any package located under `/src/robot_nodes/`. The robot runs a headless server environment.
- **Language Selection:** Write high-performance core math, transformation loops (`tf2`), and drivers in **C++**. Write high-level mission orchestration and quick scripts in **Python**.
- **Component Lifecycle:** When designing vehicle-side telemetry handlers, prioritize `rclcpp_lifecycle::LifecycleNode` structures to allow the GCS to dynamically transition, reset, or configure individual nodes.

### 2. Baseline-to-Custom Telemetry Patterns
- All vehicles must stream a universal baseline status set defined in `/src/interfaces/` (System health, Power tracking, GPS/Odom, Link quality, Failsafe states).
- If asked to add specific payloads (e.g., dynamic gimbals, thermal sensors, gas sniffing arrays), implement a plugin pattern or runtime topic subscription extension rather than mutating the universal baseline architecture.

### 3. Deployment Pipeline Awareness
- You have **no direct write or build capabilities** on the target robot. 
- Any time you write, refactor, or complete a feature inside `/src/robot_nodes/` or `/src/interfaces/`, you **must explicitly append a short reminder** at the end of your response telling the user to execute `./manage_workspace.sh deploy` to cross-compile the code onto the target vehicle.

---

## Terminal & SSH Execution Permissions
The user has configured a custom helper script (`manage_workspace.sh`) that automates environment routing and key authentication.
- **SSH Target Name:** Accessible via `ssh <robot_name>` using an identity file (`~/.ssh/id_ed25519`).
- **Build Strategy:** Local GCS code is built via `colcon build --packages-select gcs_ui`. Vehicle-side compilation is handled remotely via the deployment script invoking `colcon build --symlink-install` on the robot.

## How to Proceed
Acknowledge these architectural boundaries and rules in your next response before proposing or generating any code.
