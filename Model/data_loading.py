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

# def load_all_pickled_data(file_path) -> list:
#     """Loads all pickled data from a given file path.
    
#     Args:
#         file_path (str): Path to the pickled data.
    
#     Returns:
#         List: List of all the pickled data.
#     """
#     data = []
#     with open(file_path, 'rb') as pickle_file:
#         # data_dict = {}
#         while True:
#             try:
#                 # data_dict = pickle.load(pickle_file)
#                 # print(data_dict, flush=True)
#                 # data.append(data_dict.get("data"))
#                 data.append(pickle.load(pickle_file))
#                 # data_dict.append(pickle.load(pickle_file))
#             except EOFError:
#                 break
        
#         # data_dict.get("data")
#         # data.append(data_dict.get("data"))
#     # print(data)
#     return data

# def load_and_combine_candidates(gesture_folder_path: str, sample_length: int) -> np.ndarray:
#     """Loads all pickled data from the given gesture (including hand used) folder path and puts every candidate's data into an ndarray

#     Args:
#         gesture_folder_path (str): Path to the folder containing the pickled data.
#         sample_length (int): Length of the samples.

#     Returns:
#         np.ndarray: Array containing all the candidates' data.
#     """

#     candidates = os.listdir(gesture_folder_path)
#     candidates_data = np.empty((0, sample_length, 3), dtype=np.float32)
#     for candidate in candidates:
#         # Skip non-candidate files
#         if not "candidate_" in candidate or not candidate.endswith(".pickle"):
#             continue
        
#         pickle_file_path = os.path.join(gesture_folder_path, candidate)
#         candidate_data = load_all_pickled_data(pickle_file_path)
#         candidates_data = np.append(candidates_data, candidate_data, axis=0)

#     # candidates_data = np.array(candidates_data)
#     # print(candidates_data)
#     # candidates_data = candidates_data.reshape(-1, sample_length, 3)
#     # print(candidates_data)
#     return candidates_data

# def load_and_combine_hands(data_root_path: str = os.path.join("data")) -> Tuple[np.ndarray, np.ndarray]:
#     """
#     From the root path, load all gestures, combining all left and right hands, and return the data and labels.

#     Args:
#         data_root_path (str, optional): Root path of the data. Defaults to os.path("data").

#     Returns:
#         tuple[np.ndarray, np.ndarray]: Data and labels.
#     """

#     gestures = os.listdir(data_root_path)
#     data = []
#     labels = []
#     for gesture in gestures:
#         # Skip non-gesture folders
#         if not os.path.isdir(os.path.join(data_root_path, gesture)):
#             continue
        
#         # Load all candidates' data for the gesture using the left hand
#         left_hand_folder_path = os.path.join(data_root_path, gesture, "left_hand")
#         left_hand_data = None
#         if os.path.exists(left_hand_folder_path):
#             left_hand_data = load_and_combine_candidates(left_hand_folder_path, sample_length=100)

#         right_hand_folder_path = os.path.join(data_root_path, gesture, "right_hand")
#         right_hand_data = None
#         if os.path.exists(right_hand_folder_path):
#             right_hand_data = load_and_combine_candidates(right_hand_folder_path, sample_length=100)

#         combined_data = left_hand_data
#         if right_hand_data is not None:
#             combined_data = np.append(left_hand_data, right_hand_data, axis=0)
#         data.extend(combined_data)

#         # Add labels for both hands
#         labels.extend([gesture for _ in combined_data])
    
#     data = np.concatenate(data, axis=0)
#     labels = np.concatenate(labels, axis=0)
#     return data, labels