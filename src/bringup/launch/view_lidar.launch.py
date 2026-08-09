import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    try:
        peripherals_path = get_package_share_directory('peripherals')
    except Exception:
        peripherals_path = os.path.expanduser('~/anzym_robot_ws/src/peripherals')

    rviz_config_file = os.path.join(peripherals_path, 'rviz', 'lidar_view.rviz')

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2_lidar_view',
        arguments=['-d', rviz_config_file],
        output='screen'
    )

    return LaunchDescription([
        rviz_node
    ])
