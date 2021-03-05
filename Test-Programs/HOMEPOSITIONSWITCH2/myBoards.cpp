// ---------------------------------------------------------------------------
// TITLE: myFP2ESP DRIVER BOARD CODE
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// COPYRIGHT
// ---------------------------------------------------------------------------
// (c) Copyright Robert Brown 2014-2020. All Rights Reserved.
// (c) Copyright Holger M, 2019-2020. All Rights Reserved.
// ---------------------------------------------------------------------------

#include <Arduino.h>
#include "generalDefinitions.h"
#include "myBoards.h"

// ____ESP8266 Boards
#if DRVBRD == WEMOSDRV8825
const char* DRVBRD_ID = "WEMOSDRV8825";
#elif  DRVBRD == PRO2EULN2003
const char* DRVBRD_ID = "PRO2EULN2003";
#elif  DRVBRD == PRO2EDRV8825
const char* DRVBRD_ID = "PRO2EDRV8825";
#elif  DRVBRD == PRO2EL293DNEMA
const char* DRVBRD_ID = "PRO2EL293DNEMA";
#elif  DRVBRD == PRO2EL293D28BYJ48
const char* DRVBRD_ID = "PRO2EL293D28BYJ48";
#elif  DRVBRD == PRO2EL298N
const char* DRVBRD_ID = "PRO2EL298N";
#elif  DRVBRD == PRO2EL293DMINI
const char* DRVBRD_ID = "PRO2EL293DMINI";
#elif  DRVBRD == PRO2EL9110S
const char* DRVBRD_ID = "PRO2EL9110S";

// ____ESP32 Boards
#elif  DRVBRD == PRO2ESP32DRV8825
const char* DRVBRD_ID = "PRO2ESP32DRV8825";

#elif  DRVBRD == PRO2ESP32ULN2003
const char* DRVBRD_ID = "PRO2ESP32ULN2003";
#elif  DRVBRD == PRO2ESP32L298N
const char* DRVBRD_ID = "PRO2ESP32L298N";
#elif  DRVBRD == PRO2ESP32L293DMINI
const char* DRVBRD_ID = "PRO2ESP32L293DMINI";
#elif  DRVBRD == PRO2ESP32L9110S
const char* DRVBRD_ID = "PRO2ESP32L9110S";
#elif  DRVBRD == PRO2ESP32R3WEMOS
const char* DRVBRD_ID = "PRO2ESP32R3WEMOS";
#else
const char* DRVBRD_ID = "UNKNOWN";
#endif

volatile bool timerSemaphore = false;
volatile uint32_t stepcount = 0;
bool stepdir;
extern DriverBoard* driverboard;

// timer Interrupt
#if defined(ESP8266)
#include "ESP8266TimerInterrupt.h"
ESP8266Timer myfp2Timer;                                      // use a unique name for the timer
#else
#include "esp32-hal-cpu.h"                                    // so we can get CPU frequency
hw_timer_t * myfp2timer = NULL;                               // use a unique name for the timer
#endif

/*
  stepcount   HPS_altert    stepdir           action
  ----------------------------------------------------
    0           x             x             stop
    >0        false           x             step
    !0        true        moving_in         stop
    !0        true        moving_out        step
*/

inline void asm2uS()  __attribute__((always_inline));

// On esp8266 with 80mHz clock a nop takes 1/80000000 second, i.e. one clock pulse, or 0.0000000125 of a second
// 2 Microseconds = 2 x 10-6 Seconds or 0.000001
// To get to 2uS for ESP8266 we will need 80 nop instructions
// On esp32 with 240mHz clock a nop takes ? 1/240000000 second or 0.000000004166 of a second
// To get to 2us for ESP32 we will need 240 nop instructions
// TODO
// I need to finish code with clock_frequency and half asm1uS calls if using a clock frequency of 120MHZ on ESP32

inline void asm1uS()                  // 1uS on ESP8266, 1/3uS on ESP32
{
  asm volatile (
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    ::
  );
}

