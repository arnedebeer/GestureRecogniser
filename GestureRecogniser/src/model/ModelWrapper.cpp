#include "ModelWrapper.hpp"

#include "model_data.hpp" // The model converted by xxd -i

// Uncomment this to _remove_ error reporting and lower memory space usage
// #define TF_LITE_STRIP_ERROR_STRINGS

// The tensor arena size needs to be determined by experimentation.
// We need to preallocate memory for the model's tensors.
const int tensor_arena_size = 8192;

ModelWrapper::ModelWrapper()
{
	// Make use of the micro error reporter because it consumes less space
	error_reporter = tflite::GetMicroErrorReporter();
	
	model = tflite::GetModel(converted_model_tflite);

	// Make sure our model is running the same version of TensorFlow as we are
	if (model->version() != TFLITE_SCHEMA_VERSION)
	{
		// From (tensorflow/lite/core/api/error_reporter.h):
		// You should not make bare calls to the error reporter, instead use the
		// TF_LITE_REPORT_ERROR macro, since this allows message strings to be
		// stripped when the binary size has to be optimized. If you are looking to
		// reduce binary size, define TF_LITE_STRIP_ERROR_STRINGS when compiling and
		// every call will be stubbed out, taking no memory.

		TF_LITE_REPORT_ERROR(error_reporter,
							 "Model provided is schema version %d not equal "
							 "to supported version %d.",
							 model->version(), TFLITE_SCHEMA_VERSION);
		return;
	}

	resolver = new tflite::MicroMutableOpResolver<12>();

	// Add all the operations to the resolver that are used in the model
	resolver->AddFullyConnected();
	resolver->AddMul();
	resolver->AddAdd();
	resolver->AddLogistic();
	resolver->AddRelu();
	resolver->AddSoftmax();
	resolver->AddReshape();
	resolver->AddPad();
	resolver->AddConv2D();
	resolver->AddMaxPool2D();

	resolver->AddQuantize();
	resolver->AddDequantize();

	// Allocate memory for the tensor_arena for the model's tensors
	tensor_arena = (uint8_t*) malloc(tensor_arena_size * sizeof(uint8_t));
	if (tensor_arena == nullptr)
	{
		TF_LITE_REPORT_ERROR(error_reporter, "Failed to allocate memory for tensor arena");
		return;
	}

	// Build an interpreter to run the model with
	static tflite::MicroInterpreter static_interpreter(model, *resolver, tensor_arena, tensor_arena_size);
	interpreter = &static_interpreter;

	// Allocate memory from the tensor_arena for the model's tensors
	TfLiteStatus allocate_status = interpreter->AllocateTensors();
	if (allocate_status != kTfLiteOk)
	{
		TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");

		return;
	}

	// Create preprocessor
	preprocessor = new Preprocessor();

	// Use this to specify the tensor_arena_size
	size_t used_bytes = interpreter->arena_used_bytes();
    TF_LITE_REPORT_ERROR(error_reporter, "Used bytes %d\n", used_bytes);

	// Get pointers to the model's input and output tensors
	input = interpreter->typed_input_tensor<float>(0);
	output = interpreter->typed_output_tensor<float>(0);
}

float* ModelWrapper::infer(uint16_t inputData[NUM_LIGHT_SENSORS][GESTURE_BUFFER_LENGTH]) 
{
	#ifdef DEBUG_PRINTS
	Serial.println("Input data before processing:");
	Serial.print("[");
	for (int i = 0; i < GESTURE_BUFFER_LENGTH; i++)
	{
		Serial.print("[");
		for (int j = 0; j < NUM_LIGHT_SENSORS; j++)
		{
			Serial.print(inputData[j][i]);
			if (j < NUM_LIGHT_SENSORS - 1) {
				Serial.print(", ");
			}
		}
		if (i < GESTURE_BUFFER_LENGTH - 1) {
			Serial.println("],");
		} else {
			Serial.println("]");
		}
	}
	Serial.println("]");
	#endif // DEBUG_PRINTS

	// Serial.print("Running pre-processing pipeline...");
	auto start = micros();
	preprocessor->runPipeline(inputData);
	auto stop = micros();

	// Calculate the time it took to run the inference
	auto duration = stop - start;

	Serial.print("Pre-processing done in: ");
	Serial.print(duration);
	Serial.print(" microseconds. ");

	float (* processedData)[100] = preprocessor->getPipelineOutput();
	
	#ifdef DEBUG_PRINTS
	Serial.println("Input data after processing:");
	Serial.print("[");
	for (int i = 0; i < GESTURE_BUFFER_LENGTH; i++)
	{
		Serial.print("[");
		for (int j = 0; j < NUM_LIGHT_SENSORS; j++)
		{
			float value = processedData[j][i];
			Serial.print(value);
			if (j < NUM_LIGHT_SENSORS - 1) {
				Serial.print(", ");
			}
		}
		if (i < GESTURE_BUFFER_LENGTH - 1) {
			Serial.println("],");
		} else {
			Serial.println("]");
		}
	}
	Serial.println("]");
	#endif // DEBUG_PRINTS

	// Before passing the data to the model we need to reshape the data to the expected shape (20, 5, 3)
	// We can do this by reinterpreting the array with different indices.
	// Because the model is trained on a transposed version of the data, we need to transpose the data before passing it to the model.
	// float (* reshapedData)[DIM1][DIM2][DIM3] = (float (*)[DIM1][DIM2][DIM3]) processedData;
	float (* reshapedData)[DIM3][DIM2][DIM1] = (float (*)[DIM3][DIM2][DIM1]) processedData;

	size_t current_index = 0;
	for (int dim2 = 0; dim2 < DIM2; dim2++)
	{
		for (int dim1 = 0; dim1 < DIM1; dim1++)
		{
			for (int dim3 = 0; dim3 < DIM3; dim3++)
			{
				input[current_index] = (*reshapedData)[dim3][dim2][dim1];
				current_index++;
			}
		}
	}

	// Run the model on this input and make sure it succeeds
	start = micros();
	TfLiteStatus invoke_status = interpreter->Invoke();
	stop = micros();

	// Calculate the time it took to run the inference
	duration = stop - start;

	Serial.print("Inference finished in: ");
	Serial.print(duration);
	Serial.println(" microseconds.");

	if (invoke_status != kTfLiteOk)
	{
		TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed");
	}

	// Return the prediction array from the model's output tensor
	return output;
}