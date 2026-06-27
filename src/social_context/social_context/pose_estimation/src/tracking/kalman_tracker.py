"""
Implements Kalman Filtering for 1 person
Remembers where a person was
Predicts where they will next frame
Needs position and velocity - [x, y, vx, vy]

"""

import numpy as np
from filterpy.kalman import KalmanFilter

class KalmanPersonTracker():
    count = 0

    def __init__(self, initial_position):
        """
        New person is detected
        Set up Kalman Filter for this person and 

        Args: 
            initial-position: (x, y)
        """

        self.kf = KalmanFilter(dim_x=4, dim_z=2)

        # Initialize state [x, y, vx=0, vy=0]
        x, y = initial_position
        self.kf.x = np.array([[x], [y], [0.], [0.]])
        
        # State transition matrix
        # next_x  = x + vx
        # next_y  = y + vy
        # next_vx = vx
        # next_vy = vy
        self.kf.F = np.array([[1., 0., 1., 0.],
                                [0., 1., 0., 1.],
                                [0., 0., 1., 0.],
                                [0., 0., 0., 1.]])
        
        # Measurement matrix - we only measure position (x, y)
        # measured_x = x
        # measured_y = y
        self.kf.H = np.array([[1., 0., 0., 0.],
                                [0., 1., 0., 0.]])
        

        
        # Process noise covariance
        self.kf.Q = np.eye(4) * 0.1
        
        # Measurement noise covariance
        self.kf.R = np.eye(2) * 0.1
        
        # Initial state covariance
        self.kf.P = np.eye(4) * 1.0
        
    
        self.initial_position = initial_position

        KalmanPersonTracker.count += 1
        self.id = KalmanPersonTracker.count

        self.history = [initial_position]
        self.missed = 0
        self.hits = 1
        self.state = 'TENTATIVE'  # Can be 'TENTATIVE', 'CONFIRMED' (3 hits for now), 'DELETED'

    def predict(self):
        """
        Predict the next position of the person based on the Kalman Filter

        Returns:
            predicted_position: (x, y)
        """

        self.kf.predict()
        predicted_x = self.kf.x[0, 0]
        predicted_y = self.kf.x[1, 0]

        return (predicted_x, predicted_y)

    def update (self, new_position, hits_threshold = 3):
        """
        Update the Kalman Filter with a new position measurement

        Args:
            new_position: (x, y)
        """
        self.kf.update(np.array([[new_position[0]], [new_position[1]]]))
        self.history.append(new_position)
        self.missed = 0
        self.hits += 1

        # Threshold to confirm a track - if we have 3 hits, we consider it confirmed (COULD BE CHANGED)
        if self.state == 'TENTATIVE' and self.hits >= hits_threshold:
            self.state = 'CONFIRMED'


    def mark_missed(self, missed_threshold = 5):
        """
        Mark the person as missed

        Args:
            missed_threshold: Number of consecutive misses before a tracker is deleted
        """
        self.missed += 1
        self.hits = 0  # reset hits if we miss a detection
        if self.state == 'TENTATIVE' and self.missed >= 3:
            self.state = 'DELETED'
        elif self.missed >= missed_threshold:
            self.state = 'DELETED'  


    def get_position(self):
        """
        Get the current position of the person

        Returns:
            position: (x, y)
        """
        x = self.kf.x[0, 0]
        y = self.kf.x[1, 0]
        return (x, y)
