"""
This module implements the SORT (Simple Online and Realtime Tracking) algorithm for tracking multiple objects
"""

import numpy as np
from scipy.optimize import linear_sum_assignment
from .kalman_tracker import KalmanPersonTracker

class SortTracker():
    def __init__(self, distance_threshold = 2.0, missed_threshold = 5, hits_threshold = 3):
        """
        Initialize the SORT tracker

        Args: distance_threshold: Maximum distance to associate detections to existing trackers
              missed_threshold: Number of consecutive misses before a tracker is deleted
              hits_threshold: Number of consecutive hits before a tracker is confirmed
        """
        self.trackers = []
        self.distance_threshold = distance_threshold
        self.missed_threshold = missed_threshold
        self.hits_threshold = hits_threshold
        self.frame_count = 0


    def get_confirmed_tracks(self):
        """
        Get the list of confirmed tracks

        Returns:
            List of (id, position) for confirmed tracks
        """
        confirmed_tracks = []
        for track in self.trackers:
            if track.state == 'CONFIRMED':
                confirmed_tracks.append({'id': track.id, 'position': track.get_position(), 'history': track.history})
        return confirmed_tracks

    def update(self, detections):
        """
        Update the tracker with new detections.

        Args:
            detections: list of (x,y) from /human_poses_3d
        """
        self.frame_count += 1

        # Predict new locations of existing trackers
        predicted_positions = []
        for track in self.trackers:
            predicted_pos = track.predict()
            predicted_positions.append(predicted_pos)
        
        #No existing trackers, create new ones for all detections
        if len(self.trackers) == 0:
            for det in detections:
                self.trackers.append(KalmanPersonTracker(det))
            return self.get_confirmed_tracks()
        
        # No detections, mark all trackers as missed
        if len(detections) == 0:
            for track in self.trackers:
                track.mark_missed(self.missed_threshold)
            self.trackers = [t for t in self.trackers if t.state != 'DELETED']
            return self.get_confirmed_tracks()

        # Compute cost matrix (Euclidean distance for now)
        cost_matrix = np.zeros((len(predicted_positions), len(detections)))
        for i, pred in enumerate(predicted_positions):
            for j, det in enumerate(detections):
                cost_matrix[i, j] = np.linalg.norm(np.array(pred) - np.array(det))

        # Solve the assignment problem
        row_indices, col_indices = linear_sum_assignment(cost_matrix)

        accepted_assignments = set()

        # Update existing trackers or create new ones
        for i, j in zip(row_indices, col_indices):
            if cost_matrix[i, j] < self.distance_threshold:
                self.trackers[i].update(detections[j], self.hits_threshold)
                accepted_assignments.add(j)
            else:
                self.trackers[i].mark_missed(self.missed_threshold)

        # mark unmatched trackers as missed
        for i in range(len(self.trackers)):
            if i not in row_indices:
                self.trackers[i].mark_missed(self.missed_threshold)

        # Create new trackers for unmatched detections
        for j in range(len(detections)):
            if j not in accepted_assignments:
                self.trackers.append(KalmanPersonTracker(detections[j]))

        # Remove deleted trackers
        self.trackers = [t for t in self.trackers if t.state != 'DELETED']

        return self.get_confirmed_tracks()
