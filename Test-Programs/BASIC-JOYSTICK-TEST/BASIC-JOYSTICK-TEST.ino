// ESP32 Test Basic-Joystick-Test

// Output
// View in serial monitor at speed 115200
// Moving joystick one way so decrease value to 0
// Moving other way will increase value to 4095

#define JOYINOUTPIN   34      // ADC1_6, D34 - Wire to X, with 10K resitor to GND

void updatejoystick(void)
{
  int joyval;
  joyval = analogRead(JOYINOUTPIN);             // range is 0 - 4095, midpoint is 2047
  Serial.println(joyval);
}

void initjoystick(void)
{
  Serial.println("joystick: initialise joystick");
}

void setup(void)
{
  Serial.begin(115200);
  initjoystick();
}

void loop(void)
{
  updatejoystick();
  // small delay so things do not happen so fast
  delay(100);
}
