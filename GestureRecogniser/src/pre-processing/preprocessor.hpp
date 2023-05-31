// #define USE_ARDUINO

#include <Arduino.h>

#include "global_constants.hpp"

#include <inttypes.h>

// Parameters for threshold computation from stable signal value,
// trimming and cutting-off the signal data in the first stages
// of the pipeline.
// #define CUTT_OFF_THRESHOLD_COEFF            0.92f // (DETECTION_THRESHOLD_COEFF + (1 - DETECTION_THRESHOLD_COEFF) / 2.0f)
// #define CUTT_OFF_THRESHOLD_COEFF_PRE_FFT    1.1f    
// #define CUTT_OFF_THRESHOLD_COEFF_POST_FFT   (CUTT_OFF_THRESHOLD_COEFF_PRE_FFT - 1.0f)  

#include "pre-processing/pipeline/MaxNormaliser.h"
// #include "pre-processing/pipeline/FFTCutOffFilter.h"
// #include "pre-processing/pipeline/SignalStretcher.h"
// #include "pre-processing/pipeline/SignalFlipper.h"

class GRPreprocessingPipeline {
public:
    auto getPipelineOutput() {
        return output;
    }

    void RunPipeline(uint16_t rawData[NUM_LIGHT_SENSORS][GESTURE_BUFFER_LENGTH]);
    
private:
    // float photodiodeDataFFTFiltered[NUM_LIGHT_SENSORS][FFT_SIGNAL_LENGTH];
    // float normPhotodiodeData[NUM_LIGHT_SENSORS][NUM_DATAPOINTS];
    float output[NUM_LIGHT_SENSORS][NUM_DATAPOINTS];

    MaxNormaliser maxNormaliser;
    // HampelOutlierDetection hampel(5);
    // SmoothFilter smf;
    // SignalStretcher sstretch;
    // SignalFlipper sFlipper;
    // FFTCutOffFilter fftFilter[NUM_LIGHT_SENSORS];
};