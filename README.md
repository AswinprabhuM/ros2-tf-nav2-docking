# ROS 2 C++ Interview Submission

Target distro: **ROS 2 Jazzy** (C++17).

## Overview
This repo has my answers to the three coding questions. Everything is C++17 and
built with colcon on ROS 2 Jazzy (Ubuntu 24.04).

| Task | Package | Status |
|------|---------|--------|
| 1. TF frames + 2D Goal Pose | `tf_goal_localizer` | done |
| 2. Nav2 controller plugin | `p_controller_plugin` | done |
| 3. Action vs Service | `docking_interfaces`, `docking_comparison` | done |

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

A Nav2 controller plugin (`p_controller_plugin::PController`, inherits
`nav2_core::Controller`) that follows the global plan with a simple P controller.

**Strategy:** drive at a constant linear speed towards the nearest unvisited waypoint
and steer with `angular_vel = kp * heading_error`, clamped to `max_angular_vel`.
A waypoint counts as visited when the robot is within `waypoint_tolerance` of it.
When all waypoints are visited the command is zero.

**Lifecycle:** the node is kept as a `weak_ptr`. Parameters are declared and read in
`configure()`, `cleanup()` resets the plan and the tf pointer. `setSpeedLimit()` scales
the linear speed.

**One thing that bit me:** `controller_server` gives the robot pose in the local costmap
frame (`odom`), but the plan is in `map`. The plugin transforms each waypoint into the
pose's frame with the tf buffer before using it. Without that the robot steers
towards the wrong point (I saw it drive into a wall). There is a unit test for it.

**Parameters** (under `FollowPath`, see `params/p_controller.yaml`)
- `kp`, `linear_vel`, `max_angular_vel`, `waypoint_tolerance`

**Files**
- Header and source: `include/p_controller_plugin/p_controller.hpp`, `src/p_controller.cpp`
  (ends with `PLUGINLIB_EXPORT_CLASS(p_controller_plugin::PController, nav2_core::Controller)`)
- Manifest: `plugin.xml`

`plugin.xml`
```xml
<library path="p_controller">
  <class type="p_controller_plugin::PController"
         base_class_type="nav2_core::Controller">
    <description>Proportional heading controller</description>
  </class>
</library>
```

`CMakeLists.txt` (library and plugin registration)
```cmake
add_library(p_controller SHARED src/p_controller.cpp)
ament_target_dependencies(p_controller ${deps})

# installs plugin.xml and registers it under the nav2_core plugin group
pluginlib_export_plugin_description_file(nav2_core plugin.xml)

install(TARGETS p_controller
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
```

**How Nav2 finds the plugin:** `package.xml` has `<nav2_core plugin="${prefix}/plugin.xml" />`
in its export section, and the CMake line above installs `plugin.xml` and registers it
in the ament index. When `controller_server` reads `FollowPath.plugin: "p_controller_plugin::PController"`,
pluginlib looks up that class name in the registered manifests, loads `libp_controller.so`
(the `path` in `plugin.xml`) and creates the object.

**Unit tests**
```bash
colcon test --packages-select p_controller_plugin && colcon test-result --verbose
```
The gtest loads the plugin through pluginlib (like `controller_server` does) and checks
the output on a straight path: driving straight, steering back, clamping, visiting
waypoints, stopping at the end, speed limit and the `odom`/`map` frame case.

**Run it in Nav2:** put the `FollowPath` block from `params/p_controller.yaml` into the
`controller_server` section of your Nav2 params and launch Nav2 as usual (I used the
TurtleBot3 Gazebo sim from `nav2_bringup`, with the stock params and only `FollowPath`
replaced).

Checked without RViz: initial pose (-2, -0.5), `NavigateToPose` goal (0.5, 0.5) ends with
`SUCCEEDED` and the robot stops at about (0.27, 0.49), inside the 0.25 m goal tolerance.

## Task 3 — Action vs Service (`docking_interfaces`, `docking_comparison`)

"Go to charging station" takes 30 s. Version A is a service, version B is an action.
`docking_interfaces` has `GoToCharger.srv` and `GoToCharger.action`. Comparison and
explanations: [docs/ACTION_VS_SERVICE.md](docs/ACTION_VS_SERVICE.md).

**Run**
```bash
./scripts/run_task3.sh service   # server + client with a 5 s timeout
./scripts/run_task3.sh action    # server + client, 30 s with feedback
```
or the nodes by hand: `ros2 run docking_comparison service_server` / `service_client` / `action_server` / `action_client`.
`action_client cancel` cancels the goal after 5 s.

**Service output**
```
[client] Requesting charging station trip via SERVICE...
[client] Waiting for response (5s timeout)...
[client] TIMED OUT - service did not respond within 5 seconds!
```
The server keeps running and still "responds" after 30 s, nobody receives it.

**Action output**
```
[client] Goal accepted - receiving feedback...
[feedback] Distance remaining: 58.0 m
[feedback] Distance remaining: 56.0 m
...
[client] Result: Arrived at charging station! Travel time: 30.0s
```
The trip is 60 m at 2 m/s so it takes 30 s. Cancel prints `Goal canceled after 5.0s`.

## Video
Task 1 in RViz: [media/task1.mp4](media/task1.mp4)

All three frames start on top of each other at the origin. After a goal on `/goal_pose`
(first (3, 2) at 45 degrees, then (-2, 3) at 135 degrees), `odom` and `base_link` move
there and `map` stays put, because only `map -> odom` changes.

## Docs
- [docs/ACTION_VS_SERVICE.md](docs/ACTION_VS_SERVICE.md): service vs action, timeout behaviour, when to use which
