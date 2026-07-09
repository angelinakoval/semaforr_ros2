import cv2
import numpy as np

class FeatureExtractor():
    def __init__(self, bbox_width=100, bbox_height=200):
        """
        Initialize the feature extractor

        Args:
            bbox_width: Width of the bounding box for feature extraction
            bbox_height: Height of the bounding box for feature extraction
        """
        self.bbox_width = bbox_width
        self.bbox_height = bbox_height


    def extract_features(self, image, center_x, center_y):
        """
        Extract features from the image at the specified pixel coordinates

        Args:
            image: Input image -> numpy array in BGR format
            center_x: X coordinate of the center of the bounding box
            center_y: Y coordinate of the center of the bounding box

        Returns:
            Normalized histogram vector
        """
       

    def compute_similarity(self, feature1, feature2):
        """
        Compute the similarity between two feature vectors using Bhattacharyya distance

        Args:
            feature1: First feature vector
            feature2: Second feature vector

        Returns:
            Similarity score (0.0 - 1.0), where 1.0 means identical and 0.0 means completely different
        """
        