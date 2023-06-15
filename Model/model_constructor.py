from enum import Enum
import tensorflow as tf
import keras.layers as layers

import data_processing

def experimental(input_shape: tuple, num_classes: int) -> tf.keras.Model:
    """
    Experimental model for testing purposes.
    """

    # [
    #     tf.keras.layers.Conv1D(32, kernel_size=3, strides=1, padding='same', activation='relu'),
    #     tf.keras.layers.MaxPooling1D(pool_size=2, strides=1, padding='same'),

    #     tf.keras.layers.Conv1D(64, kernel_size=3, strides=1, padding='same', activation='relu'),
    #     tf.keras.layers.MaxPooling1D(pool_size=2, strides=2, padding='valid'),

    #     tf.keras.layers.Flatten(),

    #     tf.keras.layers.Dense(128, activation='relu'),
    #     tf.keras.layers.Dropout(0.5),

    #     # tf.keras.layers.Dense(128, activation='relu'),
    #     # tf.keras.layers.Dropout(0.5),

    #     tf.keras.layers.Dense(num_classes, activation='softmax', name="predictions")
    # ]

    return [
        tf.keras.layers.Conv2D(32, kernel_size=(3,2), strides=(1,1), padding='same', activation='relu'),
        tf.keras.layers.MaxPooling2D(pool_size=(2,2), strides=(1,1), padding='same'),

        tf.keras.layers.Conv2D(64, kernel_size=(3,3), strides=(1,1), padding='same', activation='relu'),
        tf.keras.layers.MaxPooling2D(pool_size=(2,2), strides=(2,2), padding='valid'),

        tf.keras.layers.Flatten(),

        tf.keras.layers.Dense(128, activation='relu'),
        tf.keras.layers.Dropout(0.5),

        tf.keras.layers.Dense(128, activation='relu'),
        tf.keras.layers.Dropout(0.5),

        tf.keras.layers.Dense(num_classes, activation='softmax', name="predictions")
    ]

def alexnet(input_shape: tuple, num_classes: int) -> tf.keras.Model:
    """
    AlexNet model.
    """
    return [
        # tf.keras.Input(shape=input_shape, name="sensor_image"), # 25, 4, 3
        tf.keras.layers.Conv2D(25*4*3, kernel_size=(3, 1), strides=(1, 1), padding='valid', activation="relu"), # 23, 4, 48 
        tf.keras.layers.MaxPooling2D(pool_size=(2, 2), strides=(1, 1), padding='valid'), # 22, 3, 48
        tf.keras.layers.Conv2D(128, kernel_size=(3, 1), strides=(1, 1), padding='valid', activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(2, 1), strides=(2, 2), padding='valid'),
        tf.keras.layers.Conv2D(192, kernel_size=(2, 1), strides=(1, 1), padding='valid', activation="relu"),
        tf.keras.layers.Conv2D(192, kernel_size=(2, 1), strides=(1, 1), padding='valid', activation="relu"),
        tf.keras.layers.Conv2D(128, kernel_size=(2, 1), strides=(1, 1), padding='valid', activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(2, 1), strides=(1, 1), padding='valid'),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dropout(0.5),
        tf.keras.layers.Dense(128, activation="relu"),
        tf.keras.layers.Dropout(0.5),
        tf.keras.layers.Dense(128, activation="relu"),
        tf.keras.layers.Dense(num_classes, activation="softmax", name="predictions"),
    ]

