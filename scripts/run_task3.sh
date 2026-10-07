#!/usr/bin/env bash
# Run task 3 (docking_comparison). Build first with scripts/build.sh.
set -e
cd "$(dirname "$0")/.."
source /opt/ros/${ROS_DISTRO:-jazzy}/setup.bash
source install/setup.bash
ros2 launch docking_comparison service_demo.launch.py
