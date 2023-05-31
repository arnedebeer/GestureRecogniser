#ifndef LED_CONTROL_HPP
#define LED_CONTROL_HPP

#include <Arduino.h>

void setupLeds()
{
    // Set the LED pins as outputs
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
}

enum LedColour
{
    RED,
    GREEN,
    BLUE,
};

void setLedColour(LedColour colour)
{
    switch (colour)
    {
        case RED:
            digitalWrite(LED_RED, LOW);
            digitalWrite(LED_GREEN, HIGH);
            digitalWrite(LED_BLUE, HIGH);
            break;
        case GREEN:
            digitalWrite(LED_RED, HIGH);
            digitalWrite(LED_GREEN, LOW);
            digitalWrite(LED_BLUE, HIGH);
            break;
        case BLUE:
            digitalWrite(LED_RED, HIGH);
            digitalWrite(LED_GREEN, HIGH);
            digitalWrite(LED_BLUE, LOW);
            break;
    }
}

#endif // LED_CONTROL_HPP