def beernet(input_shape: tuple, num_classes: int) -> tf.keras.Model:
    """
    BeerNet Model.

    """
    return [
        # tf.keras.Input(shape=input_shape, name="sensor_image"), # 25, 4, 3

        # After input we want multiple 2D conv layers with a max pooling layer after each one
        tf.keras.layers.Conv2D(filters=32, kernel_size=(3, 1), strides=(1, 1), padding='same', activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(2, 1), strides=(1, 1), padding='valid'),

        tf.keras.layers.Conv2D(filters=64, kernel_size=(2, 2), strides=(1, 1), padding='valid', activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(2, 2), strides=(2, 1), padding='same'),

        tf.keras.layers.Conv2D(filters=128, kernel_size=(2, 1), strides=(1, 1), padding='valid', activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(2, 2), strides=(2, 1), padding='valid'),

        # After the conv layers we want to flatten our data
        # After flattening we want a dense layer
        # After each dense layer we want a dropout layer
        tf.keras.layers.Flatten(),

        tf.keras.layers.Dense(units=128, activation='relu'),        
        tf.keras.layers.Dropout(rate=0.5),
        
        tf.keras.layers.Dense(units=128, activation='relu'),
        tf.keras.layers.Dropout(rate=0.5),

        # Finally, we want a dense layer with softmax activation for classification
        tf.keras.layers.Dense(units=num_classes, activation='softmax', name="predictions")
    ]

def beernet_lite(input_shape: tuple, num_classes: int) -> list:
    """
    BeerNet Lite Model.

    """

    return [
        # tf.keras.Input(shape=input_shape, name="sensor_image"), # 25, 4, 3

        # After input we want multiple 2D conv layers with a max pooling layer after each one
        tf.keras.layers.Conv2D(filters=16, kernel_size=(3, 1), strides=(1, 1), padding='same', activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(2, 1), strides=(1, 1), padding='valid'),

        tf.keras.layers.Conv2D(filters=32, kernel_size=(2, 2), strides=(1, 1), padding='valid', activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(2, 2), strides=(2, 1), padding='same'),

        tf.keras.layers.Conv2D(filters=64, kernel_size=(2, 1), strides=(1, 1), padding='valid', activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(2, 2), strides=(2, 1), padding='valid'),

        # After the conv layers we want to flatten our data
        # After flattening we want a dense layer
        # After each dense layer we want a dropout layer
        tf.keras.layers.Flatten(),

        tf.keras.layers.Dense(units=128, activation='relu'),        
        tf.keras.layers.Dropout(rate=0.5),
        
        tf.keras.layers.Dense(units=128, activation='relu'),
        tf.keras.layers.Dropout(rate=0.5),

        # Finally, we want a dense layer with softmax activation for classification
        tf.keras.layers.Dense(units=num_classes, activation='softmax', name="predictions")
    ]

def beernet_experimental(input_shape: tuple, num_classes: int) -> tf.keras.Model:
    """
    BeerNet Model.

    """
    return [
        # tf.keras.Input(shape=input_shape, name="sensor_image"), # 25, 4, 3

        # After input we want multiple 2D conv layers with a max pooling layer after each one
        tf.keras.layers.Conv2D(filters=32, kernel_size=(3, 1), strides=(1, 1), padding='same', activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(2, 1), strides=(1, 1), padding='same'),

        tf.keras.layers.Conv2D(filters=32, kernel_size=(2, 2), strides=(1, 1), padding='valid', activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(2, 1), strides=(1, 1), padding='same'),

        tf.keras.layers.Conv2D(filters=64, kernel_size=(2, 1), strides=(1, 1), padding='valid', activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(2, 2), strides=(2, 1), padding='valid'),

        # tf.keras.layers.Conv2D(filters=64, kernel_size=(5, 1), strides=(1, 1), padding='valid', activation="relu"),
        # tf.keras.layers.MaxPooling2D(pool_size=(2, 2), strides=(1, 1), padding='valid'),

        # After the conv layers we want to flatten our data
        # After flattening we want a dense layer
        # After each dense layer we want a dropout layer
        tf.keras.layers.Flatten(),

        # tf.keras.layers.Dense(units=64, activation='relu'),        
        # tf.keras.layers.Dropout(rate=0.3),
        
        tf.keras.layers.Dense(units=128, activation='relu'),
        tf.keras.layers.Dropout(rate=0.5),

        # tf.keras.layers.Dense(units=256, activation='relu'),
        # tf.keras.layers.Dropout(rate=0.7),

        # Finally, we want a dense layer with softmax activation for classification
        tf.keras.layers.Dense(units=num_classes, activation='softmax', name="predictions")
    ]

