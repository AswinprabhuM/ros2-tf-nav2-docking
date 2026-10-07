import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    share = get_package_share_directory('tf_goal_localizer')
    return LaunchDescription([
        Node(
            package='tf_goal_localizer',
            executable='tf_goal_localizer_node',
            parameters=[os.path.join(share, 'config', 'params.yaml')],
            output='screen',
        ),
        Node(
            package='rviz2',
            executable='rviz2',
            arguments=['-d', os.path.join(share, 'rviz', 'tf_demo.rviz')],
        ),
    ])
