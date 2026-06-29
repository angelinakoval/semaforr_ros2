import cv2
import numpy as np

class FeatureExtractor():
    def __init__(self, bbox_width=100, bbox_height=200, h_bins = 16, s_bins = 8):
        """
        Initialize the feature extractor

        Args:
            bbox_width: Width of the bounding box for feature extraction
            bbox_height: Height of the bounding box for feature extraction
            h_bins: Number of bins for the Hue channel in HSV color space
            s_bins: Number of bins for the Saturation channel in HSV color space
        """
        self.bbox_width = bbox_width
        self.bbox_height = bbox_height
        self.h_bins = h_bins
        self.s_bins = s_bins

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
        if image is None or image.size == 0:
            return None

        # Define the bounding box
        x1 = int(center_x - self.bbox_width / 2)
        y1 = int(center_y - self.bbox_height / 2)
        x2 = int(center_x + self.bbox_width / 2)
        y2 = int(center_y + self.bbox_height / 2)

        # Ensure the bounding box is within image boundaries
        x1 = max(0, x1)
        y1 = max(0, y1)
        x2 = min(image.shape[1], x2)
        y2 = min(image.shape[0], y2)

        if x1 >= x2 or y1 >= y2:
            return None

        # Crop the region of interest
        crop = image[y1:y2, x1:x2]

        # Convert BGR to HSV
        hsv = cv2.cvtColor(crop, cv2.COLOR_BGR2HSV)

        # Compute the histogram for H and S channels
        hist = cv2.calcHist([hsv], [0, 1], None, [self.h_bins, self.s_bins], [0, 180, 0, 256])
        hist = cv2.normalize(hist, hist).flatten()

        return hist.flatten()

    def compute_similarity(self, feature1, feature2):
        """
        Compute the similarity between two feature vectors using Bhattacharyya distance

        Args:
            feature1: First feature vector
            feature2: Second feature vector

        Returns:
            Similarity score (0.0 - 1.0), where 1.0 means identical and 0.0 means completely different
        """
        if feature1 is None or feature2 is None:
            return 0.0

        if len(feature1) != len(feature2):
            return 0.0

        h1 = feature1.reshape(self.h_bins, self.s_bins)
        h2 = feature2.reshape(self.h_bins, self.s_bins)

        similarity = 1.0 - cv2.compareHist(h1, h2, cv2.HISTCMP_BHATTACHARYYA)

        return similarity
