#ifndef MODEL_WRAPPER_HPP
#define MODEL_WRAPPER_HPP

#include <stdint.h>

#include "global_constants.hpp"

// To use the TFLite micro library, we must include the following headers:
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"			// Provides the operations used by the interpreter to run the model
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h" 	// Provides debug information
#include "tensorflow/lite/micro/micro_interpreter.h"				  	// Provides loading and running of models
#include "tensorflow/lite/schema/schema_generated.h"				  	// Contains the schema for the TFLite 'FlatBuffer' model file format

// #include "../pre-processing/preprocessor.hpp"

// namespace tflite
// {
//     template <unsigned int tOpCount>
//     class MicroMutableOpResolver;
//     class ErrorReporter;
//     class Model;
//     class MicroInterpreter;
// } // namespace tflite

// struct TfLiteTensor;

class ModelWrapper
{
    public:
        // ModelWrapper(tflite::ErrorReporter* error_reporter, const tflite::Model* model);
        ModelWrapper();
        ~ModelWrapper() {
            // delete resolver;
            // delete error_reporter;
            // delete model;
            // delete interpreter;
            // delete input;
            // delete output;
            // delete tensor_arena;
        }

        float* getInputBuffer();
        float* infer(uint16_t input[NUM_LIGHT_SENSORS][GESTURE_BUFFER_LENGTH]);

    private:
        tflite::MicroMutableOpResolver<12>* resolver;
        tflite::ErrorReporter* error_reporter;
        const tflite::Model* model;
        tflite::MicroInterpreter* interpreter;
        
        // TfLiteTensor* input;
        // TfLiteTensor* output;
        float* input;
        float* output;

        uint8_t* tensor_arena;
};  // class ModelWrapper

#endif // MODEL_WRAPPER_HPP