// timer ISR  Interrupt Service Routine
#if defined(ESP8266)
ICACHE_RAM_ATTR void onTimer()
{
  static bool mjob = false;      // motor job is running or not
  if (stepcount  && !(HPS_alert && stepdir == moving_in))
  {
    driverboard->movemotor(stepdir, true);
    stepcount--;
    mjob = true;                  // mark a running job
  }
  else
  {
    if (mjob == true)
    {
      stepcount = 0;              // just in case HPS_alert was fired up
      mjob = false;               // wait, and do nothing
      timerSemaphore = true;
    }
  }
}
#else
void IRAM_ATTR onTimer()
{
  static bool mjob = false;      // motor job is running or not
  if (stepcount  && !(HPS_alert && stepdir == moving_in))
  {
    driverboard->movemotor(stepdir, true);
    stepcount--;
    mjob = true;                  // mark a running job
  }
  else
  {
    if (mjob == true)
    {
      stepcount = 0;              // just in case HPS_alert was fired up
      mjob = false;               // wait, and do nothing
      timerSemaphore = true;
    }
  }
}
#endif

DriverBoard::DriverBoard(byte brdtype, unsigned long startposition) : boardtype(brdtype)
{
  do {
#if defined(ESP8266)
    // do nothing
#else
    // esp32
    clock_frequency = ESP.getCpuFreqMHz();    // returns the CPU frequency in MHz as an unsigned 8-bit integer
    //Serial.print("Clock Freq: ");
    //Serial.println(clock_frequency);
#endif

    this->stepdelay = MSPEED;

#if (DRVBRD == WEMOSDRV8825 || DRVBRD == PRO2EDRV8825 || DRVBRD == PRO2ESP32R3WEMOS )
    pinMode(ENABLEPIN, OUTPUT);
    pinMode(DIRPIN, OUTPUT);
    pinMode(STEPPIN, OUTPUT);
    digitalWrite(ENABLEPIN, 1);
    digitalWrite(STEPPIN, 0);
    DriverBoard::setstepmode(DRV8825TEPMODE);
#elif( DRVBRD == PRO2ESP32DRV8825 )
    pinMode(ENABLEPIN, OUTPUT);
    pinMode(DIRPIN, OUTPUT);
    pinMode(STEPPIN, OUTPUT);
    digitalWrite(ENABLEPIN, 1);
    digitalWrite(STEPPIN, 0);
    pinMode(MS1, OUTPUT);
    pinMode(MS2, OUTPUT);
    pinMode(MS3, OUTPUT);
    
#elif (DRVBRD == PRO2EULN2003 || DRVBRD == PRO2ESP32ULN2003)
    // IN1, IN2, IN3, IN4
    this->inputPins[0] = IN1;
    this->inputPins[1] = IN2;
    this->inputPins[2] = IN3;
    this->inputPins[3] = IN4;
    for (int i = 0; i < 4; i++)
    {
      pinMode(this->inputPins[i], OUTPUT);
    }
    mystepper = new HalfStepper(STEPSPERREVOLUTION, IN1, IN2, IN3, IN4);  // ok
    
#elif (DRVBRD == PRO2EL298N || DRVBRD == PRO2ESP32L298N)
    // IN1, IN2, IN3, IN4
    this->inputPins[0] = IN1;
    this->inputPins[1] = IN2;
    this->inputPins[2] = IN3;
    this->inputPins[3] = IN4;
    for (int i = 0; i < 4; i++)
    {
      pinMode(this->inputPins[i], OUTPUT);
    }
    mystepper = new HalfStepper(STEPSPERREVOLUTION, IN1, IN2, IN3, IN4);  // DONE
    
#elif (DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2ESP32L293DMINI)
    // IN1, IN2, IN3, IN4
    this->inputPins[0] = IN1;
    this->inputPins[1] = IN2;
    this->inputPins[2] = IN3;
    this->inputPins[3] = IN4;
    for (int i = 0; i < 4; i++)
    {
      pinMode(this->inputPins[i], OUTPUT);
    }
    mystepper = new HalfStepper(STEPSPERREVOLUTION, IN1, IN2, IN3, IN4);  // 
    
#elif (DRVBRD == PRO2EL9110S || DRVBRD == PRO2ESP32L9110S)
    // IN1, IN2, IN3, IN4
    this->inputPins[0] = IN1;
    this->inputPins[1] = IN2;
    this->inputPins[2] = IN3;
    this->inputPins[3] = IN4;
    for (int i = 0; i < 4; i++)
    {
      pinMode(this->inputPins[i], OUTPUT);
    }
    mystepper = new HalfStepper(STEPSPERREVOLUTION, IN1, IN2, IN3, IN4);  // DONE
    
#elif (DRVBRD == PRO2EL293DNEMA )
    // IN2, IN3, IN1, IN4
    this->inputPins[0] = IN2;
    this->inputPins[1] = IN3;
    this->inputPins[2] = IN1;
    this->inputPins[3] = IN4;
    for (int i = 0; i < 4; i++)
    {
      pinMode(this->inputPins[i], OUTPUT);
    }
    mystepper = new Stepper(STEPSPERREVOLUTION, IN2, IN3, IN1, IN4);  // DONE
    setstepmode(STEP1);
    
#elif (DRVBRD == PRO2EL293D28BYJ48 )
    // IN2, IN3, IN1, IN4 mystepper.h
    this->inputPins[0] = IN1;
    this->inputPins[1] = IN3;
    this->inputPins[2] = IN2;
    this->inputPins[3] = IN4;
    for (int i = 0; i < 4; i++)
    {
      pinMode(this->inputPins[i], OUTPUT);
    }
    mystepper = new Stepper(STEPSPERREVOLUTION, IN2, IN3, IN1, IN4);  // DONE
    setstepmode(STEP1);
#endif
    // set default focuser position - ensure it is same as mySetupData when loaded
    this->focuserposition = startposition;
  } while (0);
}

