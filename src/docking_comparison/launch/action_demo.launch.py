from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    # TODO: add the client once implemented (usually run in its own terminal for clear output).
    return LaunchDescription([
        Node(package='docking_comparison', executable='action_server', output='screen'),
    ])
