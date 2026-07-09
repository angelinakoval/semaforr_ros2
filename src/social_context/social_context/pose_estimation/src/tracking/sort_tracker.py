"""
This module implements the SORT (Simple Online and Realtime Tracking) algorithm for tracking multiple objects
"""

import numpy as np
from scipy.optimize import linear_sum_assignment
from .kalman_tracker import KalmanPersonTracker

class SortTracker():
    def __init__(self, distance_threshold = 2.0, missed_threshold = 15, hits_threshold = 2):
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
                confirmed_tracks.append({
                    'id': track.id, 
                    'position': track.get_position(), 
                    'history': track.history,
                    'confidence': track.confidence,
                    'orientation': track.orientation
                    })
        return confirmed_tracks


    def is_ambiguous(self, item1, item2, threshold=1.5):
        """
        Helper function
        Check if two detections or track predictions are close enough to cause matching confusion

        Args:
            item1 (detection or prediction): (x, y)
            item2 (detection or prediction): (x, y)
            threshold: distance below which matching becomes ambiguous

        Returns:
            True if appearance features are needed
            False - no ambiguity, can match based on distance alone
        """
        pos1 = np.array([item1[0], item1[1]])
        pos2 = np.array([item2[0], item2[1]])
        distance = np.linalg.norm(pos1 - pos2)
        return distance < threshold


    def get_ambiguous_items(self, detections, predicted_positions, threshold=1.5):
        """
        Get detections that are close enough to cause matching confusion

        Args:
            detections: list of (x, y)
            predicted_positions: list of (x, y)
            threshold: distance below which matching becomes ambiguous

        Returns:
            Set of detection indices for ambiguous pairs
        """
        ambiguous_detections = set()

        for i in range (len(detections)):
            for j in range(i+1, len(detections)):
                if self.is_ambiguous(detections[i], detections[j], threshold):
                    ambiguous_detections.add(i)
                    ambiguous_detections.add(j)

        # Adds detections that are near the ambiguous predictions
        for i in range (len(predicted_positions)):
            for j in range(i+1, len(predicted_positions)):
                if self.is_ambiguous(predicted_positions[i], predicted_positions[j], threshold):
                    for k in range(len(detections)):
                        if self.is_ambiguous(predicted_positions[i], detections[k], threshold) or self.is_ambiguous(predicted_positions[j], detections[k], threshold):
                            ambiguous_detections.add(k)

        return ambiguous_detections
    

    def match_positions(self, predicted_positions, positions, confidences, orientations):
        """
        Match predicted positions of existing trackers to new detections using the Hungarian algorithm

        Args:
            predicted_positions: list of (x, y) from existing trackers
            positions: list of (x, y) from new detections
            confidences: list of confidence values for the new detections

        Returns:
            accepted_assignments: set of indices of detections that were matched to existing trackers
        """
        cost_matrix = np.zeros((len(predicted_positions), len(positions)))
        for i, pred in enumerate(predicted_positions):
            for j, pos in enumerate(positions):
                cost_matrix[i, j] = np.linalg.norm(np.array(pred) - np.array(pos))

        # Solve the assignment problem
        row_indices, col_indices = linear_sum_assignment(cost_matrix)

        accepted_assignments = set()

        # Update existing trackers or create new ones
        for i, j in zip(row_indices, col_indices):
            if cost_matrix[i, j] < self.distance_threshold:
                self.trackers[i].update(
                    positions[j], confidences[j], 
                    self.hits_threshold, orientations[j])
                accepted_assignments.add(j)
            else:
                self.trackers[i].mark_missed(self.missed_threshold)

        # mark unmatched trackers as missed
        for i in range(len(self.trackers)):
            if i not in row_indices:
                self.trackers[i].mark_missed(self.missed_threshold)
        
        return accepted_assignments
    

    def match_appearance(self, predicted_positions, positions, confidences, ambiguous_detections, features):
        pass
    

    def update(self, detections):
        """
        Update the tracker with new detections.

        Args:
            detections: list of (x,y,confidence) from /human_poses_3d
        """
        self.frame_count += 1

        positions = [det[:2] for det in detections]
        confidences = [det[2] for det in detections]
        orientations = [det[3] for det in detections]

        predicted_positions = []
        for track in self.trackers:
            predicted_pos = track.predict()
            predicted_positions.append(predicted_pos)
        
        #No existing trackers, create new ones for all detections
        if len(self.trackers) == 0:
            for i, pos in enumerate(positions):
                new_tracker = KalmanPersonTracker(pos)
                new_tracker.confidence = confidences[i]
                new_tracker.orientation = orientations[i]
                self.trackers.append(new_tracker)
            return self.get_confirmed_tracks()
        
        # No detections, mark all trackers as missed
        if len(detections) == 0:
            for track in self.trackers:
                track.mark_missed(self.missed_threshold)
            self.trackers = [t for t in self.trackers if t.state != 'DELETED']
            return self.get_confirmed_tracks()

        
        ambiguous_detections = self.get_ambiguous_items(positions, predicted_positions)

        if len(ambiguous_detections) == 0:
            accepted_assignments = self.match_positions(predicted_positions, positions, confidences, orientations)
        else:
            accepted_assignments = self.match_positions(predicted_positions, positions, confidences, orientations)

        # Create new trackers for unmatched detections
        for j in range(len(detections)):
            if j not in accepted_assignments:
                new_tracker = KalmanPersonTracker(positions[j])
                new_tracker.confidence = confidences[j]
                new_tracker.orientation = orientations[j]
                self.trackers.append(new_tracker)

        # Remove deleted trackers
        self.trackers = [t for t in self.trackers if t.state != 'DELETED']

        return self.get_confirmed_tracks()
    
