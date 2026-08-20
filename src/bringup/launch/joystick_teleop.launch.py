import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    enable_camera_arg = DeclareLaunchArgument(
        'enable_camera',
        default_value='true',
        description='Enable 3D Depth / RGB Camera driver'
    )
    enable_lidar_arg = DeclareLaunchArgument(
        'enable_lidar',
        default_value='true',
        description='Enable 2D DToF LIDAR driver'
    )

    # 1. Base STM32 Hardware Driver Node (communicates over serial with STM32 board)
    driver_node = Node(
        package='ros_robot_controller',
        executable='ros_robot_controller_node',
        name='ros_robot_controller',
        output='screen',
    )

    # 2. Ackermann Kinematics & Motor/Steering Drive Node (converts cmd_vel into wheel motor speed & steering servo angle)
    motor_steering_node = Node(
        package='controller',
        executable='odom_publisher',
        name='odom_publisher',
        output='screen',
        parameters=[
            {'machine_type': 'ROSOrin_Acker',
             'pub_odom_topic': True}
        ]
    )

    # 3. Wireless Joystick Controller Node (maps wireless gamepad sticks to cmd_vel)
    joystick_node = Node(
        package='peripherals',
        executable='joystick_control',
        name='joystick_control',
        output='screen',
        parameters=[
            {'max_linear': 0.5,
             'max_angular': 2.0,
             'disable_servo_control': True}
        ]
    )

    # 3.4 Rosbridge WebSocket Server Node (for GCS network connection)
    rosbridge_node = Node(
        package='rosbridge_server',
        executable='rosbridge_websocket',
        name='rosbridge_websocket',
        output='screen',
        parameters=[{'port': 9090, 'address': '0.0.0.0'}]
    )

    rosapi_node = Node(
        package='rosapi',
        executable='rosapi_node',
        name='rosapi',
        output='screen'
    )

    # 3.5 Teleop Mode Switcher & Heartbeat Watchdog Node
    teleop_mode_switcher_node = Node(
        package='peripherals',
        executable='teleop_mode_switcher',
        name='teleop_mode_switcher',
        output='screen',
        parameters=[
            {'default_mode': 'LOCAL',
             'heartbeat_timeout_sec': 3.0}
        ]
    )

    # 4. OLED Info Display Node (shows live IP address, hostname, battery voltage, and CPU temp)
    oled_node = Node(
        package='bringup',
        executable='oled_info_node',
        name='oled_info_node',
        output='screen',
    )

    # 5. 3D Camera Launch Integration
    try:
        peripherals_path = get_package_share_directory('peripherals')
    except Exception:
        peripherals_path = os.path.expanduser('~/anzym_robot_ws/src/peripherals')

    camera_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(peripherals_path, 'launch', 'depth_camera.launch.py')
        ),
        condition=IfCondition(LaunchConfiguration('enable_camera'))
    )

    lidar_node = Node(
        package='ldlidar_stl_ros2',
        executable='ldlidar_stl_ros2_node',
        name='ldlidar_published',
        output='screen',
        parameters=[{
            'product_name': 'LDLiDAR_LD19',
            'port_name': '/dev/ttyCH341USB0',
            'port_baudrate': 230400,
            'frame_id': 'lidar_frame',
            'topic_name': 'scan',
            'laser_scan_dir': True,
            'enable_angle_crop_func': False
        }],
        condition=IfCondition(LaunchConfiguration('enable_lidar'))
    )

    lidar_tf_node = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='base_to_lidar_tf',
        arguments=['0.115', '0', '0.13', '0', '0', '0', 'base_link', 'lidar_frame']
    )

    # 6. Web Video Server for GCS streaming
    web_video_server_node = Node(
        package="web_video_server",
        executable="web_video_server",
        name="web_video_server",
        output="screen",
        parameters=[{
            "port": 8080,
            "address": "0.0.0.0"
        }]
    )

    # 7. Vehicle Core Telemetry & Failsafe Node
    telemetry_node = Node(
        package="vehicle_core",
        executable="telemetry_node",
        name="telemetry_node",
        output="screen",
    )

    return LaunchDescription([
        enable_camera_arg,
        enable_lidar_arg,
        driver_node,
        motor_steering_node,
        joystick_node,
        rosbridge_node,
        rosapi_node,
        teleop_mode_switcher_node,
        oled_node,
        camera_launch,
        lidar_node,
        lidar_tf_node,
        web_video_server_node,
        telemetry_node,
    ])
