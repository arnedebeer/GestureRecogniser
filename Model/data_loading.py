import os
import pickle
import re

import tensorflow as tf

import numpy as np
from typing import Tuple
from enum import Enum

import data_processing

class Hand(Enum):
    LEFT = "left_hand"
    RIGHT = "right_hand"

class GestureNames(Enum):
    SWIPE_LEFT = "swipe_left"
    SWIPE_RIGHT = "swipe_right"
    SWIPE_UP = "swipe_up"
    SWIPE_DOWN = "swipe_down"

    CIRCLE_CLOCKWISE = "clockwise"
    CIRCLE_COUNTER_CLOCKWISE = "counter_clockwise"

    TAP = "tap"
    DOUBLE_TAP = "double_tap"

    ZOOM_IN = "zoom_in"
    ZOOM_OUT = "zoom_out"

class LoadGestureException(Exception):
    pass

def load_gesture_samples(gesture_name: GestureNames, hand: Hand = Hand.RIGHT):
    result = []
    base_path = f"dataset/gestures/{gesture_name.value}/{hand.value}"
    folder_items = os.listdir(base_path)

    # Filter on the .pickle extension
    filtered_ext = list(filter(lambda x: re.search(r'\.pickle$', x) is not None, folder_items))

    if len(filtered_ext) == 0:
        # raise LoadGestureException("No gestures found in folder: %s" % base_path)
        return result

    for item in filtered_ext:
        r_match = re.match(r'candidate_(\w+).pickle$', item)
        if r_match is None:
            raise LoadGestureException("Incorrectly formatted data file name: %s" % item)

        candidate_id = r_match.group(1)
        with open(os.path.join(base_path, item), 'rb') as f:
            while True:
                try:
                    data_contents = pickle.load(f)

                    if isinstance(data_contents, dict):
                        result.append(data_contents)
                    else:
                        # Old data loader
                        data = {
                            'data': data_contents,
                            'gesture': gesture_name.value,
                            'candidate': candidate_id
                        }
                        result.append(data)
                except EOFError:
                    break
    return result

def load_gestures_grouped_per_candidate(use_left_hand: bool = True, use_right_hand: bool = True, split_candidate_per_hand: bool = True, gestures: list = GestureNames):
    if not use_left_hand and not use_right_hand:
        raise ValueError("At least one hand must be used")
    
    # This dictionary will store all candidates names as keys and their data, per gesture, as values
    result = dict()

    # We want to collect all the gestures
    for gesture in gestures:
        
        left_hand = []
        right_hand = []

        # These two are in the form of a list of dictionaries
        if use_left_hand:
            left_hand = load_gesture_samples(gesture_name=gesture, hand=Hand.LEFT)
        if use_right_hand:
            right_hand = load_gesture_samples(gesture_name=gesture, hand=Hand.RIGHT)

        # We want to combine the two hands, but only some used both hands
        for hand in [left_hand, right_hand]:
            for candidate in hand:
                
                candidate_id = candidate['candidate']

                if split_candidate_per_hand:
                    # Concatenate the candidate id and the hand used to get a unique id
                    # Use 'L' and 'R' to differentiate between the two hands
                    candidate_id += "_" + candidate['hand'][0].upper()
                    
                if candidate_id not in result:
                    result[candidate_id] = dict()

                if gesture.value not in result[candidate_id]:
                    result[candidate_id][gesture.value] = []

                result[candidate_id][gesture.value].append(candidate['data'])

    return result

def get_data_and_labels_from_candidates(candidates: list, all_data: dict, input_shape: tuple) -> Tuple[np.ndarray, np.ndarray]:
    '''
    Args:
        candidates: A list of candidate ids to pick from the all_data dictionary
        all_data: A dictionary containing all the needed data for all the candidates and gestures

    Returns:
        A tuple containing a numpy array of the data and a numpy array of the labels
    '''

    data = []
    labels = []
    for candidate in candidates:
        for gesture in all_data[candidate]:
            for sample in all_data[candidate][gesture]:
                
                # data_visualisation.plot_data_as_image(sample, label=f"{gesture} - Before Pre-processing")
                # data_visualisation.plot_data_as_graph(sample, title=f"{gesture} - Before Pre-processing")

                # We can pre-process the data here
                sample = data_processing.preprocess_data(sample)

                # data_visualisation.plot_data_as_image(sample, label=f"{gesture} - After preprocessing")
                # data_visualisation.plot_data_as_graph(sample, title=f"{gesture} - After preprocessing")

                try:
                    sample = np.reshape(sample, input_shape)
                except ValueError:
                    print(f"Could not reshape sample with shape {sample.shape} to {input_shape}")
                    print(f"Candidate: {candidate}, Gesture: {gesture}")
                    raise

                data.append(sample)
                labels.append(gesture)

    return np.array(data), np.array(labels, dtype=str)

def map_labels_to_integers(labels: list) -> list:
    """
    Maps the feature labels to integers. Useful for the loss function during training.

    Args:
        labels (list): List of labels to map to integers.

    Returns:
        list: List of integers mapped to the labels.
    """

    gesture_classes = [gesture_name.value for gesture_name in GestureNames]
    mapped_labels = np.array([gesture_classes.index(label) for label in labels])
    
    return mapped_labels

def map_labels_to_one_hot_encoding(labels: list) -> list:
    """
    Maps the feature labels to one hot encoding. Useful for the loss function during training.

    Args:
        labels (list): List of labels to map to one hot encoding.

    Returns:
        list: List of one hot encoded labels.
    """

    gesture_classes = [gesture_name.value for gesture_name in GestureNames]
    mapped_labels = np.array([gesture_classes.index(label) for label in labels])
    one_hot_encoded_labels = tf.keras.utils.to_categorical(mapped_labels, num_classes=len(gesture_classes))
    
    return one_hot_encoded_labels