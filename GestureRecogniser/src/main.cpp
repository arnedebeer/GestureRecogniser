#include <Arduino.h>

#include <SimpleTimer.h>

#include "global_constants.hpp"

#define DEBUG_PRINTS

#include "model/ModelWrapper.hpp"
#include "model/gestures.hpp"

#include "light_sensors/light_intensity_regulator.hpp"
#include "edge_detector.cpp"

#include "sample_data/sample_data.hpp"

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

void recalibrateSensors()
{
	lightIntensityRegulator->calibrateSensors();
}

void setup()
{
	Serial.begin(115200);
	delay(5000);

	pinMode(LED_RED, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE, OUTPUT);

	digitalWrite(LED_RED, LOW);
	digitalWrite(LED_GREEN, LOW);
	digitalWrite(LED_BLUE, LOW);

	delay(2000);

	digitalWrite(LED_RED, HIGH);
	digitalWrite(LED_GREEN, LOW);
	digitalWrite(LED_BLUE, HIGH);

	setupPhotodiodes();
	setupGestureDetector();

	lightIntensityRegulator = new LightIntensityRegulator();
	// recalibrateTimerID = timer.setInterval(RECALIBRATE_PERIOD, recalibrateSensors);

	// Setup model wrapper which will load the model and handle all machine learning related stuff
	modelWrapper = new ModelWrapper();

	Serial.println("Setup complete.");

	// // For testing purposes infer sample data from sample_data.hpp
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