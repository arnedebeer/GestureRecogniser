#include <Arduino.h>

#include <SimpleTimer.h>

#include "global_constants.hpp"

#include "model/ModelWrapper.hpp"
#include "model/gestures.hpp"

#include "light_sensors/light_intensity_regulator.hpp"
#include "gesture_detector.hpp"

#include "util/led_control.hpp"

ModelWrapper* modelWrapper;
LightIntensityRegulator* lightIntensityRegulator;
GestureDetector* gestureDetector;

// Timers for managing sample rate and recalibrating the sensitivity of the light sensors periodically.
SimpleTimer timer;
int sampleTimerID;
// int recalibrateTimerID;

// GestureDetector::GestureDetectedCallback gestureDetectedCallback;
void gestureDetectedCallback(uint16_t photodiodeData[NUM_LIGHT_SENSORS][GESTURE_BUFFER_LENGTH]);

void setupPhotodiodes()
{
	for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
	{
		pinMode(PHOTO_DIODE_PINS[i], INPUT);
	}
}

void setupGestureDetector()
{
	gestureDetector = new GestureDetector();
	gestureDetector->setGestureDetectedCallback(gestureDetectedCallback);

	gestureDetector->setResetCallback([]() { timer.restartTimer(sampleTimerID); });

	// Setup timer to call detect gesture every READ_PERIOD milliseconds
	sampleTimerID = timer.setInterval(READ_PERIOD, []() { gestureDetector->detectGesture(); });
}

void recalibrate()
{
	// Disable timer to prevent the gesture detector from running while the sensors are being recalibrated
	timer.disable(sampleTimerID);

	// Turn on the red LED to indicate that the recalibration has started
	setLedColour(RED);

	// Recalibrate the light sensors. This will change the LED to indicate the result of the recalibration.
	lightIntensityRegulator->calibrateSensors();

	// Recalibrate the gesture detector
	gestureDetector->recalibrateThresholds();

	// Re-enable the timer
	timer.enable(sampleTimerID);
}

void setupLightIntensityRegulator()
{
	lightIntensityRegulator = new LightIntensityRegulator();
	// recalibrateTimerID = timer.setInterval(RECALIBRATE_PERIOD, []() { 
	// 	timer.disable(sampleTimerID);
	// 	lightIntensityRegulator->calibrateSensors();
	// 	gestureDetector->recalibrateThresholds();
	// 	timer.enable(sampleTimerID);
	// });
}

void setup()
{
	Serial.begin(115200);
	delay(3000);

	Serial.print("Setup started...");

	setupLeds();

	// Turn on the red LED to indicate that the setup has started
	setLedColour(RED);

	setupPhotodiodes();

	// Light intensity regulator should be setup before the gesture detector
	setupLightIntensityRegulator();

	// Add delay so result (LED colour) of recalibration is visible
	delay(1000);

	setupGestureDetector();

	// Setup model wrapper which will load the model and handle all machine learning related stuff
	modelWrapper = new ModelWrapper();

	// Turn on the blue LED to indicate that the setup has finished 
	// and the device is ready to start collecting data
	setLedColour(BLUE);

	Serial.println("Done.");
}

void loop()
{
	timer.run();
}

void gestureDetectedCallback(uint16_t photodiodeData[NUM_LIGHT_SENSORS][GESTURE_BUFFER_LENGTH])
{
	// When data collection is done set it to white to indicate that inference is running.
	setLedColour(WHITE);

	#ifdef DEBUG_PRINTS
	Serial.println("Data for gesture collected. Passing data to model and starting inference.");
	#endif

	float* result = modelWrapper->infer(photodiodeData);

	// Print the result array.
	Serial.print("Result array: ");
	for (size_t i = 0; i < NUM_FEATURES; i++)
	{
		Serial.print(result[i]);
		Serial.print(" ");
	}

	Serial.println();

	// Get the index of the highest value in the result array.
	int maxIndex = 0;
	for (size_t i = 0; i < NUM_FEATURES; i++)
	{
		if (result[i] > result[maxIndex])
		{
			maxIndex = i;
		}
	}

	// Print the gesture name and confidence
	Serial.print("Predicted gesture: ");
	Serial.print(GESTURE_NAMES[maxIndex]);
	Serial.print(", with confidence: ");
	Serial.println(result[maxIndex]);

	// Turn on the red LED to indicate that it's done with inference and not yet ready to collect data again.
	setLedColour(RED);

	// Add a delay to slow down the serial prints and avoid detecting the same gesture multiple times.
	// Can be removed if desired.
	delay(500);

	// Turn on the blue LED to indicate that it is ready to start collecting data again.
	setLedColour(BLUE);
}