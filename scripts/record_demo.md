# Demo recording shot list

I record this by hand, then upload and put the links in the README.
The paper shows an RViz outcome only for Task 1 (and terminal output for Task 3),
so there is no Task 2 video.

## Recording command (X11)
Run in a spare terminal, stop with Ctrl+C (or `q`). Check the screen size first with `xdpyinfo | grep dimensions`.
```bash
ffmpeg -f x11grab -framerate 30 -video_size 1920x1080 -i :1 \
  -c:v libx264 -preset veryfast -crf 23 -pix_fmt yuv420p media/task1.mp4
```
- `:1` is my `$DISPLAY`. Change `-video_size` to the real screen size.
- No audio in that command. Add `-f pulse -i default -c:a aac` before `media/task1.mp4`
  if I want to talk over it.
- `yuv420p` makes the mp4 play everywhere.
- Keep each clip short. Close other windows first.

## Task 1: TF frames + 2D Goal Pose (about 2 min)
1. Show the repo / package quickly, `./scripts/build.sh` already done.
2. `./scripts/run_task1.sh` (identity default). RViz opens, show map, odom, base_link
   all on top of each other at the origin.
3. In another terminal: `ros2 run tf2_ros tf2_echo map base_link` (shows 0, 0, 0).
4. Click **2D Goal Pose** at about (3, 2) with a 45 degree direction.
   `base_link` jumps to the goal and `odom` moves with it.
5. Show `tf2_echo map base_link` again, matches the goal. Optionally `tf2_echo map odom`.
6. Say in one sentence: only `map -> odom` changes, `odom -> base_link` stays constant,
   `T_map_odom = T_map_goal * inverse(T_odom_base)`.
7. Optional second clip: `params_file:=.../params_offset.yaml`, so odom and base_link can be told apart.

## Task 3: service (timeout) vs action (feedback, result, cancel)
Two terminals, `./scripts/run_task3.sh service` (about 10 s, then Ctrl+C) and
`./scripts/run_task3.sh action` (30 s). Show the TIMED OUT line and the server still finishing,
then the feedback counting down and the result. Optional: `ros2 run docking_comparison action_client cancel`.
