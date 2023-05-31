#include "preprocessor.hpp"

#include <Arduino.h>

void GRPreprocessingPipeline::RunPipeline(uint16_t rawData[NUM_LIGHT_SENSORS][GESTURE_BUFFER_LENGTH])
{
    // Converting the unsigned integer array to floats so that the pipeline can work with them
    for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
        for (size_t j = 0; j < NUM_DATAPOINTS; j++)
            output[i][j] = (float) rawData[i][j];

    // Normalize dividing by the max
    for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
        maxNormaliser.Normalise(output[i], NUM_DATAPOINTS);

    // Subtraction of mean and dividing by std
    float mean = 0;
    for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++) 
    {
        for (size_t j = 0; j < NUM_DATAPOINTS; j++)
        {
            mean += output[i][j];
        }
    }
    mean /= (NUM_LIGHT_SENSORS * NUM_DATAPOINTS);

    for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++) 
    {
        for (size_t j = 0; j < NUM_DATAPOINTS; j++)
        {
            output[i][j] -= mean;
        }
    }

    float std = 0;
    for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++) 
    {
        for (size_t j = 0; j < NUM_DATAPOINTS; j++)
        {
            std += output[i][j] * output[i][j];
        }
    }
    std /= (NUM_LIGHT_SENSORS * NUM_DATAPOINTS);
    std = sqrt(std);

    for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++) 
    {
        for (size_t j = 0; j < NUM_DATAPOINTS; j++)
        {
            output[i][j] /= std;
        }
    }

    // Serial.print("mean: ");
    // Serial.println(mean);

    // Serial.print("std: ");
    // Serial.println(std);
}