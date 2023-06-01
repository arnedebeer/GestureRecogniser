#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include <Arduino.h>

#include "global_constants.hpp"

#include <stdint.h>

#include "pre-processing/pipeline/MaxNormaliser.h"
// #include "pre-processing/pipeline/FFTCutOffFilter.h"
// #include "pre-processing/pipeline/SignalStretcher.h"
// #include "pre-processing/pipeline/SignalFlipper.h"

class Preprocessor {
public:
    void runPipeline(uint16_t rawData[NUM_LIGHT_SENSORS][GESTURE_BUFFER_LENGTH]);

    auto getPipelineOutput() {
        return output;
    }
    
private:
    float output[NUM_LIGHT_SENSORS][NUM_DATAPOINTS];

    void normaliseData();
    void removeMeanDivideStd();

    void applyLowPassFilter();

    MaxNormaliser maxNormaliser;
    
};

#endif // PREPROCESSOR_HPP