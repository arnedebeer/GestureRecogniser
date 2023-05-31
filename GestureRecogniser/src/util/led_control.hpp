#ifndef LED_CONTROL_HPP
#define LED_CONTROL_HPP

enum LedColour
{
    RED,
    GREEN,
    BLUE,
    WHITE
};

void setupLeds();
void setLedColour(LedColour colour);

#endif // LED_CONTROL_HPP