#include "gesture_detector.hpp"

#include <QuickMedianLib.h>

#include "util/led_control.hpp"

GestureDetector::GestureDetector()
{
    edgeDetectors = new EdgeDetector[NUM_LIGHT_SENSORS];
    for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
    {
        photodiodeDataPtr[i] = photodiodeData[i];
        taBuffer[i] = thresholdAdjustmentBuffer[i];
        edgeDetectors[i] = EdgeDetector(DETECTION_WINDOW_LENGTH, DETECTION_END_WINDOW_LENGTH, INITIAL_DETECTION_THRESHOLD);
    }
}

void GestureDetector::detectGesture()
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

    bool startEdgeDetected = detectGestureStart(photodiodeDataPtr);

    // Try to detect a start on one of the photodiodes
    if (startEdgeDetected)
    {
        setLedColour(GREEN);
        
        Serial.println("--------------------");
        Serial.print("Gesture detected. Collecting data...");

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

        Serial.println("Done.");

        // Call the gestureDetectedCallback function with the gesture data
        if (gestureDetectedCallback != nullptr)
            gestureDetectedCallback(photodiodeData);

        // Reset the buffer pointers to the start of the buffers
        detectionWindowFull = false;
        for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
        {
            photodiodeDataPtr[i] = photodiodeData[i];
            taBuffer[i] = thresholdAdjustmentBuffer[i];
        }

        recalibrateThresholds(false);

        count = 0;

        if (resetCallback != nullptr) 
        {
            resetCallback();
        }
    }
}

bool GestureDetector::detectGestureStart(uint16_t **signals)
{
    for (int i = 0; i < NUM_LIGHT_SENSORS; i++)
    {
        if (edgeDetectors[i].detectEdgeStart(signals[i]))
        {
            return true;
        }
    }

    return false;
}

// Not used at this moment
// bool GestureDetector::detectGestureEnd(uint16_t **signals)
// {
//     for (int i = 0; i < NUM_LIGHT_SENSORS; i++)
//     {
//         if (edgeDetectors[i].detectEdgeEnd(signals[i]))
//         {
//             return true;
//         }
//     }

//     return false;
// }

void GestureDetector::recalibrateThresholds(bool resetTaBuffer)
{
    // #ifdef DEBUG_PRINTS
    // Serial.print("Recalibrating thresholds...");
    // #endif // DEBUG_PRINTS
    for (int i = 0; i < NUM_LIGHT_SENSORS; i++)
    {
        uint16_t stable = QuickMedian<uint16_t>::GetMedian(thresholdAdjustmentBuffer[i], THRESHOLD_ADJ_BUFFER_LENGTH);
        edgeDetectors[i].setThreshold(stable * DETECTION_THRESHOLD_COEFF);

        if (resetTaBuffer)
            taBuffer[i] = thresholdAdjustmentBuffer[i];
    }

    // #ifdef DEBUG_PRINTS
    // Serial.println("Done!");
    // #endif // DEBUG_PRINTS
}