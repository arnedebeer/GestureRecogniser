#ifndef GLOBAL_CONSTANTS_HPP
#define GLOBAL_CONSTANTS_HPP

// Define DEBUG_PRINTS in order to view more information in the serial monitor.
// #define DEBUG_PRINTS

// Number of OPT101 photodiode sensors used.
#define NUM_LIGHT_SENSORS 3

// Number of datapoints used as input to the model.
// This is the number of samples taken from each light sensor.
// And should be the same as the number of inputs used for training the model.
#define NUM_DATAPOINTS 100

// The shape of the input data to the model.
#define DIM1 20
#define DIM2 5
#define DIM3 3

// Number of output nodes, which is the number of classes the model can predict.
#define NUM_FEATURES 10

// The length of data buffer storing the photodiode readings.
#define GESTURE_BUFFER_LENGTH 100

// Sampling period in milliseconds. 10ms -> 100Hz sampling rate. Change to 50 for 20Hz sampling rate.
#define READ_PERIOD 10

// Time between recalibration in milliseconds. Sets up a new LightIntensityRegulator every X ms.
#define RECALIBRATE_PERIOD 5000

#endif // GLOBAL_CONSTANTS_HPP