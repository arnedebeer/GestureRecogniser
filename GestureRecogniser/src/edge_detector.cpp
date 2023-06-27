/**
 * @file edge_detector.cpp
 * @author CSE3000 RP 2022
 *
 * Modified from: https://github.com/StijnW66/CSE3000-Gesture-Recognition/blob/e6f01a54d494be49a5eebb90f43460f3c43adf4f/src/receiver/GREdgeDetector.h
 */

#include "edge_detector.hpp"

bool EdgeDetector::detectEdgeStart(uint16_t* signal)
{
    uint16_t count = m_detectionWindowLength;

    while (count > 0)
    {
        if (*signal >= m_threshold)
            return false;
        signal--;
        count--;
    }

    return true;
}

// Not used at this moment
// bool EdgeDetector::detectEdgeEnd(uint16_t *signal)
// {
//     uint16_t count = m_detectionEndWindowLength;

//     while (count > 0)
//     {
//         if (*signal < m_threshold)
//             return false;
//         signal--;
//         count--;
//     }

//     return true;
// }