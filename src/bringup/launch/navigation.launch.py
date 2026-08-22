import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, GroupAction
from launch.conditions import IfCondition, UnlessCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    pkg_bringup = get_package_share_directory('bringup')
    pkg_nav2_bringup = get_package_share_directory('nav2_bringup')
    pkg_slam_toolbox = get_package_share_directory('slam_toolbox')

    default_params_file = os.path.join(pkg_bringup, 'config', 'nav2_params_ackermann.yaml')

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
    slam_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(pkg_slam_toolbox, 'launch', 'online_async_launch.py')),
        launch_arguments={
            'use_sim_time': use_sim_time,
        }.items(),
        condition=IfCondition(slam)
    )

    # 2. Nav2 Navigation Stack
    nav2_bringup_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(pkg_nav2_bringup, 'launch', 'navigation_launch.py')),
        launch_arguments={
            'use_sim_time': use_sim_time,
            'autostart': autostart,
            'params_file': params_file,
        }.items()
    )

    return LaunchDescription([
        declare_use_sim_time_cmd,
        declare_autostart_cmd,
        declare_params_file_cmd,
        declare_slam_cmd,
        declare_map_yaml_cmd,
        slam_launch,
        nav2_bringup_launch,
    ])
