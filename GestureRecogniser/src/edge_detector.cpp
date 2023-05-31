// /**
//  * @file gesture_detector.hpp
//  * @author CSE3000 RP 2022
//  *
//  * Taken from: https://github.com/StijnW66/CSE3000-Gesture-Recognition/blob/e6f01a54d494be49a5eebb90f43460f3c43adf4f/src/receiver/GREdgeDetector.h
//  *
//  */

// #include "edge_detector.hpp"
//
//
//
// #include <QuickMedianLib.h>
//
// EdgeDetector::EdgeDetector(uint8_t detectionStartCounterThreshold, uint8_t detectionEndCounterThreshold, uint16_t activationThreshold, size_t adjBufferLength) : 
//                             detectionStartCounterThreshold(detectionStartCounterThreshold), 
//                             detectionEndCounterThreshold(detectionEndCounterThreshold), 
//                             activationThreshold(activationThreshold),
//                             adjBufferLength(adjBufferLength)
// {
//     // Allocate memory for the buffer
//     // prevReadings = new uint16_t[NUM_DATAPOINTS];
//
//     // Initialize the buffer
//     for (size_t i = 0; i < NUM_DATAPOINTS; i++)
//     {
//         prevReadings[i] = 0;
//     }
//
//     startDetected = false;
//     detectionStartCounter = 0;
//
//     endDetected = false;
//     detectionEndCounter = 0;
// }
//
// EdgeDetector::~EdgeDetector()
// {
//     delete[] prevReadings;
// }
//
// void EdgeDetector::update(uint16_t newReading)
// {
//     // Update the buffer by shifting all elements one position to the left
//     for (size_t i = 0; i < NUM_DATAPOINTS - 1; i++)
//     {
//         prevReadings[i] = prevReadings[i + 1];
//     }
//
//     // Add the new reading to the buffer
//     prevReadings[NUM_DATAPOINTS - 1] = newReading;
//
//     // Update activation threshold by looking at the median of the buffer
//     uint16_t median = QuickMedian::getMedian(&prevReadings + NUM_DATAPOINTS - adjBufferLength, adjBufferLength);
//     activationThreshold = median - 50; //TODO: Possibly subtract a constant from the median to make it less sensitive
//
//     // Update the start and end detection counters
//     if (newReading <= activationThreshold)
//     {
//         // If the new reading is below the activation threshold we increment the start counter
//         detectionStartCounter++;
//     }
//     else
//     {
//         // If the new reading is above the activation threshold we reset the start counter
//         detectionStartCounter = 0;
//     }
//
//     if (detectionStartCounter >= detectionStartCounterThreshold)
//     {
//         // If the start counter is above the detection threshold we set the start detected flag
//         startDetected = true;
//     }
//
//     if (!startDetected)
//     {
//         // If the start has not been detected yet, we reset the end counter
//         detectionEndCounter = 0;
//     }
//     else
//     {
//         if (newReading >= activationThreshold)
//         {
//             // If the new reading is above the activation threshold we increment the end counter
//             detectionEndCounter++;
//         }
//         else
//         {
//             // If the new reading is below the activation threshold we reset the end counter
//             detectionEndCounter = 0;
//         }
//
//         if (detectionEndCounter >= detectionEndCounterThreshold)
//         {
//             // If the end counter is above the detection threshold we set the end detected flag
//             endDetected = true;
//         }
//     }
// }

#include <stdint.h>

#include <QuickMedianLib.h>

#include "global_constants.hpp"

#include "util/led_control.hpp"

// #include "light_sensors/light_sensor_reader.hpp"

#define DEBUG_RECEIVER

// Edge detection parameters
#define DETECTION_BUFFER_LENGTH 10
#define DETECTION_WINDOW_LENGTH 5
#define DETECTION_END_WINDOW_LENGTH 100 //50
#define INITIAL_DETECTION_THRESHOLD 750

// Minimum duration of a gesture, otherwise it is seen as noise and ignored
#define GESTURE_MIN_TIME_MS 100

#define THRESHOLD_ADJ_BUFFER_LENGTH 100