def fcn(input_shape: tuple, num_classes: int) -> tf.keras.Model:
    """
    Fully Convolutional Network.

    """
    return [
        tf.keras.layers.BatchNormalization(),
        tf.keras.layers.Conv1D(filters=128, kernel_size=8, padding='same', activation='relu'),
        tf.keras.layers.BatchNormalization(),
        tf.keras.layers.Conv1D(filters=256, kernel_size=5, padding='same', activation='relu'),
        tf.keras.layers.BatchNormalization(),
        tf.keras.layers.Conv1D(filters=128, kernel_size=3, padding='same', activation='relu'),

        tf.keras.layers.GlobalAveragePooling2D(),

        # tf.keras.layers.Flatten(),

        # tf.keras.layers.Dense(units=32, activation='relu'),
        # tf.keras.layers.Dropout(rate=0.5),

        tf.keras.layers.Dense(units=num_classes, activation='softmax', name="predictions")
    ] 

### Models from last year ###

def slam_cnn(input_shape: tuple, num_classes: int) -> tf.keras.Model:
    return [
        # tf.keras.Input(shape=input_shape, name="sensor_image"),
        tf.keras.layers.Conv2D(32, kernel_size=(2, 2), strides=(1, 1), activation="relu"),
        tf.keras.layers.Conv2D(32, kernel_size=(2, 2), activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(3, 1)),
        tf.keras.layers.Conv2D(16, kernel_size=(5, 1), activation="relu"),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dropout(0.5),
        tf.keras.layers.Dense(num_classes, activation="softmax", name="predictions"),
    ]

def slam_cnn_padding(input_shape: tuple, num_classes: int) -> tf.keras.Model:
    return [
        # tf.keras.Input(shape=input_shape, name="sensor_image"),
        tf.keras.layers.ZeroPadding2D(padding=(0, 2)),
        tf.keras.layers.Conv2D(32, kernel_size=(3, 3), strides=(1, 1), activation="relu"),
        tf.keras.layers.Conv2D(16, kernel_size=(2, 2), strides=(1, 1), activation="relu"),
        tf.keras.layers.Conv2D(16, kernel_size=(2, 2), activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(3, 1)),
        tf.keras.layers.Conv2D(16, kernel_size=(5, 1), activation="relu"),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dropout(0.5),
        tf.keras.layers.Dense(num_classes, activation="softmax", name="predictions"),
    ]

def slam_cnn_padding_lite(input_shape: tuple, num_classes: int) -> tf.keras.Model:
    return [
        # tf.keras.Input(shape=input_shape, name="sensor_image"),
        tf.keras.layers.ZeroPadding2D(padding=(0, 2)),
        tf.keras.layers.Conv2D(16, kernel_size=(3, 3), strides=(1, 1), activation="relu"),
        tf.keras.layers.Conv2D(16, kernel_size=(2, 2), strides=(1, 1), activation="relu"),
        tf.keras.layers.Conv2D(16, kernel_size=(2, 2), activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(3, 1)),
        tf.keras.layers.Conv2D(8, kernel_size=(5, 1), activation="relu"),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dropout(0.5),
        tf.keras.layers.Dense(num_classes, activation="softmax", name="predictions"),
    ]

def slam_cnn_padding_pyramid(input_shape: tuple, num_classes: int) -> tf.keras.Model:
    return [
        # tf.keras.Input(shape=input_shape, name="sensor_image"),
        tf.keras.layers.ZeroPadding2D(padding=(0, 2)),
        tf.keras.layers.Conv2D(8, kernel_size=(3, 3), strides=(1, 1), activation="relu"),
        tf.keras.layers.Conv2D(16, kernel_size=(2, 2), strides=(1, 1), activation="relu"),
        tf.keras.layers.Conv2D(16, kernel_size=(2, 2), activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(3, 1)),
        tf.keras.layers.Conv2D(32, kernel_size=(5, 1), activation="relu"),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dropout(0.5),
        tf.keras.layers.Dense(num_classes, activation="softmax", name="predictions"),
    ]

