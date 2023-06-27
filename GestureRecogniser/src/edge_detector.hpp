#ifndef EDGE_DETECTOR_HPP
#define EDGE_DETECTOR_HPP

#include <stdint.h>

/**
 * @brief A class implementing edge detection for gesture signals. Determines start and end point of a gesture given a threshold.
 */
class EdgeDetector
{
public:
    EdgeDetector() {}
    EdgeDetector(uint16_t detWL, uint16_t detEWL, uint16_t t) : m_detectionWindowLength(detWL), m_detectionEndWindowLength(detEWL), m_threshold(t) {}

    bool detectEdgeStart(uint16_t* signal);
    // bool detectEdgeEnd(uint16_t *signal);

    int getThreshold() { return m_threshold; }
    void setThreshold(uint16_t t) { this->m_threshold = t; }

private:
    uint16_t m_detectionWindowLength;
    uint16_t m_detectionEndWindowLength;
    uint16_t m_threshold;
};

#endif // EDGE_DETECTOR_HPP