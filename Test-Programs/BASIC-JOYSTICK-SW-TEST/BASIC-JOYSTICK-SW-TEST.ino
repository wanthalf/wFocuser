// ESP32 Test Joystick2 with SW

#define JOYOTHERPIN   35      // Switch pin

// Keyes KY-023 PS2 style 2-Axis Joystick
volatile bool joy2swstate = false;

void IRAM_ATTR joystick2sw_isr()
{ 
  // an interrupt means switch has been pressed
  joy2swstate = true;                             // flag joy2swstate set to 1 indicates switch is pressed
}

void init_joystick2(void)
{
  pinMode(JOYOTHERPIN, INPUT);                  // otherpin is the switch botton on Joystick2
  // setup interrupt, falling edge, pin state = HIGH and falls to GND (0) when pressed
  attachInterrupt(JOYOTHERPIN, joystick2sw_isr, FALLING);
  joy2swstate = false;                          // after interrupt is processed, the joy2swtate is reset to 0
}

void setup(void)
{
  Serial.begin(115200);
  joy2swstate = false;
  init_joystick2();
}

void loop(void)
{
  if( joy2swstate == true )
  {
    Serial.println("PULSE");
    joy2swstate = false;
  }
  else
  {
    Serial.println("no pulse");
  }
  delay(100);
}
