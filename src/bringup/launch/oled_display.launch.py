import os
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    # Hardware Driver Node (talks to STM32 board)
    driver_node = Node(
        package='ros_robot_controller',
        executable='ros_robot_controller_node',
        name='ros_robot_controller',
        output='screen',
    )

    # OLED Display Info Node (publishes IP, battery, temp, and hostname to OLED)
    oled_node = Node(
        package='bringup',
        executable='oled_info_node',
        name='oled_info_node',
        output='screen',
    )

    return LaunchDescription([
        driver_node,
        oled_node,
    ])