def slam_cnn_padding_pyramid_lite(input_shape: tuple, num_classes: int) -> tf.keras.Model:
    return [
        # tf.keras.Input(shape=input_shape, name="sensor_image"),
        tf.keras.layers.ZeroPadding2D(padding=(0, 2)),
        tf.keras.layers.Conv2D(8, kernel_size=(3, 3), strides=(1, 1), activation="relu"),
        tf.keras.layers.Conv2D(16, kernel_size=(2, 2), strides=(1, 1), activation="relu"),
        tf.keras.layers.Conv2D(16, kernel_size=(2, 2), activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(3, 1)),
        tf.keras.layers.Conv2D(16, kernel_size=(5, 1), activation="relu"),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dropout(0.5),
        tf.keras.layers.Dense(num_classes, activation="softmax", name="predictions"),
    ]

def narrow_slam_cnn_padding_pyramid(input_shape: tuple, num_classes: int) -> list:
    return [
        # tf.keras.Input(shape=input_shape, name="sensor_image"),
        tf.keras.layers.ZeroPadding2D(padding=(0, 1)),
        tf.keras.layers.Conv2D(8, kernel_size=(2, 2), strides=(1, 1), activation="relu"),
        tf.keras.layers.Conv2D(16, kernel_size=(2, 2), strides=(1, 1), activation="relu"),
        tf.keras.layers.Conv2D(32, kernel_size=(2, 2), activation="relu"),
        tf.keras.layers.MaxPooling2D(pool_size=(3, 1)),
        tf.keras.layers.Conv2D(32, kernel_size=(5, 1), activation="relu"),
        tf.keras.layers.Flatten(),
        tf.keras.layers.Dropout(0.5),
        tf.keras.layers.Dense(num_classes, activation="softmax", name="predictions"),
    ]

# Instead of a dictionary with a string key with the possibility of a typo, we can use an enum
class ModelName(Enum):
    EXPERIMENTAL = experimental,
    SLAM_CNN = slam_cnn,
    SLAM_CNN_PADDING = slam_cnn_padding,
    SLAM_CNN_PADDING_LITE = slam_cnn_padding_lite,
    SLAM_CNN_PADDING_PYRAMID = slam_cnn_padding_pyramid,
    SLAM_CNN_PADDING_PYRAMID_LITE = slam_cnn_padding_pyramid_lite,
    NARROW_SLAM_CNN_PADDING_PYRAMID = narrow_slam_cnn_padding_pyramid,
    ALEXNET = alexnet,
    BEERNET = beernet,
    BEERNET_LITE = beernet_lite,
    BEERNET_EXPERIMENTAL = beernet_experimental,
    FCN = fcn,

class ModelConstructor:

    def get_model(model: ModelName, input_shape: tuple, num_classes: int, include_preprocessing: bool = True) -> tf.keras.Model:
        """
        Model constructor
        :param model_name: name of the model
        :param input_shape: input shape of the model
        :param num_classes: number of classes
        :return: model
        """

        new_model = tf.keras.models.Sequential([
            tf.keras.Input(shape=input_shape, name="sensor_image")
        ])

        if include_preprocessing:
            for layer in data_processing.preprocess_layers(input_shape):
                new_model.add(layer)
            # new_model.add(data_processing.preprocess_layers(input_shape))

        layers = model.value[0](input_shape, num_classes)

        # For all layers, add to new_model
        for layer in layers:
            new_model.add(layer)

        # new_model.add(model.value[0](input_shape, num_classes))

        # return model.value[0](input_shape, num_classes)

        return new_model