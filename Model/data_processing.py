import tensorflow as tf

import numpy as np

def preprocess_layers(input_shape: tuple = (20, 5, 3)) -> list:
    """
    Creates a list of layers for pre-processing the data. To be used in a tf.keras.Sequential model.

    """
    return [
        # Pre-processing layers
        # tf.keras.layers.Reshape(target_shape=input_shape),
        tf.keras.layers.RandomTranslation(fill_mode='nearest', height_factor=0.3, width_factor=0.25),
        tf.keras.layers.RandomContrast(factor=0.95),
        # tf.keras.layers.GaussianNoise(0.2),
    ]

def preprocess_data(signal: np.ndarray) -> np.ndarray:
    """
    Preprocesses the data by:
        - Rescaling the signal to be between 0 and 1
        - Removing the mean and dividing by the standard deviation
        - Applying a Butterworth filter

    Args:
        signal (np.ndarray): The signal to be preprocessed.

    Returns:
        np.ndarray: The preprocessed signal.
    """

    signal = rescale_signal(signal)

    signal = remove_mean_divide_std(signal)

    signal = np.apply_along_axis(butterworth_filter, 0, signal)

    # signal = np.apply_along_axis(low_pass_filter, 0, signal, 25, 100)

    return signal

# Rescale the signal to be between 0 and 1
def rescale_signal(signal: np.ndarray) -> np.ndarray:
    """
    Normalizes the signal by dividing it by the maximum value.

    Args:
        signal (np.ndarray): The signal to be normalized.

    Returns:
        np.ndarray: The normalized signal.
    """
    return signal / np.max(signal, axis=0)

# Normalize by using the Z-score
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

    # Cast to float
    signal = signal.astype(np.float32)

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