// destructor
DriverBoard::~DriverBoard()
{
#if ( DRVBRD == PRO2EULN2003   || DRVBRD == PRO2ESP32ULN2003  \
   || DRVBRD == PRO2EL298N     || DRVBRD == PRO2ESP32L298N    \
   || DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2ESP32L293MINI \
   || DRVBRD == PRO2EL9110S    || DRVBRD == PRO2ESP32L9110S   \
   || DRVBRD == PRO2EL293DNEMA || DRVBRD == PRO2EL293D28BYJ48 )
  delete mystepper;
#endif
}

byte DriverBoard::getstepmode(void)
{
  return this->stepmode;
}

void DriverBoard::setstepmode(byte smode)
{
  do {
#if (DRVBRD == WEMOSDRV8825 || DRVBRD == PRO2EDRV8825 || DRVBRD == PRO2ESP32R3WEMOS )
    // for PRO2EDRV8825 stepmode is set in hardware jumpers, cannot set by software
    this->stepmode = DRV8825TEPMODE;       // defined at beginning of myBoards.h

#elif (DRVBRD == PRO2ESP32DRV8825 )
    switch (smode)
    {
      case STEP1:
        digitalWrite(MS1, 0);
        digitalWrite(MS2, 0);
        digitalWrite(MS3, 0);
        break;
      case STEP2:
        digitalWrite(MS1, 1);
        digitalWrite(MS2, 0);
        digitalWrite(MS3, 0);
        break;
      case STEP4:
        digitalWrite(MS1, 0);
        digitalWrite(MS2, 1);
        digitalWrite(MS3, 0);
        break;
      case STEP8:
        digitalWrite(MS1, 1);
        digitalWrite(MS2, 1);
        digitalWrite(MS3, 0);
        break;
      case STEP16:
        digitalWrite(MS1, 0);
        digitalWrite(MS2, 0);
        digitalWrite(MS3, 1);
        break;
      case STEP32:
        digitalWrite(MS1, 1);
        digitalWrite(MS2, 0);
        digitalWrite(MS3, 1);
        break;
      default:
        digitalWrite(MS1, 0);
        digitalWrite(MS2, 0);
        digitalWrite(MS3, 0);
        smode = STEP1;
        this->stepmode = smode;
        break;
    }
    this->stepmode = smode;

#elif (DRVBRD == PRO2EULN2003 || DRVBRD == PRO2ESP32ULN2003 \
    || DRVBRD == PRO2EL298N || DRVBRD == PRO2ESP32L298N \
    || DRVBRD == PRO2EL9110S  || DRVBRD == PRO2ESP32L9110S \
    || DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2ESP32L293DMINI )
    switch ( smode )
    {
      case STEP1:
        mystepper->SetSteppingMode(SteppingMode::FULL);
        this->stepmode = STEP1;
        break;
      case STEP2:
        mystepper->SetSteppingMode(SteppingMode::HALF);
        this->stepmode = STEP2;
        break;
      default:
        mystepper->SetSteppingMode(SteppingMode::FULL);
        this->stepmode = smode;
        break;
    }

#elif (DRVBRD == PRO2EL293DNEMA || DRVBRD == PRO2EL293D28BYJ48 )
    this->stepmode = STEP1;

#endif
  } while (0);
}

