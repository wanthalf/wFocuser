// ESP32 Calibrate Joystick1

#define JOYINOUTPIN   34      // ADC1_6, D34 - Wire to X
#define JOYOTHERPIN   35      // ADC1_7, D35 - Do not wire

void updatejoystick(void)
{
  int joyval = analogRead(JOYINOUTPIN);             // range is 0 - 4095, midpoint is 2047
  Serial.print("Raw joyval: ");
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
}
