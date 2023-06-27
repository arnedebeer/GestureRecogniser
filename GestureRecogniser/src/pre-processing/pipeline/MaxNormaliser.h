/**
 * @file light_intensity_regulator.h
 * @brief A class that implements data normalisation through division by maximum.
 * @author Stijn van de Water @ CSE3000 RP 2022
 *
 * Modified from: https://github.com/StijnW66/CSE3000-Gesture-Recognition/blob/e6f01a54d494be49a5eebb90f43460f3c43adf4f/src/receiver/pipeline-stages/MaxNormaliser.h
 *
 */
class MaxNormaliser
{

public:
    MaxNormaliser() {}

    /**
     * @brief Normalises a single signal by finding its maximum and dividing the signal by it.
     *      The result is expected to be in the range [0,1].
     *
     * @param signal - Input signal to normalise. Expected to be in range [0, MAXIMUM].
     * @param length - Length of input signal.
     */
    void Normalise(float *signal, int length)
    {
        // Compute max
        int index = 0;
        float max = 0;
        while (index < length)
        {
            if (signal[index] > max)
                max = signal[index];
            index++;
        }

        if (max == 0)
            return;

        index = 0;
        
        // Normalise
        while (index < length)
        {
            signal[index] /= max;
            index++;
        }
    }
};