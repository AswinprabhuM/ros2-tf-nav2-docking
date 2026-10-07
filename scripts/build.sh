#!/usr/bin/env bash
# Build the whole workspace from the repo root.
set -e
cd "$(dirname "$0")/.."
source /opt/ros/${ROS_DISTRO:-jazzy}/setup.bash
colcon build --symlink-install "$@"
