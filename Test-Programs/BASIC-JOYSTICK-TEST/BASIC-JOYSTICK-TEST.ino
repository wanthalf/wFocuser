// ESP32 Test Basic-Joystick-Test

// Output
// View in serial monitor at speed 115200
// Moving joystick one way so decrease value to 0
// Moving other way will increase value to 4095

#include "joystick.h"

void update_joystick(void)
{
  int joyval;
  joyval = analogRead(JOYINOUTPIN);             // range is 0 - 4095, midpoint is 2047
  Serial.println(joyval);
}

void init_joystick(void)
{
  pinMode(JOYINOUTPIN, INPUT);
  Serial.println("joystick: initialise joystick");
}

void setup(void)
{
  Serial.begin(115200);
  init_joystick();
}

void loop(void)
{
  update_joystick();
  // small delay so things do not happen so fast
  delay(100);
}
