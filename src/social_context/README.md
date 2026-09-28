
# Social Context Package

Provides all nodes, functions, and models for the social context module in the Social-SemaFORR navigation system.

## Overview

The Social Context package enables robots to understand and predict human movement in dynamic environments using deep learning. It integrates with HuNavSim and SemaFORR to enhance socially-aware navigation.

## Features

- Deep learning-based human trajectory prediction
- ROS2 node for real-time social context updates
- Integration with HuNavSim and SemaFORR
- Configurable and extensible model support

## Pipeline

`social_context_pipeline_launch.py` starts the full perception chain, in order:

```text
camera image (/rgb_camera_frame_sensor/image_raw)
  -> pose_mediapipe (or pose_openpose)      2D pose detection      -> /human_poses
  -> person_relative_localizer              2D pose + scan -> 3D   -> local person poses
  -> global_human_localizer                 local -> map frame     -> /human_poses_3d
  -> sort_tracker                           frame-to-frame identity -> tracked people
  -> formation_detector                     clusters tracked people -> /formation_groups
  -> social_context_tracked                 trajectory prediction  -> /human_poses_3d_tracked_global,
                                                                       /pedestrian_predictions_tracked
```

These output topic names are the exact defaults SemaFORR's node already expects
(`social.input.tracked_people_topic`, `social.input.tracked_predictions_topic`,
`social.input.formations_topic` in `semaforr_node_component.cpp`) — no remapping
needed when running both packages together.

The launch file exposes `detector`, `camera_frame` (default
`rgb_camera_optical_frame`), `lidar_frame` (default `base_laser_link`), and
`map_frame` (default `map`) as arguments — e.g.
`ros2 launch social_context social_context_pipeline_launch.py camera_frame:=my_cam_frame`.
If your simulation or robot uses different TF frame names, override these;
otherwise `person_relative_localizer` and `global_human_localizer` will
silently fail to look up transforms and no detections will make it downstream.

## Build

From the workspace root (`~/semaforr_ros2`), not from inside this package:

```bash
source /opt/ros/humble/setup.bash
colcon build --packages-up-to semaforr social_context semaforr_bridge
source install/setup.bash
```

## Running a full simulation (Gazebo + HuNav + SemaFORR)

Each of these runs in its own terminal; source `/opt/ros/humble/setup.bash` and
`install/setup.bash` in every one of them first.

**1. Gazebo + HuNav + PMB2 simulation**

```bash
cd ~/hunavsim_containers/gazebo_classic
./run-hunav_gz_classic11_pmb2.bash
```

If this errors with something like `already in use by container ... "hunavsim_pmb2"`
(a previous crashed/killed session left the name taken, since the script's
`docker rm` only runs after a clean exit), clear it and re-run:
```bash
docker stop hunavsim_pmb2
docker rm hunavsim_pmb2
```

**2. Odometry -> pose bridge** (SemaFORR reads `/pose`, the sim publishes odometry)

```bash
ros2 run semaforr_bridge odom_to_pose_bridge
```

**3. Social context perception pipeline**

MediaPipe needs a downloaded pose_landmarker `.task` model bundle — it isn't
fetched automatically. One-time setup:

```bash
pip install mediapipe opencv-python

mkdir -p ~/models
curl -L -o ~/models/pose_landmarker_lite.task \
  https://storage.googleapis.com/mediapipe-models/pose_landmarker/pose_landmarker_lite/float16/1/pose_landmarker_lite.task
```

