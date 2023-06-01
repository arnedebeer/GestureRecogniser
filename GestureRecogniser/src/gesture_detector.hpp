#ifndef GESTURE_DETECTOR_HPP
#define GESTURE_DETECTOR_HPP

#include <Arduino.h>

#include <stdint.h>

#include "global_constants.hpp"

#include "edge_detector.hpp"

// Edge detection parameters
#define DETECTION_BUFFER_LENGTH 10
#define DETECTION_WINDOW_LENGTH 5
#define DETECTION_END_WINDOW_LENGTH 100 //50
#define INITIAL_DETECTION_THRESHOLD 100
#define DETECTION_THRESHOLD_COEFF 0.85f
#define THRESHOLD_ADJ_BUFFER_LENGTH 100

// Minimum duration of a gesture, otherwise it is seen as noise and ignored
#define GESTURE_MIN_TIME_MS 100


const uint8_t PHOTO_DIODE_PINS[NUM_LIGHT_SENSORS] = {A0, A1, A2};

/**
 * @brief A class combining multiple EdgeDetectors for multiple light sensors. Together they form a gesture detector.
 *
 */
class GestureDetector
{
public:
    using GestureDetectedCallback = void (*)(uint16_t photodiodeData[NUM_LIGHT_SENSORS][GESTURE_BUFFER_LENGTH]);
    using ResetCallback = void (*)();

public:
    GestureDetector();

    ~GestureDetector()
    {
        delete[] edgeDetectors;
    }

    void setGestureDetectedCallback(GestureDetectedCallback callback) { this->gestureDetectedCallback = callback; } 
    void setResetCallback(ResetCallback callback) { this->resetCallback = callback; }

    void detectGesture();

    bool detectGestureStart(uint16_t **signals);
    bool detectGestureEnd(uint16_t **signals);

    void recalibrateThresholds(bool resetTaBuffer = true);

    int getThreshold(int i) { return edgeDetectors[i].getThreshold(); }
    void setThreshold(int i, int t) { edgeDetectors[i].setThreshold(t); }

private:
    EdgeDetector* edgeDetectors;

    // The gestureDetectedCallback will be called when a gesture is detected and all the data is collected
    GestureDetectedCallback gestureDetectedCallback = nullptr;

    // The reset callback will be called after every detected gesture
    // This should reset the timer that specifies the sampling time
    ResetCallback resetCallback = nullptr;

    // Buffers for dynamic threshold adjustment
    uint16_t thresholdAdjustmentBuffer[NUM_LIGHT_SENSORS][THRESHOLD_ADJ_BUFFER_LENGTH];
    // Pointer to the current index of the thresholdAdjustmentBuffer array for each light sensor
    uint16_t* taBuffer[NUM_LIGHT_SENSORS];

    int count = 0;
    bool detectionWindowFull = false;

    // Holds the data from the light sensors
    uint16_t photodiodeData[NUM_LIGHT_SENSORS][GESTURE_BUFFER_LENGTH];

    // Pointer to the current index of the photodiodeData array between 0 and GESTURE_BUFFER_LENGTH. 
    // One for each light sensor.
    uint16_t* photodiodeDataPtr[NUM_LIGHT_SENSORS];
};

#endif // GESTURE_DETECTOR_HPP