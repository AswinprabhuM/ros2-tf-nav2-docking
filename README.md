# ROS 2 C++ Interview Submission

Target distro: **ROS 2 Jazzy** (C++17).

## Overview
This repo has my answers to the three coding questions. Everything is C++17 and
built with colcon on ROS 2 Jazzy (Ubuntu 24.04).

| Task | Package | Status |
|------|---------|--------|
| 1. TF frames + 2D Goal Pose | `tf_goal_localizer` | done |
| 2. Nav2 controller plugin | `p_controller_plugin` | todo |
| 3. Action vs Service | `docking_interfaces`, `docking_comparison` | todo |

## Build
```bash
./scripts/build.sh
source install/setup.bash
```

## Task 1 — TF frames + 2D Goal Pose (`tf_goal_localizer`)

One node that publishes the tree `map -> odom -> base_link` at 10 Hz. When a goal
comes in on `/goal_pose`, `base_link` moves to that goal.

**The idea:** I don't move `base_link` directly. `odom -> base_link` stays constant,
and I only change `map -> odom`. This is what a real robot does: odometry is smooth
and drifts, and localization fixes the drift by correcting `map -> odom`.

The chain is `T_map_base = T_map_odom * T_odom_base`. I want `T_map_base` to be the
goal, so:

```
T_map_odom = T_map_goal * inverse(T_odom_base)
```

I use `tf2::Transform` and `tf2::Quaternion` for this, so the full orientation is
handled, not just yaw.

**Parameters** (`config/params.yaml`)
- `odom_base_x`, `odom_base_y`, `odom_base_yaw`: the constant `odom -> base_link`
  transform. The default is 0, which is identity as the question asks, so before a goal
  all three frames sit on top of each other.
- `config/params_offset.yaml` is an optional demo with x=1.0, y=0.5, so `odom` and
  `base_link` can be told apart in RViz. The code is the same either way.

**Run it** (opens RViz too)
```bash
./scripts/run_task1.sh
```
Click **2D Goal Pose** in RViz. `base_link` jumps to the click, and `odom` moves with it.
For the offset demo:
```bash
./scripts/run_task1.sh params_file:=$PWD/src/tf_goal_localizer/config/params_offset.yaml
```

**Check it without RViz**
```bash
ros2 run tf_goal_localizer tf_goal_localizer_node --ros-args --params-file src/tf_goal_localizer/config/params.yaml
ros2 topic pub --once /goal_pose geometry_msgs/msg/PoseStamped \
  "{header: {frame_id: map}, pose: {position: {x: 3.0, y: 2.0}, orientation: {z: 0.3826834, w: 0.9238795}}}"
ros2 run tf2_ros tf2_echo map base_link
```
Goal (3, 2, yaw = pi/4) gives:
```
Translation: [3.000, 2.000, 0.000]
Rotation: in RPY (degree) [0.000, -0.000, 45.000]
```
`map -> odom` comes out as (2.646, 0.939) with 45 degrees. That matches the hand
calculation: (3, 2) + rotate45(-1, -0.5).

A goal whose `frame_id` is not `map` is ignored with a warning.

## Task 2 — Nav2 P-controller plugin (`p_controller_plugin`)
TODO: run, expected output

## Task 3 — Action vs Service (`docking_interfaces`, `docking_comparison`)
TODO: run, expected output. See [docs/ACTION_VS_SERVICE.md](docs/ACTION_VS_SERVICE.md)

## Video
TODO

## Docs
- [docs/PLAN.md](docs/PLAN.md)
- [docs/WALKTHROUGH.md](docs/WALKTHROUGH.md)