Heavier variants (`pose_landmarker_full`, `pose_landmarker_heavy`) exist at the
same path with the size swapped in, trading speed for accuracy — `lite` is
the default here and what `main_mediapipe()`
([camera_2d_pose_detection_node.py](social_context/pose_estimation/src/camera_2d_pose_detection_node.py))
expects a path to via the `MEDIAPIPE_POSE_MODEL_PATH` env var (it raises if
unset — there's no built-in default path).

Then, every time you launch:

```bash
export MEDIAPIPE_POSE_MODEL_PATH="$HOME/models/pose_landmarker_lite.task"
ros2 launch social_context social_context_pipeline_launch.py detector:=mediapipe
```

Add the `export` line to your shell rc file so it's set automatically in new
terminals.

Use `detector:=openpose` only if MediaPipe isn't available — OpenPose's facing
direction is a cruder 2D heuristic, so `approach_direction` will be less
reliable under it.

**4. SemaFORR**

```bash
ros2 launch semaforr stage_tutorial.launch.py use_sim_time:=true
```

## What to look at

Don't start by staring at raw topics. Answer these in order:

1. **Is perception actually producing data?**
   ```bash
   ros2 topic hz /human_poses_3d_tracked_global
   ros2 topic hz /formation_groups
   ```
   If these are silent, nothing downstream (SemaFORR's social advisors) can
   possibly fire — the problem is upstream in this package, not in SemaFORR.

2. **Is SemaFORR consuming it and deciding?**
   ```bash
   ros2 topic hz /decision_records
   ```

3. **Which tiers/components actually fired this run**, from `semaforr_msgs/DecisionRecord`
   (published on `/decision_records`):

   ```bash
   # Tier-1 veto blocked a candidate action (e.g. predicted_social_veto, avoid_obstacles)
   ros2 topic echo /decision_records | grep --line-buffered -E "^\s*rule: "

   # Tier-1 mandatory rule won the cycle (e.g. sudden_proximity_mandate, victory)
   ros2 topic echo /decision_records | grep --line-buffered -E "^\s*selected_policy: "

   # Tier-3 advisor actually scored candidates this cycle (only appears when it participates)
   ros2 topic echo /decision_records | grep --line-buffered -E "^\s*advisor: "

   # Per-component per-cycle outcome, the single most useful line:
   # e.g. "component=approach_direction,...,outcome=advisor_scored_continue"
   # vs. "component=formation_courtesy,...,outcome=advisor_not_applicable_continue"
   ros2 topic echo /decision_records | grep --line-buffered "decision_cycle:"
   ```

   Ignore any `tier1:...` / `advisor:...` (no space after the colon) strings you
   see in the output — those come from `component_manifest`, a static list of
   *registered* components copied unchanged into every record. They tell you
   what's loaded, not what fired. The lines above (with a space after the key,
   or the `decision_cycle:` summary) are the real per-cycle evidence.

4. **Full record for one cycle**, when you need the numbers behind a
   contribution (`raw_score`, `weighted_score`, `formation_evidence_available`, ...):
   ```bash
   ros2 topic echo /decision_records --once
   ```

## Recording a bag

Record the raw inputs, not the pipeline's own outputs — that way you can
replay through the perception pipeline and SemaFORR again later, exactly as
they'd run live, instead of being frozen to one code version's output shape.

```bash
ros2 bag record -o social_context_run \
  /rgb_camera_frame_sensor/image_raw \
  /rgb_camera_frame_sensor/camera_info \
  /scan_raw \
  /mobile_base_controller/odom \
  /tf \
  /tf_static \
  /clock \
  --use-sim-time
```

Add `/decision_records` too if you also want SemaFORR's own decisions captured
alongside the raw inputs from this same run (useful for later comparison
against a replay).

## Running a simulation on a recorded bag

You don't need Gazebo/HuNav running at all for this — the bag replaces the
simulator as the source of camera, scan, odometry, and TF data.

**1. Play the bag back, publishing simulated time**

```bash
ros2 bag play social_context_run --clock
```

**2. Run the bridge, pipeline, and SemaFORR against it**, each with
`use_sim_time:=true` (or `use_sim_time` set to `true` for nodes launched via
`ros2 run`, which need it as a parameter override):

```bash
ros2 run semaforr_bridge odom_to_pose_bridge --ros-args -p use_sim_time:=true
ros2 launch social_context social_context_pipeline_launch.py detector:=mediapipe
ros2 launch semaforr stage_tutorial.launch.py use_sim_time:=true
```

Everything from the "What to look at" section above applies identically here
— this is the reproducible way to test a change to `predicted_social_veto`,
`sudden_proximity_mandate`, `formation_courtesy`, or `approach_direction`
against the exact same recorded scenario, instead of a fresh, non-repeatable
Gazebo run every time.

## Testing SemaFORR without this package (HuNav ground truth)

To check whether a problem is in SemaFORR's decision logic or in this
package's perception, you can skip the whole camera/MediaPipe/tracker chain
and point SemaFORR straight at HuNav's ground-truth agent states instead. Edit
`social.input.mode` from `tracked` to `hunav` in
[config/semaforr.yaml](../semaforr/config/semaforr.yaml), rebuild
(`colcon build --packages-select semaforr`), and just run the Gazebo/HuNav sim
+ SemaFORR — no bridge or pipeline launch needed, since SemaFORR then reads
`/human_states` directly.

This exercises `predicted_social_veto`, `sudden_proximity_mandate`, and
`approach_direction` (HuNav agents carry a real yaw, converted to `facing`)
against perfect, noise-free positions. It does **not** exercise
`formation_courtesy` — formations only come from this package's
`formation_detector`, HuNav has no equivalent. Revert `social.input.mode` to
`tracked` afterward.

## Troubleshooting

- **`Package 'semaforr_bridge' not found`**: the terminal hasn't sourced
  `install/setup.bash` in this workspace. Every terminal needs both
  `source /opt/ros/humble/setup.bash` and `source ~/semaforr_ros2/install/setup.bash`.
- **`SemaFORR startup failed: configuration: unknown Tier-1 rule '...'`**:
  a Tier-1 rule name is missing from `tier_one_order` in
  `src/config/navigation_configuration.cpp` — a second, independent allowlist
  from the actual rule registry. Only relevant if you're adding a new Tier-1
  rule yourself; rebuild after fixing.
- **A `pose transform unavailable ... extrapolation into the past` warning at
  startup**: benign and self-resolving — TF catching up right after the node
  and the simulator both come up, not a sign anything is broken.
- **`formation_courtesy` never participates**: check `/formation_groups`
  directly (`ros2 topic echo /formation_groups`) — its `confidence` field
  must clear `minimum_formation_confidence` (default `0.5` in
  `FormationCourtesyAdvisorConfiguration`) before SemaFORR will use it, even
  if `formation_detector` is publishing groups.
- **No detections at all from MediaPipe**: confirm `MEDIAPIPE_POSE_MODEL_PATH`
  is set in *this* terminal (it's not persisted unless you added it to your
  shell rc), and that `camera_frame`/`lidar_frame` match your simulation's
  actual TF frame names.

## Known version constraints

- **`numpy` must stay below `2.0`.** MediaPipe 0.10.x and `opencv-python`
  wheels from this era are built against the numpy 1.x C API; numpy 2.0's ABI
  break can silently corrupt or crash pose detection. A stray
  `pip install --upgrade numpy` pulled in by an unrelated package is the usual
  way this happens. Verified working combination on this setup: `numpy 1.26.4`,
  `mediapipe 0.10.5`, `opencv-python 4.13.0`.
- **Python 3.8–3.11 for MediaPipe 0.10.5.** No wheels exist for 3.12+ at that
  version. ROS 2 Humble's system Python (3.10) is within range.
- **`trajectory_prediction/requirements.txt` pins an old `torch==1.12.1`**
  from the original GST research code, but that's not what actually runs the
  ROS node — `social_context_tracked` executes under the same Python
  `ros2 launch` uses (system `python3`, not the venv below), and the
  checkpoint loader (`wrapper.py`, `torch.load(..., weights_only=False)`)
  has been confirmed working against a much newer `torch 2.13.0` already
  installed there. Only build the dedicated venv below if you need exact
  reproducibility with the original GST benchmark scripts
  (`trajectory_prediction/test.py`, `pec_net`, `mgnn`) — it is not required
  just to run the live pipeline.

## Virtual Environment & Dependencies

Running the social context model's original research/benchmark scripts
(not the live ROS pipeline — see above) requires specific pinned Python
dependencies.

1. Create a Python 3.10 virtual environment (recommended: conda).
2. Install dependencies:

	 ```bash
	 pip install -r src/social_context/social_context/trajectory_prediction/requirements.txt
	 ```
