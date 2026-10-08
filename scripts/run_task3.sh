#!/usr/bin/env bash
# Run task 3 (docking_comparison). Build first with scripts/build.sh.
set -e
cd "$(dirname "$0")/.."
source /opt/ros/${ROS_DISTRO:-jazzy}/setup.bash
source install/setup.bash
# usage: ./scripts/run_task3.sh service   or   ./scripts/run_task3.sh action
ros2 launch docking_comparison ${1:-service}_demo.launch.py
