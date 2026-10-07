#!/usr/bin/env bash
# Run task 2 (p_controller_plugin). Build first with scripts/build.sh.
set -e
cd "$(dirname "$0")/.."
source /opt/ros/${ROS_DISTRO:-jazzy}/setup.bash
source install/setup.bash
echo 'TODO: launch Nav2 with params/p_controller.yaml'