void DriverBoard::enablemotor(void)
{
#if (DRVBRD == WEMOSDRV8825    || DRVBRD == PRO2EDRV8825 || DRVBRD == PRO2ESP32DRV8825 || DRVBRD == PRO2ESP32R3WEMOS )
  digitalWrite(ENABLEPIN, 0);
  delay(1);                     // boards require 1ms before stepping can occur
#endif
}

void DriverBoard::releasemotor(void)
{
#if (DRVBRD == WEMOSDRV8825 || DRVBRD == PRO2EDRV8825 || DRVBRD == PRO2ESP32DRV8825 || DRVBRD == PRO2ESP32R3WEMOS )
  digitalWrite(ENABLEPIN, 1);

#elif (DRVBRD == PRO2EULN2003   || DRVBRD == PRO2ESP32ULN2003 \
    || DRVBRD == PRO2EL298N     || DRVBRD == PRO2ESP32L298N \
    || DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2ESP32L293DMINI \
    || DRVBRD == PRO2EL9110S    || DRVBRD == PRO2ESP32L9110S \
    || DRVBRD == PRO2EL293DNEMA || DRVBRD == PRO2EL293D28BYJ48)
  digitalWrite(this->inputPins[0], 0 );
  digitalWrite(this->inputPins[1], 0 );
  digitalWrite(this->inputPins[2], 0 );
  digitalWrite(this->inputPins[3], 0 );
#endif
}

void DriverBoard::movemotor(byte dir, bool updatefpos)
{
  //Serial.print("movemotor() : ");
  //Serial.println(dir);
  // only some boards have in out leds ESP32 only
#if (DRVBRD == PRO2ESP32ULN2003 || DRVBRD == PRO2ESP32L298N || DRVBRD == PRO2ESP32L293DMINI || DRVBRD == PRO2ESP32L9110S) || (DRVBRD == PRO2ESP32DRV8825 )
  if ( drvbrdleds )
  {
    ( dir == moving_in ) ? digitalWrite(INLEDPIN, 1) : digitalWrite(OUTLEDPIN, 1);
  }
#endif

#if (DRVBRD == WEMOSDRV8825 || DRVBRD == PRO2EDRV8825 || DRVBRD == PRO2ESP32DRV8825 || DRVBRD == PRO2ESP32R3WEMOS )
  digitalWrite(DIRPIN, dir);            // set Direction of travel
  digitalWrite(ENABLEPIN, 0);           // Enable Motor Driver
  digitalWrite(STEPPIN, 1);             // Step pin on
#if defined(ESP8266)
  asm1uS();                             // ESP8266 must be 2uS delay for DRV8825 chip
  asm1uS();
  asm1uS();
#else
  // ESP32
  if ( clock_frequency == 160 )
  {
    asm1uS();                           // ESP32 must be 2uS delay for DRV8825 chip
    asm1uS();
    asm1uS();
    asm1uS();
  }
  else
  {
    // assume clock frequency is 240mHz
    asm1uS();                             // ESP32 must be 2uS delay for DRV8825 chip
    asm1uS();
    asm1uS();
    asm1uS();
    asm1uS();
    asm1uS();
  }
#endif // #if defined(ESP8266)
  digitalWrite(STEPPIN, 0);               // Step pin off
#endif // #if (DRVBRD == WEMOSDRV8825 || DRVBRD == PRO2EDRV8825 || DRVBRD == PRO2ESP32DRV8825 || DRVBRD == PRO2ESP32R3WEMOS )

#if (DRVBRD == PRO2EULN2003     || DRVBRD == PRO2ESP32ULN2003  \
    || DRVBRD == PRO2EL298N     || DRVBRD == PRO2ESP32L298N \
    || DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2ESP32L293DMINI \
    || DRVBRD == PRO2EL9110S    || DRVBRD == PRO2ESP32L9110S \
    || DRVBRD == PRO2EL293DNEMA || DRVBRD == PRO2EL293D28BYJ48)
  ( dir == moving_in ) ? mystepper->step(-1) : mystepper->step(1);
  asm1uS();
  asm1uS();
