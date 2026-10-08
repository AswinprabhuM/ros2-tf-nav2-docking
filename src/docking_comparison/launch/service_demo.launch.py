from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(package='docking_comparison', executable='service_server', output='screen'),
        Node(package='docking_comparison', executable='service_client', output='screen'),
    ])
