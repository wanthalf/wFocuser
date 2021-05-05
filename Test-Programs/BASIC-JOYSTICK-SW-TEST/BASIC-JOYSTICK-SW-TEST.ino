// ESP32 Test Joystick2 with SW

#define JOYOTHERPIN   35      // Switch pin

volatile bool joystickswstate = false;

// Keyes KY-023 PS2 style 2-Axis Joystick
void IRAM_ATTR joystick2sw_isr()
{
  // Normal state of pin is high via INPUT_PULLUP - there is a R1 pull down resistor of 10K via J16-PB0EN
  // When SW is pressed a falling edge is created which triggers ISR
  joystickswstate =  true;
}

void init_joystick(void)
{
  pinMode(JOYOTHERPIN, INPUT_PULLUP);
  // setup interrupt, falling, when switch is pressed, pin goes from high to low
  attachInterrupt(JOYOTHERPIN, joystick2sw_isr, FALLING);
}

void setup(void)
{
  Serial.begin(115200);
  joystickswstate = false;
  init_joystick();
}

void loop(void)
{
  if( joystickswstate == true )
  {
    Serial.println("PULSE");
    joystickswstate = false;
  }
  else
  {
    Serial.println("no pulse");
  }
  delay(100);
}
