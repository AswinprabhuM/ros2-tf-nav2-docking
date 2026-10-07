#!/usr/bin/env bash
# Run task 1 (tf_goal_localizer). Build first with scripts/build.sh.
set -e
cd "$(dirname "$0")/.."
source /opt/ros/${ROS_DISTRO:-jazzy}/setup.bash
source install/setup.bash
ros2 launch tf_goal_localizer tf_demo.launch.py "$@"