#define DETECTION_THRESHOLD_COEFF 0.70f
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

    void (*callback)(uint16_t photodiodeData[NUM_LIGHT_SENSORS][GESTURE_BUFFER_LENGTH]) = nullptr;
    void (*resetCallback)() = nullptr;

    // Buffers for dynamic threshold adjustment
    uint16_t thresholdAdjustmentBuffer[NUM_LIGHT_SENSORS][THRESHOLD_ADJ_BUFFER_LENGTH];
    uint16_t *taBuffer[NUM_LIGHT_SENSORS];

    int count = 0;
    bool detectionWindowFull = false;

    // Holds the data from the light sensors
    uint16_t photodiodeData[NUM_LIGHT_SENSORS][GESTURE_BUFFER_LENGTH];

    // Pointer to the current index of the photodiodeData array between 0 and GESTURE_BUFFER_LENGTH
    // size_t photodiodeDataIndex[NUM_LIGHT_SENSORS] = {0, 0, 0};
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

                // lightSensorReader.read(PHOTO_DIODE_PINS[i], photodiodeDataPtr[i]++);
                // lightSensorReader.read(PHOTO_DIODE_PINS[i], taBuffer[i]++);
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
            // TODO: Switch place of the two for loops... probably a bit faster (makes cache happier, although it's not a lot of data)
            for (size_t pdId = 0; pdId < NUM_LIGHT_SENSORS; pdId++)
                for (size_t i = 0; i < DETECTION_BUFFER_LENGTH - 1; i++)
                    photodiodeData[pdId][i] = photodiodeData[pdId][i + 1];

            for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
            {
                uint16_t data = analogRead(PHOTO_DIODE_PINS[i]);
                *photodiodeDataPtr[i] = data;
                
                *taBuffer[i] = data;
                taBuffer[i]++;
                
                // lightSensorReader.read(PHOTO_DIODE_PINS[i], photodiodeDataPtr[i]);
                // lightSensorReader.read(PHOTO_DIODE_PINS[i], taBuffer[i]++);
            }
        }

        // If there was no gesture recently, update the threshold
        if (taBuffer[0] - thresholdAdjustmentBuffer[0] >= THRESHOLD_ADJ_BUFFER_LENGTH)
        // if (taBufferIndex[0] >= THRESHOLD_ADJ_BUFFER_LENGTH)
        {
            // for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
            // {
            //     uint16_t stable = QuickMedian<uint16_t>::GetMedian(thresholdAdjustmentBuffer[i], THRESHOLD_ADJ_BUFFER_LENGTH);
            //     detectors[i].setThreshold(stable * DETECTION_THRESHOLD_COEFF);
            //     detectors[i].setCutOffThreshold(stable * CUTT_OFF_THRESHOLD_COEFF);
            //     taBuffer[i] = thresholdAdjustmentBuffer[i];
            // }

            recalibrateThresholds(true);
        }

        // Only check for gesture if the detection window is full
        if (!detectionWindowFull) 
            return;

        // bool startEdgeDetected = false;
        // for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
        //     startEdgeDetected = startEdgeDetected || detectors[i].DetectStart(photodiodeDataPtr[i]);
        
        bool startEdgeDetected = DetectStart(photodiodeDataPtr);

        // Try to detect a start on one of the photodiodes
        if (startEdgeDetected)
        {
            setLedColour(GREEN);
            #ifdef DEBUG_RECEIVER
            Serial.println("Gesture detected. Collecting data...");//, looking for end of gesture...");
            #endif

            // bool endDetected = false;

            // Read enough more data to avoid buffer overflow when checking end
            // of gesture if more samples are checked for end than for start
            while (count++ < DETECTION_END_WINDOW_LENGTH - DETECTION_BUFFER_LENGTH)
            {
                for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
                {
                    photodiodeDataPtr[i]++;

                    uint16_t data = analogRead(PHOTO_DIODE_PINS[i]);
                    *photodiodeDataPtr[i] = data;

                    // lightSensorReader.read(PHOTO_DIODE_PINS[i], photodiodeDataPtr[i]);
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

                    // lightSensorReader.read(PHOTO_DIODE_PINS[i], photodiodeDataPtr[i]);
                }

                // Allow for new data to come in
                delay(READ_PERIOD);

                // bool endEdgeDetected = true;

            }

                        // for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
                        // {
                        //     if (!detectors[i].DetectEnd(photodiodeDataPtr[i]))
                        //     {
                        //         continue;
                        //     }
                        //     // endEdgeDetected = endEdgeDetected && detectors[i].DetectEnd(photodiodeDataPtr[i]);
                        // }




                // if (!endEdgeDetected)
                //     continue;

                // Determine the gesture length by looking at the difference between
                // the current position (the pointer) and index 0




                            // uint16_t gestureSignalLength = photodiodeDataPtr[0] - photodiodeData[0] + 1;

                            // // Reject gestures that took too short time
                            // if (gestureSignalLength < GESTURE_MIN_TIME_MS / READ_PERIOD + 1)
                            // {

                            //     // #ifdef DEBUG_RECEIVER
                            //     Serial.println("Gesture took too little time! Rejecting and starting over ...");
                            //     // #endif

                            //     break;
                            // }
                            // else
                            //     endDetected = true;




                // ------------------------------------------
                // Run the pipeline

                // uint16_t thresholds[NUM_LIGHT_SENSORS];
                // for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
                // {
                //     thresholds[i] = detectors[i].getCutOffThreshold();
                // }

                // pipeline.RunPipeline(photodiodeData, gestureSignalLength, thresholds);

                // float(*output)[100] = pipeline.getPipelineOutput();

                // Gesture g =  inferGesture2d(output);
                // Serial.print("Gesture: ");
                // Serial.println(g);

                if (callback != nullptr)
                    callback(photodiodeData);

                // break;
            // }

            // Reset the buffer pointers to the start of the buffers
            detectionWindowFull = false;
            for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
            {
                photodiodeDataPtr[i] = photodiodeData[i];
                taBuffer[i] = thresholdAdjustmentBuffer[i];
            }

            count = 0;

            // Gesture took too long -> Light Intensity Change -> Threshold Recalculation
            // if (!endDetected)
            // {
            //     // for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
            //     // {
            //     //     uint16_t stable = QuickMedian<uint16_t>::GetMedian(thresholdAdjustmentBuffer[i], THRESHOLD_ADJ_BUFFER_LENGTH);
            //     //     detectors[i].setThreshold(stable * DETECTION_THRESHOLD_COEFF);
            //     //     detectors[i].setCutOffThreshold(stable * CUTT_OFF_THRESHOLD_COEFF);
            //     // }

            //     recalibrateThresholds(false);
            // }

            // timer.restartTimer(timID);

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
        // if (i < 0 || i >= NUM_LIGHT_SENSORS)
        //     return;

        detectors[i].setThreshold(t);
    }

    void setCutOffThreshold(int i, int t)
    {
        // if (i < 0 || i >= NUM_LIGHT_SENSORS)
        //     return;

        detectors[i].setCutOffThreshold(t);
    }

    int getThreshold(int i)
    {
        // if (i < 0 || i >= NUM_LIGHT_SENSORS)
        //     return -1;

        return detectors[i].getThreshold();
    }

    int getCutOffThreshold(int i)
    {
        // if (i < 0 || i >= NUM_LIGHT_SENSORS)
        //     return -1;

        return detectors[i].getCutOffThreshold();
    }
};