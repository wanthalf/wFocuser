// ESP32 Calibrate Joystick2
// Paul Porters, 2021

#define SERIALPORTSPEED 115200
#define JOYINOUTPIN     34                  // ADC1_6, D34 - Wire to X
#define JOYOTHERPIN     35                  // ADC1_7, D35 - Do not wire
#define ITERATIONS      250

long JMIDPOINT  = 2048;                     // 0 - 4095, 2048 is halfway or center point
long midLow     = 2048;
long midHigh    = 2048;
long JMIN       = 0;                        // minimum value for analogRead
long JTHRESHOLD = 0;
long JMAX       = 4095;                     // maximum value for analogRead

// This program writes to the serial port and you must use the Serial Monitor 
// in the Arduino IDE to view the output
// Set the speed in the monitor to 115200
// Set the sending terminator in the bottom to "Newline" [located to the left of the speed setting]
// When you see the message - "Press Enter Key when ready" - click the Send button in the monitor window

void read_enterkey()
{
  char inchar = 'A';
  while ( inchar != '\n' )
  {
    if( Serial.available() )
    {
      inchar = Serial.read();
    }
    else
    {
      // wait for keyboard input ENTER key
    }
  }
  // Serial.println("Enter key pressed");
}

void updatejoystick(void)
{
  int joyval = analogRead(JOYINOUTPIN);     // range is 0 - 4095, midpoint is 2047
  Serial.print("Raw joyval: ");
  Serial.println(joyval);
}

void initjoystick(void)
{
  Serial.println("joystick: initialise joystick");
}

void CalibrateMid() {
  Serial.println("Don't touch/move joystick");
  Serial.println("Press enter key when ready");
  read_enterkey();
  Serial.println("Starting in");
  Serial.println("3");
  delay(1000);
  Serial.println("2");
  delay(1000);
  Serial.println("1");
  delay(1000);
  Serial.println("Starting midpoint calibration");
  for (int i = 0; i < ITERATIONS; i++) {
    long newmid = analogRead(JOYINOUTPIN);
    midLow = (newmid < midLow) ? newmid : midLow;
    midHigh = (newmid > midHigh) ? newmid : midHigh;
    delay(25);
  }

  JMIDPOINT = midLow + ( (midHigh - midLow) / 2);
  JTHRESHOLD = round( (((midHigh - midLow) / 2) + ((midHigh - midLow) / 4)) );
  Serial.println("Calibration completed");
  Serial.print("Lowest midpoint = ");
  Serial.println(midLow);
  Serial.print("Highest midpoint = ");
  Serial.println(midHigh);
  Serial.print("Midpoint = ");
  Serial.println(JMIDPOINT);
  Serial.print("Threshold = ");
  Serial.println(JTHRESHOLD);
}

void CalibrateMin() {
  Serial.println("Move joystick to MIN position (= connector side)");
  Serial.println("Press enter key when ready");
  read_enterkey();
  Serial.println("Starting in");
  Serial.println("3");
  delay(1000);
  Serial.println("2");
  delay(1000);
  Serial.println("1");
  delay(1000);
  Serial.println("Starting MIN calibration");
  for (int i = 0; i < ITERATIONS; i++) {
    long newmin = analogRead(JOYINOUTPIN);
    JMIN = newmin > JMIN ? newmin : JMIN;
    delay(25);
  }
  Serial.println("Calibration completed");
  Serial.print("JMIN = ");
  Serial.println(JMIN);
}

void CalibrateMax() {
  Serial.println("Move joystick to MAX position");        // connector side
  Serial.println("Press enter key when ready");
  read_enterkey();
  Serial.println("Starting in");
  Serial.println("3");
  delay(1000);
  Serial.println("2");
  delay(1000);
  Serial.println("1");
  delay(1000);
  Serial.println("Starting MAX calibration");
  for (int i = 0; i < ITERATIONS; i++) {
    long newmax = analogRead(JOYINOUTPIN);
    JMAX = newmax < JMAX ? newmax : JMAX;
    delay(25);
  }
  Serial.println("Calibration completed");
  Serial.print("JMAX = ");
  Serial.println(JMAX);
}

void setup(void)
{
  Serial.begin(SERIALPORTSPEED);
  delay(2500);
  initjoystick();
  delay(2500);
  CalibrateMin();
  delay(2500);
  CalibrateMax();
  delay(2500);
  CalibrateMid();
}

void loop(void)
{
  //  updatejoystick();
}
