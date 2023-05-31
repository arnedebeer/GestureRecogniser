import json
import numpy as np
import matplotlib.pyplot as plt

from model_constructor import ModelConstructor

import data_loading
import model_training

if __name__ == "__main__":
    # model = ModelConstructor().get_model(model_name='cnn', input_shape=(28, 28, 1), num_classes=10)
    # print(model.summary())

    # Load data

    all_data = data_loading.load_gestures_grouped_per_candidate(use_left_hand=True, use_right_hand=True)
    all_candidates = list(all_data.keys())
    data, labels = data_loading.get_data_and_labels_from_candidates(all_candidates, all_data)

    plt.plot(data[0])