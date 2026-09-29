"""
Launches the social context pipeline, which consists of the following nodes:
    pose_mediapipe (or pose_openpose)
      -> person_relative_localizer
      -> global_human_localizer
      -> sort_tracker
      -> formation_detector
      -> social_context_tracked
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def _launch_nodes(context):
    detector = LaunchConfiguration("detector").perform(context)
    camera_frame = LaunchConfiguration("camera_frame").perform(context)
    lidar_frame = LaunchConfiguration("lidar_frame").perform(context)
    map_frame = LaunchConfiguration("map_frame").perform(context)

    detector_executable = "pose_mediapipe" if detector == "mediapipe" else "pose_openpose"

    pose_node = Node(
        package="social_context",
        executable=detector_executable,
        name="camera_2d_pose_detection_node",
        output="screen",
        parameters=[{"camera_frame_id": camera_frame}],
    )
    localizer_node = Node(
        package="social_context",
        executable="person_relative_localizer",
        name="person_relative_localizer",
        output="screen",
        parameters=[
            {
                "camera_frame": camera_frame,
                "lidar_frame": lidar_frame,
                "output_frame": lidar_frame,
            }
        ],
    )
    global_localizer_node = Node(
        package="social_context",
        executable="global_human_localizer",
        name="global_human_localizer",
        output="screen",
        parameters=[
            {
                "robot_frame": lidar_frame,
                "map_frame": map_frame,
            }
        ],
    )
    sort_tracker_node = Node(
        package="social_context",
        executable="sort_tracker",
        name="sort_tracker",
        output="screen",
    )
    formation_detector_node = Node(
        package="social_context",
        executable="formation_detector",
        name="formation_detector",
        output="screen",
    )
    prediction_node = Node(
        package="social_context",
        executable="social_context_tracked",
        name="social_context_tracked",
        output="screen",
    )

    return [
        pose_node,
        localizer_node,
        global_localizer_node,
        sort_tracker_node,
        formation_detector_node,
        prediction_node,
    ]


def generate_launch_description():
    return LaunchDescription(
        [
            DeclareLaunchArgument(
                "detector",
                default_value="mediapipe",
                choices=["mediapipe", "openpose"],
                description="2D pose detector backend",
            ),
            DeclareLaunchArgument(
                "camera_frame",
                default_value="rgb_camera_optical_frame",
            ),
            DeclareLaunchArgument(
                "lidar_frame",
                default_value="base_laser_link",
            ),
            DeclareLaunchArgument(
                "map_frame",
                default_value="map",
            ),
            OpaqueFunction(function=_launch_nodes),
        ]
    )
