/**
 * @file light_intensity_regulator.h
 * @author Stijn van de Water @ CSE3000 RP 2022
 *
 * Taken from: https://github.com/StijnW66/CSE3000-Gesture-Recognition/blob/e6f01a54d494be49a5eebb90f43460f3c43adf4f/src/diode_calibration/diode_calibration.h
 *
 */

#include "light_intensity_regulator.hpp"

#include "util/led_control.hpp"

// Resistor comparator function for sorting in decreasing order.
bool comparator(Resistor const &a, Resistor const &b)
{
	return a.value > b.value;
}

// Constructor with parameters for resistors (defaults to "resisistors" defined above). Number of resistors (size) is also required (default is 4).
LightIntensityRegulator::LightIntensityRegulator(const Resistor *resistors, int size)
{
	this->resistor_index = 0;

	// Create powerset and sort
	this->size = pow(2, size);
	this->powerSet = this->createPowerSet(resistors, this->size);

	std::sort(powerSet.begin(), powerSet.end(), &comparator);

	this->calibrateSensors();
}

void LightIntensityRegulator::calibrateSensors()
{
	#ifdef DEBUG_PRINTS
	Serial.println("Calibrating sensors...");
	#endif

	set_resistor(powerSet[0].pins);

	// Allow the capacitor to charge up
	delay(100);

	int reading = this->get_reading();

	// Loop trough available resistors until the value drops enough
	while (reading > MAXIMUM_THRESHOLD)
	{
		if (!this->resistorDown())
		{
			// Required resistor does not exist, set red LED
			setLedColour(RED);
			return;
		}
		else
		{
			// Get new reading
			reading = this->get_reading();
		}
	}

	// Reading is now low enough for accurate sensing.
	if (reading < MINIMUM_THRESHOLD)
	{
		if (!this->resistorUp())
		{
			// Required resistor does not exist, set red LED
			setLedColour(RED);
		}
		else
		{
			// System maybe configured correctly, set blue LED
			setLedColour(BLUE);
		}
	}
	else
	{
		// System configured correctly, set green LED
		setLedColour(GREEN);
	}

	#ifdef DEBUG_PRINTS
	Serial.println("Calibration done!");
	#endif
}

// Use a resistor that is higher than the current value. Voltage output of the OPT101, and thus the received value, will go down.
// Returns true on success, false when the active resistor was already the highest possible.
bool LightIntensityRegulator::resistorUp()
{
	int index = this->resistor_index - 1;
	if (index < 0)
	{
		// Index is not possible
		return false;
	}

	this->resistor_index--;
	set_resistor(this->powerSet[this->resistor_index].pins);
	
	return true;
}

// Use a resistor that is lower than the current value. Voltage output of the OPT101, and thus the received value, will go down.
// Returns true on success, false when the active resistor was already the lowest possible.
bool LightIntensityRegulator::resistorDown()
{
	int index = this->resistor_index + 1;

	if (index >= this->size)
	{
		// Index is not possible
		return false;
	}
	
	this->resistor_index++;
	set_resistor(this->powerSet[this->resistor_index].pins);
	
	return true;
}


// Create a powerSet of all available resistors.
std::vector<Resistor> LightIntensityRegulator::createPowerSet(const Resistor *set, int size)
{
	std::vector<Resistor> powerSet(size);

	// Fill powerset
	for (int counter = 0; counter < size; counter++)
	{
		std::vector<uint8_t> pin_numbers;
		std::vector<float> values;

		for (int j = 0; j < size; j++)
		{
			if (counter & (1 << j))
			{
				values.push_back(set[j].value);
				pin_numbers.push_back(set[j].pins.at(0)); // Since these are single resistors the pin numbers are at postion 0.
			}
		}
		powerSet[counter] = Resistor{pin_numbers, calculate_total_resistance_series(values)};
	}
	return powerSet;
}

// Set a list of resistors.
void LightIntensityRegulator::set_resistor(const std::vector<uint8_t> indices)
{
	// First unset all resistors

	for (uint8_t i = 0; i < sizeof(resistors) / sizeof(Resistor); i++)
	{
		digitalWrite(resistors[i].pins.at(0), HIGH);
	}

	// Then set all resistors
	for (unsigned int j = 0; j < indices.size(); j++)
	{
		digitalWrite(indices.at(j), LOW);
	}
}

// Calculates the total resistive value of a set of resistors.
// 1/Rtot = 1/R1 + 1/R2 ...
float LightIntensityRegulator::calculate_total_resistance_parallel(std::vector<float> values)
{
	float sum = 0;
	for (unsigned int i = 0; i < values.size(); i++)
	{
		sum += 1 / values.at(i);
	}
	return 1 / sum;
}

// Adds all resistances toghether to get the resistance in series.
float LightIntensityRegulator::calculate_total_resistance_series(std::vector<float> values)
{
	float sum = 0;
	for (unsigned int i = 0; i < values.size(); i++)
	{
		sum += values.at(i);
	}
	return sum;
}

int LightIntensityRegulator::get_reading()
{
	delay(10);

	int read_sum = 0;
	for (int i = 0; i < window; i++)
	{
		read_sum += analogRead(diode);
		delay(delay_period);
	}
	return read_sum / window;
}