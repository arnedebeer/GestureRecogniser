import tensorflow as tf

import numpy as np

def preprocess_layers(input_shape: tuple = (20, 5, 3)) -> tf.keras.Model:
    """
    Creates a model that preprocesses the data. To be used in a tf.keras.Sequential model.

    """
    return tf.keras.models.Sequential([
        # Pre-processing layers
        # layers.BatchNormalization(),

        # tf.keras.layers.Reshape(target_shape=input_shape),

        # tf.keras.layers.Lambda(lambda x: preprocess_data(x)),

        tf.keras.layers.RandomTranslation(fill_mode='nearest', height_factor=0.4, width_factor=0.1),
        tf.keras.layers.RandomContrast(factor=0.8),
        # layers.RandomCrop(height=20, width=4),
    ])

def preprocess_data(signal: np.ndarray) -> np.ndarray:
    """
    Preprocesses the data by removing the mean and dividing by the standard deviation.

    Args:
        signal (np.ndarray): The signal to be preprocessed.

    Returns:
        np.ndarray: The preprocessed signal.
    """

    ### signal = np.apply_along_axis(low_pass_filter, 0, signal, 25, 100)
    signal = np.apply_along_axis(butterworth_filter, 0, signal)

    signal = normalize_signal(signal)
    signal = remove_mean_divide_std(signal)

    return signal

def normalize_signal(signal: np.ndarray) -> np.ndarray:
    """
    Normalizes the signal by dividing it by the maximum value.

    Args:
        signal (np.ndarray): The signal to be normalized.

    Returns:
        np.ndarray: The normalized signal.
    """
    return signal / np.max(signal, axis=0)

def remove_mean_divide_std(signal: np.ndarray) -> np.ndarray:
    """
    Removes the mean from the signal and divides by the standard deviation.

    Args:
        signal (np.ndarray): The signal to remove the mean from.

    Returns:
        np.ndarray: The signal with the mean removed.
    """
    
    mean = np.mean(signal)
    std = np.std(signal)

    # print(f"Mean: {mean}, Std: {std}")

    signal -= mean
    signal /= std

    return signal

def low_pass_filter(data: list, band_limit: int, sampling_rate: int = 100):
    """
    Applies a low pass filter to the data. Source: https://stackoverflow.com/questions/70825086/python-lowpass-filter-with-only-numpy

    Args:	
        data (list): The data to be filtered.	
        band_limit (int): The band limit of the filter.	
        sampling_rate (int, optional): The sampling rate of the data. Defaults to 100.
    """

    # Ensure the band limit is less than half the sampling rate
    assert band_limit < sampling_rate / 2, "Band limit must be less than half the sampling rate."

    cutoff_index = int(band_limit * data.size / sampling_rate)
    F = np.fft.rfft(data)
    F[cutoff_index + 1:] = 0
    return np.fft.irfft(F, n=data.size).real

def butterworth_filter(data: list):
    """
    Applies a butterworth filter to the data.

    Args:	
        data (list): The data to be filtered.	
    """

    # Coefficients for a 2nd order Butterworth filter
    # Calculated using sample rate of 100 Hz and cutoff frequency of 25 Hz
    a = [0.28094574, -0.18556054]
    b = [0.2261537, 0.4523074, 0.2261537] 

    # Formula for 2nd order Butterworth filter, where a and b are the coefficients and x and y are the input and output respectively
    # y[n] = a[0] * y[n-1] + a[1] * y[n-2] + b[0] * x[n] + b[1] * x[n-1] + b[2] * x[n-2]

    # The first two values of the output are the same as the input
    # The rest of the values are calculated using the formula above
    for i in range(2, len(data)):
        data[i] = a[0] * data[i-1] + a[1] * data[i-2] + b[0] * data[i] + b[1] * data[i-1] + b[2] * data[i-2]
        
    return data

# def high_pass_filter(data: list, band_limit: int, sampling_rate: int = 100):
#     """
#     Applies a high pass filter to the data. Source: https://stackoverflow.com/questions/70825086/python-lowpass-filter-with-only-numpy

#     Args:	
#     data (list): The data to be filtered.	
#     band_limit (int): The band limit of the filter.	
#     sampling_rate (int, optional): The sampling rate of the data. Defaults to 100.
#     """

#     # Ensure the band limit is less than half the sampling rate
#     # assert band_limit < sampling_rate / 2, "Band limit must be less than half the sampling rate."

#     cutoff_index = int(band_limit * data.size / sampling_rate)
#     F = np.fft.rfft(data)
#     F[:cutoff_index] = 0
#     F[-cutoff_index:] = 0
#     return np.fft.irfft(F, n=data.size).real