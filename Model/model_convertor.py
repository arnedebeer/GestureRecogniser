# This python file will contain all code related to converting models to TFLite format.
# It will have various functions for converting models with different optimisations

import tensorflow as tf
import numpy as np

def quantize_model( model, representative_data, write_to_file = False): 
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]

    def representative_data_generator():
        for i in range(min(1000, len(representative_data))):
            sample_data = representative_data[i]
            sample_data = np.expand_dims(sample_data, axis=0)
            yield [sample_data.astype(np.float32)]

    converter.representative_dataset = representative_data_generator

    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    # converter._experimental_new_quantizer = True
    converter.allow_custom_ops = True
    # converter.target_spec.supported_types = [tf.int8]

    # converter.inference_input_type = tf.float32
    # converter.inference_output_type = tf.uint8

    tflite_model = converter.convert()

    if write_to_file:
        open("converted_model.tflite", "wb").write(tflite_model)

    return tflite_model
