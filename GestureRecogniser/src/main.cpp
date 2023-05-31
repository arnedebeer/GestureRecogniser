#include <Arduino.h>

#include <SimpleTimer.h>

#include "global_constants.hpp"

#define DEBUG_PRINTS

#include "model/ModelWrapper.hpp"
#include "model/gestures.hpp"

#include "light_sensors/light_intensity_regulator.hpp"
#include "edge_detector.cpp"

#include "util/led_control.hpp"

ModelWrapper* modelWrapper;
LightIntensityRegulator* lightIntensityRegulator;
GestureEdgeDetectors* gestureDetector;

// Timers for managing sample rate and recalibrating the sensitivity of the light sensors periodically.
SimpleTimer timer;
int sampleTimerID;
int recalibrateTimerID;

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
	gestureDetector = new GestureEdgeDetectors();
	gestureDetector->setGestureDetectedCallback(gestureDetectedCallback);

	gestureDetector->setResetCallback([]() { timer.restartTimer(sampleTimerID); });

	// Setup timer to call detect gesture every READ_PERIOD milliseconds
	sampleTimerID = timer.setInterval(READ_PERIOD, []() { gestureDetector->detect_gesture(); });
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
	recalibrateTimerID = timer.setInterval(RECALIBRATE_PERIOD, []() { 
		timer.disable(sampleTimerID);
		lightIntensityRegulator->calibrateSensors(); 
		timer.enable(sampleTimerID);
	});
}

void setup()
{
	Serial.begin(115200);
	delay(2000);

	Serial.print("Setup started...");

	setupLeds();

	// Turn on the red LED to indicate that the setup has started
	setLedColour(RED);

	setupPhotodiodes();
	setupGestureDetector();
	setupLightIntensityRegulator();

	// Setup model wrapper which will load the model and handle all machine learning related stuff
	modelWrapper = new ModelWrapper();

	// Turn on the blue LED to indicate that the setup has finished 
	// and the device is ready to start collecting data
	setLedColour(BLUE);

	Serial.println("Done.");
	// Serial.println("Starting sample data inference.");
	// float* result = modelWrapper->infer(DUMMY_LEFT_SWIPE);

	// Serial.println("Sample result array: ");

	// // Print result
	// for (size_t i = 0; i < NUM_FEATURES; i++)
	// {
	// 	Serial.print(result[i]);
	// 	Serial.print(", ");
	// }

	// Serial.println("\n------------------\n");
}

void loop()
{
	// static unsigned long previousTime = 0UL;
	// unsigned long currentTime = millis();
	//
	// if (currentTime - previousTime >= READ_PERIOD)
	// {
	// 	previousTime = currentTime;
	//
	// 	// sampleFromLightSensors();
	// 	receiverLoopMain();
	// }
	//
	// gestureDetector->run();
	//
	//
	// // Testing analogRead every 10 ms
	//
	// static unsigned long previousTime = 0UL;
	// unsigned long currentTime = millis();
	//
	// if (currentTime - previousTime >= 10)
	// {
	// 	previousTime = currentTime;
	//
	// 	for (size_t i = 0; i < NUM_LIGHT_SENSORS; i++)
	// 	{
	// 		uint16_t value = analogRead(PHOTO_DIODE_PINS[i]);
	// 		// Serial.print(analogRead(PHOTO_DIODE_PINS[i]));
	// 		Serial.print(value);
	// 		Serial.print(", ");
	// 	}
	// 	Serial.println();
	// }

	timer.run();
}

void gestureDetectedCallback(uint16_t photodiodeData[NUM_LIGHT_SENSORS][GESTURE_BUFFER_LENGTH])
{
	Serial.println("Data for gesture collected. Passing data to model and starting inference...");
	Serial.println();

	float* result = modelWrapper->infer(photodiodeData);

	Serial.println("Inference finished.");
	Serial.println();

	// Print the result array
	Serial.print("Result array: ");
	for (size_t i = 0; i < NUM_FEATURES; i++)
	{
		Serial.print(result[i]);
		Serial.print(" ");
	}

	Serial.println();

	// Get the index of the highest value in the result array
	int maxIndex = 0;
	for (size_t i = 0; i < NUM_FEATURES; i++)
	{
		if (result[i] > result[maxIndex])
		{
			maxIndex = i;
		}
	}

	// // Print the result
	// Serial.print("Result: ");
	// Serial.println(maxIndex);

	// Print the gesture name and confidence
	Serial.print("Predicted gesture: ");
	Serial.print(GESTURE_NAMES[maxIndex]);
	Serial.print(", with confidence: ");
	Serial.println(result[maxIndex]);

	// Add a delay to slow down the serial prints
	delay(500);
}