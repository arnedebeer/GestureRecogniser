#include "preprocessor.hpp"

#include <Arduino.h>

void Preprocessor::runPipeline(uint16_t rawData[NUM_LIGHT_SENSORS][GESTURE_BUFFER_LENGTH])
{
    // Converting the unsigned integer array to floats so that the pipeline can work with them
    for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
        for (size_t j = 0; j < NUM_DATAPOINTS; j++)
            output[i][j] = (float) rawData[i][j];
    
    normaliseData();
    removeMeanDivideStd();

    applyLowPassFilter();
}

void Preprocessor::normaliseData()
{
    // Normalize dividing by the max
    for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
        maxNormaliser.Normalise(output[i], NUM_DATAPOINTS);
}

void Preprocessor::removeMeanDivideStd()
{
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
}

// Technical info from: https://www.youtube.com/watch?v=HJ-C4Incgpw
// Coefficients calculated using https://github.com/curiores/ArduinoTutorials/blob/main/BasicFilters/Design/LowPass/ButterworthFilter.ipynb
void Preprocessor::applyLowPassFilter()
{   
    // Coefficients for a 2nd order Butterworth filter
    // Calculated using sample rate of 100 Hz and cutoff frequency of 25 Hz
    static const float a[] = {0.28094574, -0.18556054};
    static const float b[] = {0.2261537, 0.4523074, 0.2261537}; 

    // Formula for 2nd order Butterworth filter, where a and b are the coefficients and x and y are the input and output respectively
    // y[n] = a[0] * y[n-1] + a[1] * y[n-2] + b[0] * x[n] + b[1] * x[n-1] + b[2] * x[n-2]

    // For each photo diode we want to do a separate run of the filter
    for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
    {
        // The first two values of the output are the same as the input
        // output[i][0] = output[i][0];
        // output[i][1] = output[i][1];

        // The rest of the values are calculated using the formula above
        for (size_t j = 2; j < NUM_DATAPOINTS; j++)
        {
            output[i][j] = a[0] * output[i][j-1] + a[1] * output[i][j-2] + b[0] * output[i][j] + b[1] * output[i][j-1] + b[2] * output[i][j-2];
        }
    }
}