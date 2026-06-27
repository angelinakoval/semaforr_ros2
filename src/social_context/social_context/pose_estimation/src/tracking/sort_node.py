import rclpy
from rclpy.node import Node
from geometry_msgs.msg import PoseArray, Pose
from .sort_tracker import SortTracker

class SortNode(Node):
    """
    ROS2 node for SORT tracking.

    Subscribes to: /human_poses_3d (PoseArray with 3D detections)
    Publishes to: /tracked_human_poses (PoseArray with tracked 3D positions)
    """

    def __init__(self):
        """
        Initialize the SORT node.
        """

        super().__init__('sort_node')

        # Parameters
        self.declare_parameter('distance_threshold', 2.0)
        self.declare_parameter('missed_threshold', 5)
        self.declare_parameter('hits_threshold', 3)

        distance_threshold = self.get_parameter('distance_threshold').value
        missed_threshold = self.get_parameter('missed_threshold').value
        hits_threshold = self.get_parameter('hits_threshold').value

        self.tracker = SortTracker(distance_threshold, missed_threshold, hits_threshold)

        self.pose_sub = self.create_subscription(
            PoseArray,
            'human_poses_3d',
            self.pose_callback,
            10
        )

        self.pose_pub = self.create_publisher(PoseArray, 'human_poses_3d_tracked', 10)

        self.get_logger().info('=' * 60)
        self.get_logger().info(f'SORT Node Ready!')
        self.get_logger().info(f'  Distance threshold: {distance_threshold}')
        self.get_logger().info(f'  Missed threshold: {missed_threshold}')
        self.get_logger().info(f'  Hits threshold: {hits_threshold}')
        self.get_logger().info(f'  Subscribing to: /human_poses_3d')
        self.get_logger().info(f'  Publishing to: /human_poses_3d_tracked')
        self.get_logger().info('=' * 60)

    
    def pose_callback(self, msg):
        """
        Callback for the /human_poses_3d topic.

        Args:
            msg: PoseArray from person_relative_localizer
                position.x
                position.y 
                position.z
        """
        
        detections = []
        for pose in msg.poses:
            x = pose.position.x
            y = pose.position.y
            detections.append((x, y)) 

        confirmed_tracks = self.tracker.update(detections)

        tracked_msg = PoseArray()
        tracked_msg.header = msg.header

        for track in confirmed_tracks:
            pose = Pose()
            pose.position.x = float(track['position'][0])
            pose.position.y = float(track['position'][1])
            pose.position.z = float(track['id'])
            tracked_msg.poses.append(pose)

        self.pose_pub.publish(tracked_msg)

        if confirmed_tracks:
            ids = [track['id'] for track in confirmed_tracks]
            self.get_logger().info(
                f'Tracking {len(confirmed_tracks)} person(s) with IDs: {ids}'
            )
        else:
            self.get_logger().debug('No confirmed tracks in this frame.')
    

def main(args=None):
    rclpy.init(args=args)
    node = SortNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()
    
if __name__ == '__main__':
    main()