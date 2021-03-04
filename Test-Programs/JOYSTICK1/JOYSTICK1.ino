// ESP32 Test Joystick1

#define JOYINOUTPIN   34      // ADC1_6, D34 - Wire to X
#define JOYOTHERPIN   35      // ADC1_7, D35 - Do not wire
//#define JZEROPOINT    2047
#define JZEROPOINT    1765    // value read when joystick is centered
#define JTHRESHOLD    300     // margin of error around center position
#define JMAXVALUE     4095    // maximum value reading of joystick
#define JMINVALUE     0       // minimum value reading of joystick

#define MSFAST        1
#define MSMED         1000
#define MSSLOW        8000
unsigned long ftargetPosition = 5000;
int joyspeed;

void updatejoystick(void)
{
  int joypos;
  int joyval;
  joyval = analogRead(JOYINOUTPIN);             // range is 0 - 4095, midpoint is 2047
  Serial.print("Raw joyval: ");
  Serial.println(joyval);
  if ( joyval < (JZEROPOINT - JTHRESHOLD) )
  {
    ftargetPosition--;                          // move IN
    Serial.print("X IN joyval:");
    Serial.print(joyval);
    joyspeed = map(joyval, 0, (JZEROPOINT - JTHRESHOLD), MSFAST, MSSLOW);
    Serial.print(", Speed:");
    Serial.println(joyspeed);
  }
  else if ( joyval > (JZEROPOINT + JTHRESHOLD) )
  {
    ftargetPosition++;                          // move OUT
    if ( ftargetPosition > 10000)
    {
      ftargetPosition = 10000;
    }
    joyval = joyval - (JZEROPOINT + JTHRESHOLD);
    if ( joyval < 0 )
    {
      //joyval = JZEROPOINT - joyval;
    }
    Serial.print("X OUT joyval:");
    Serial.print(joyval);
    joyspeed = map(joyval, 0, (JMAXVALUE-(JZEROPOINT + JTHRESHOLD)), MSSLOW, MSFAST);      
    Serial.print(", Speed:");
    Serial.println(joyspeed);
  }
  Serial.print("Position:");
  Serial.print(ftargetPosition);
  Serial.print(", Speed:");
  Serial.println(joyspeed);
}

void initjoystick(void)
{
  Serial.println("joystick: initialise joystick");
  // perform any inititalisations necessary
  //pinMode(JOYINOUTPIN, INPUT_PULLUP);
  //pinMode(JOYOTHERPIN, INPUT_PULLUP);
  joyspeed = MSFAST;
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
