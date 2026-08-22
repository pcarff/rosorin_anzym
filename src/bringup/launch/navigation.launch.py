import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, GroupAction
from launch.conditions import IfCondition, UnlessCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    pkg_bringup = get_package_share_directory('bringup')
    pkg_nav2_bringup = get_package_share_directory('nav2_bringup')
    pkg_slam_toolbox = get_package_share_directory('slam_toolbox')

    default_params_file = os.path.join(pkg_bringup, 'config', 'nav2_params_ackermann.yaml')
    default_slam_params_file = os.path.join(pkg_bringup, 'config', 'mapper_params_online_async.yaml')

    use_sim_time = LaunchConfiguration('use_sim_time')
    autostart = LaunchConfiguration('autostart')
    params_file = LaunchConfiguration('params_file')
    slam = LaunchConfiguration('slam')
    map_yaml_file = LaunchConfiguration('map')

    declare_use_sim_time_cmd = DeclareLaunchArgument(
        'use_sim_time',
        default_value='false',
        description='Use simulation clock if true'
    )

    declare_autostart_cmd = DeclareLaunchArgument(
        'autostart',
        default_value='true',
        description='Automatically startup the nav2 stack'
    )

    declare_params_file_cmd = DeclareLaunchArgument(
        'params_file',
        default_value=default_params_file,
        description='Full path to the ROS2 parameters file to use for all launched nodes'
    )

    declare_slam_cmd = DeclareLaunchArgument(
        'slam',
        default_value='true',
        description='Whether to run SLAM Toolbox for online map generation'
    )

    declare_map_yaml_cmd = DeclareLaunchArgument(
        'map',
        default_value='',
        description='Full path to map yaml file to load (when slam is false)'
    )

    # 1. SLAM Toolbox for dynamic online mapping and map -> odom TF
    slam_node = Node(
        package='slam_toolbox',
        executable='async_slam_toolbox_node',
        name='slam_toolbox',
        parameters=[
            default_slam_params_file,
            {'use_sim_time': use_sim_time, 'autostart': autostart}
        ],
        output='screen',
        condition=IfCondition(slam)
    )

    # Fallback static map -> odom transform if SLAM is disabled
    map_to_odom_static_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='map_to_odom_tf',
        arguments=['0', '0', '0', '0', '0', '0', 'map', 'odom'],
        condition=UnlessCondition(slam)
    )

    # 2. Nav2 Core Nodes
    controller_server_node = Node(
        package='nav2_controller',
        executable='controller_server',
        output='screen',
        respawn=False,
        parameters=[params_file],
        remappings=[('cmd_vel', 'cmd_vel_nav')]
    )

    smoother_server_node = Node(
        package='nav2_smoother',
        executable='smoother_server',
        name='smoother_server',
        output='screen',
        respawn=False,
        parameters=[params_file]
    )

    planner_server_node = Node(
        package='nav2_planner',
        executable='planner_server',
        name='planner_server',
        output='screen',
        respawn=False,
        parameters=[params_file]
    )

    behavior_server_node = Node(
        package='nav2_behaviors',
        executable='behavior_server',
        name='behavior_server',
        output='screen',
        respawn=False,
        parameters=[params_file]
    )

    velocity_smoother_node = Node(
        package='nav2_velocity_smoother',
        executable='velocity_smoother',
        name='velocity_smoother',
        output='screen',
        respawn=False,
        parameters=[params_file],
        remappings=[('cmd_vel', 'cmd_vel_nav')]
    )

    collision_monitor_node = Node(
        package='nav2_collision_monitor',
        executable='collision_monitor',
        name='collision_monitor',
        output='screen',
        respawn=False,
        parameters=[params_file]
    )

    bt_navigator_node = Node(
        package='nav2_bt_navigator',
        executable='bt_navigator',
        name='bt_navigator',
        output='screen',
        respawn=False,
        parameters=[params_file]
    )

    waypoint_follower_node = Node(
        package='nav2_waypoint_follower',
        executable='waypoint_follower',
        name='waypoint_follower',
        output='screen',
        respawn=False,
        parameters=[params_file]
    )

    lifecycle_manager_node = Node(
        package='nav2_lifecycle_manager',
        executable='lifecycle_manager',
        name='lifecycle_manager_navigation',
        output='screen',
        parameters=[{
            'autostart': autostart,
            'node_names': [
                'controller_server',
                'smoother_server',
                'planner_server',
                'behavior_server',
                'velocity_smoother',
                'collision_monitor',
                'bt_navigator',
                'waypoint_follower'
            ],
            'bond_timeout': 10.0,
            'attempt_respawn_reconnection': True
        }]
    )

    return LaunchDescription([
        declare_use_sim_time_cmd,
        declare_autostart_cmd,
        declare_params_file_cmd,
        declare_slam_cmd,
        declare_map_yaml_cmd,
        map_to_odom_static_tf,
        slam_node,
        controller_server_node,
        smoother_server_node,
        planner_server_node,
        behavior_server_node,
        velocity_smoother_node,
        collision_monitor_node,
        bt_navigator_node,
        waypoint_follower_node,
        lifecycle_manager_node,
    ])
