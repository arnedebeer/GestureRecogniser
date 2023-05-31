/**
 * @file light_intensity_regulator.h
 * @author Stijn van de Water @ CSE3000 RP 2022
 *
 * Taken from: https://github.com/StijnW66/CSE3000-Gesture-Recognition/blob/e6f01a54d494be49a5eebb90f43460f3c43adf4f/src/diode_calibration/diode_calibration.h
 *
 */

#ifndef LIGHT_INTENSITY_REGULATOR_HPP
#define LIGHT_INTENSITY_REGULATOR_HPP

#include "Arduino.h"
#include <vector>
#include <algorithm>

// Resistor struct. Pins determine which pins should be on, value represents the resistive value that is then reached.
struct Resistor
{
	std::vector<uint8_t> pins;
	float value;
};

// Set available resistor values.
const Resistor resistors[] = {{{D12}, 660000}, {{D11}, 330000}, {{D10}, 100000}, {{D9}, 22000}};

// Class that handles resistor configuration.
class LightIntensityRegulator
{
public:

	// Set diode to finetune
	const uint8_t diode = A0;

	// Parameters for diode calibration.
	const int window = 10;
	const int delay_period = 10;

	// Threshold values for initial configuration
	const int MINIMUM_THRESHOLD = 350;
	const int MAXIMUM_THRESHOLD = 750;

public:
	// Constructor with parameters for resistors (defaults to "resisistors" defined above). Number of resistors (size) is also required (default is 4).
	LightIntensityRegulator(const Resistor *resistors = resistors, int size = 4);

	void calibrateSensors();

	// Use a resistor that is higher than the current value. Voltage output of the OPT101, and thus the received value, will go down.
	// Returns true on success, false when the active resistor was already the highest possible.
	bool resistorUp();

	// Use a resistor that is lower than the current value. Voltage output of the OPT101, and thus the received value, will go down.
	// Returns true on success, false when the active resistor was already the lowest possible.
	bool resistorDown();

private:
	int resistor_index;
	int size;
	std::vector<Resistor> powerSet;

private:

	// Create a powerSet of all available resistors.
	std::vector<Resistor> createPowerSet(const Resistor *set, int size);

	// Set a list of resistors.
	void set_resistor(const std::vector<uint8_t> indices);

	// Calculates the total resistive value of a set of resistors.
	// 1/Rtot = 1/R1 + 1/R2 ...
	float calculate_total_resistance_parallel(std::vector<float> values);

	// Adds all resistances toghether to get the resistance in series.
	float calculate_total_resistance_series(std::vector<float> values);

	int get_reading();
};

#endif // LIGHT_INTENSITY_REGULATOR_HPP