#endif

  // turn off leds
#if (DRVBRD == PRO2ESP32ULN2003 || DRVBRD == PRO2ESP32L298N || DRVBRD == PRO2ESP32L293DMINI || DRVBRD == PRO2ESP32L9110S) || (DRVBRD == PRO2ESP32DRV8825 )
  if ( drvbrdleds )
  {
    ( dir == moving_in ) ? digitalWrite(INLEDPIN, 0) : digitalWrite(OUTLEDPIN, 0);
  }
#endif
  // update focuser position
  if ( updatefpos )
  {
    ( stepdir == moving_in ) ? this->focuserposition-- : this->focuserposition++;
  }
}

uint32_t DriverBoard::halt(void)
{
#if defined(ESP8266)
  myfp2Timer.detachInterrupt();
#else
  timerAlarmDisable(myfp2timer);      // stop alarm
  timerDetachInterrupt(myfp2timer);   // detach interrupt
  timerEnd(myfp2timer);               // end timer
#endif
  DebugPrint(F(">halt_alert "));
  delay(10);
  return stepcount;
}

void DriverBoard::initmove(bool dir, unsigned long steps, byte motorspeed, bool leds)
{
  stepcount = steps;
  stepdir = dir;
  DriverBoard::enablemotor();
  drvbrdleds = leds;
  timerSemaphore = false;

  DebugPrint(F(">initmove "));
  DebugPrint(dir);
  DebugPrint(F(":"));
  DebugPrint(steps);
  DebugPrint(F(" "));

  //Serial.print("initmove: ");
  //Serial.print(dir);
  //Serial.print(" : ");
  //Serial.print(steps);
  //Serial.print(" : ");
  //Serial.print(motorspeed);
  //Serial.print(" : ");
  //Serial.println(leds);
#if defined(ESP8266)
  unsigned long curspd = DriverBoard::getstepdelay();
  switch ( motorspeed )
  {
    case 0: // slow, 1/3rd the speed
      curspd *= 3;
      break;
    case 1: // med, 1/2 the speed
      curspd *= 2;
      break;
  }
  if (myfp2Timer.attachInterruptInterval(curspd, onTimer) == false)
  {
    DebugPrint(F("Can't set myfp2Timer correctly. Select another freq. or interval"));
  }
#else
  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual)
  myfp2timer = timerBegin(0, 80, true);
  timerAttachInterrupt(myfp2timer, &onTimer, true);  // Attach onTimer function to our timer.

  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter)
  unsigned long curspd = DriverBoard::getstepdelay();
  switch ( motorspeed )
  {
    case 0: // slow, 1/3rd the speed
      curspd *= 3;
      break;
    case 1: // med, 1/2 the speed
      curspd *= 2;
      break;
  }
  timerAlarmWrite(myfp2timer, curspd, true);   // timer for ISR
  timerAlarmEnable(myfp2timer);                // start timer alarm
#endif
}

int DriverBoard::getstepdelay(void)
{
  return this->stepdelay;
}

void DriverBoard::setstepdelay(int sdelay)
{
  this->stepdelay = sdelay;
}

unsigned long DriverBoard::getposition(void)
{
  return this->focuserposition;
}

void DriverBoard::setposition(unsigned long pos)
{
  this->focuserposition = pos;
}
