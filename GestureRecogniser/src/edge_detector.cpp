/**
 * @file gesture_detector.hpp
 * @author CSE3000 RP 2022
 *
 * Taken from: https://github.com/StijnW66/CSE3000-Gesture-Recognition/blob/e6f01a54d494be49a5eebb90f43460f3c43adf4f/src/receiver/GREdgeDetector.h
 *
 */

#include <stdint.h>

#include <QuickMedianLib.h>

#include "global_constants.hpp"

#include "util/led_control.hpp"

#define DEBUG_RECEIVER

// Edge detection parameters
#define DETECTION_BUFFER_LENGTH 10
#define DETECTION_WINDOW_LENGTH 5
#define DETECTION_END_WINDOW_LENGTH 100 //50
#define INITIAL_DETECTION_THRESHOLD 600

// Minimum duration of a gesture, otherwise it is seen as noise and ignored
#define GESTURE_MIN_TIME_MS 100

#define THRESHOLD_ADJ_BUFFER_LENGTH 100

#define DETECTION_THRESHOLD_COEFF 0.85f
#define CUTT_OFF_THRESHOLD_COEFF 0.92f

const uint8_t PHOTO_DIODE_PINS[NUM_LIGHT_SENSORS] = {A0, A1, A2};

/**
 * @brief A class implementing edge detection for gesture signals. Determines start and end point
 *          provided a threshold.
 *
 */
class GestureEdgeDetector
{

private:
    int detectionWindowLength;
    int detectionEndWindowLength;
    int threshold;
    int cutOffThreshold;

public:
    GestureEdgeDetector() {}
    GestureEdgeDetector(int detWL, int detEWL, int t) : detectionWindowLength(detWL), detectionEndWindowLength(detEWL), threshold(t) {}

    bool DetectStart(uint16_t* signal)
    {
        int count = detectionWindowLength;

        while (count > 0)
        {
            if (*signal >= threshold)
                return false;
            signal--;
            count--;
        }

        return true;
    }

    bool DetectEnd(uint16_t *signal)
    {
        int count = detectionEndWindowLength;

        while (count > 0)
        {
            if (*signal < threshold)
                return false;
            signal--;
            count--;
        }

        return true;
    }

    void setThreshold(int t)
    {
        threshold = t;
    }

    void setCutOffThreshold(int t)
    {
        cutOffThreshold = t;
    }

    int getThreshold()
    {
        return threshold;
    }

    int getCutOffThreshold()
    {
        return cutOffThreshold;
    }
};

/**
 * @brief A class combining multiple GestureEdgeDetectors for multiple light sensors.
 *
 */
class GestureEdgeDetectors
{
private:
    GestureEdgeDetector *detectors;

    // The callback will be called when a gesture is detected and all the data is collected
    void (*callback)(uint16_t photodiodeData[NUM_LIGHT_SENSORS][GESTURE_BUFFER_LENGTH]) = nullptr;

    // The reset callback will be called after every detected gesture
    // This should reset the timer that specifies the sampling time
    void (*resetCallback)() = nullptr;

    // Buffers for dynamic threshold adjustment
    uint16_t thresholdAdjustmentBuffer[NUM_LIGHT_SENSORS][THRESHOLD_ADJ_BUFFER_LENGTH];
    // Pointer to the current index of the thresholdAdjustmentBuffer array for each light sensor
    uint16_t *taBuffer[NUM_LIGHT_SENSORS];

    int count = 0;
    bool detectionWindowFull = false;

    // Holds the data from the light sensors
    uint16_t photodiodeData[NUM_LIGHT_SENSORS][GESTURE_BUFFER_LENGTH];

    // Pointer to the current index of the photodiodeData array between 0 and GESTURE_BUFFER_LENGTH. 
    // One for each light sensor.
    uint16_t* photodiodeDataPtr[NUM_LIGHT_SENSORS];

public:
    GestureEdgeDetectors()
    {
        for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
        {
            photodiodeDataPtr[i] = photodiodeData[i];
        }

        detectors = new GestureEdgeDetector[NUM_LIGHT_SENSORS];
        for (int i = 0; i < NUM_LIGHT_SENSORS; i++)
        {
            taBuffer[i] = thresholdAdjustmentBuffer[i];
            detectors[i] = GestureEdgeDetector(DETECTION_WINDOW_LENGTH, DETECTION_END_WINDOW_LENGTH, INITIAL_DETECTION_THRESHOLD);
        }
    }

    ~GestureEdgeDetectors()
    {
        delete[] detectors;
    }

    void setGestureDetectedCallback(void (*callback)(uint16_t photodiodeData[NUM_LIGHT_SENSORS][GESTURE_BUFFER_LENGTH]))
    {
        this->callback = callback;
    }

    void setResetCallback(void (*resetCallback)())
    {
        this->resetCallback = resetCallback;
    }

    void detect_gesture()
    {
        if (detectionWindowFull == false)
        {
            // If the detection window is not filled, fill it
            for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
            {
                uint16_t data = analogRead(PHOTO_DIODE_PINS[i]);
                *photodiodeDataPtr[i] = data;
                photodiodeDataPtr[i]++;

                *taBuffer[i] = data;
                taBuffer[i]++;
            }

            count++;

            if (count == DETECTION_BUFFER_LENGTH)
            {
                // TODO: Get rid of this for loop
                for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
                {   
                    photodiodeDataPtr[i]--;
                }

                detectionWindowFull = true;
            }
        }
        else
        {
            // If the detection window is already filled, shift all data left with 1
            // and put the new sample in the last place
            for (size_t pdId = 0; pdId < NUM_LIGHT_SENSORS; pdId++)
                for (size_t i = 0; i < DETECTION_BUFFER_LENGTH - 1; i++)
                    photodiodeData[pdId][i] = photodiodeData[pdId][i + 1];

            for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
            {
                uint16_t data = analogRead(PHOTO_DIODE_PINS[i]);
                *photodiodeDataPtr[i] = data;
                
                *taBuffer[i] = data;
                taBuffer[i]++;
            }
        }

        // If there was no gesture recently, update the threshold
        // This will happen every THRESHOLD_ADJ_BUFFER_LENGTH * READ_PERIOD ms (= 100 * 10 ms = 1000 ms)
        // Unless a gesture is detected, in which case the threshold is updated after the gesture
        if (taBuffer[0] - thresholdAdjustmentBuffer[0] >= THRESHOLD_ADJ_BUFFER_LENGTH)
        {
            recalibrateThresholds(true);
        }

        // Only check for gesture if the detection window is full
        if (!detectionWindowFull) 
            return;

        bool startEdgeDetected = DetectStart(photodiodeDataPtr);

        // Try to detect a start on one of the photodiodes
        if (startEdgeDetected)
        {
            setLedColour(GREEN);
            #ifdef DEBUG_RECEIVER
            Serial.println("Gesture detected. Collecting data...");
            #endif

            // Read enough more data to avoid buffer overflow when checking end
            // of gesture if more samples are checked for end than for start
            while (count++ < DETECTION_END_WINDOW_LENGTH - DETECTION_BUFFER_LENGTH)
            {
                for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
                {
                    photodiodeDataPtr[i]++;

                    uint16_t data = analogRead(PHOTO_DIODE_PINS[i]);
                    *photodiodeDataPtr[i] = data;
                }

                delay(READ_PERIOD);
            }

            // Read new data and check for end of gesture
            while (count++ <= GESTURE_BUFFER_LENGTH)
            {
                for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
                {
                    photodiodeDataPtr[i]++;
                    uint16_t data = (uint16_t) analogRead(PHOTO_DIODE_PINS[i]);

                    *photodiodeDataPtr[i] = data;
                }

                // Allow for new data to come in
                delay(READ_PERIOD);
            }

            // Call the callback function with the gesture data
            if (callback != nullptr)
                callback(photodiodeData);

            // Reset the buffer pointers to the start of the buffers
            detectionWindowFull = false;
            for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
            {
                photodiodeDataPtr[i] = photodiodeData[i];
                taBuffer[i] = thresholdAdjustmentBuffer[i];
            }

            // recalibrateThresholds(false);

            count = 0;

            if (resetCallback != nullptr) 
            {
                resetCallback();
            }
        }
    }

    bool DetectStart(uint16_t **signals)
    {
        for (int i = 0; i < NUM_LIGHT_SENSORS; i++)
        {
            if (detectors[i].DetectStart(signals[i]))
            {
                return true;
            }
        }

        return false;
    }

    bool DetectEnd(uint16_t **signals)
    {
        for (int i = 0; i < NUM_LIGHT_SENSORS; i++)
        {
            if (detectors[i].DetectEnd(signals[i]))
            {
                return true;
            }
        }

        return false;
    }

    void recalibrateThresholds(bool resetTaBuffer = true)
    {
        #ifdef DEBUG_PRINTS
        Serial.print("Recalibrating thresholds...");
        #endif // DEBUG_PRINTS
        for (int i = 0; i < NUM_LIGHT_SENSORS; i++)
        {
            uint16_t stable = QuickMedian<uint16_t>::GetMedian(thresholdAdjustmentBuffer[i], THRESHOLD_ADJ_BUFFER_LENGTH);
            detectors[i].setThreshold(stable * DETECTION_THRESHOLD_COEFF);
            detectors[i].setCutOffThreshold(stable * CUTT_OFF_THRESHOLD_COEFF);

            if (resetTaBuffer)
                taBuffer[i] = thresholdAdjustmentBuffer[i];
        }

        #ifdef DEBUG_PRINTS
        Serial.println("Done!");
        #endif // DEBUG_PRINTS
    }

    void setThreshold(int i, int t)
    {
        detectors[i].setThreshold(t);
    }

    void setCutOffThreshold(int i, int t)
    {
        detectors[i].setCutOffThreshold(t);
    }

    int getThreshold(int i)
    {
        return detectors[i].getThreshold();
    }

    int getCutOffThreshold(int i)
    {
        return detectors[i].getCutOffThreshold();
    }
};