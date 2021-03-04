#include <Arduino.h>

#include "generalDefinitions.h"
#include "chipModels.h"
#include "myBoardTypes.h"
#include "myBoards.h"

// this is DRV8825 constructor
DriverBoard::DriverBoard(byte brdtype, String brdname, byte smode, byte mspd) : boardtype(brdtype), boardname(brdname)
{
  pinMode(DRV8825ENABLE, OUTPUT);
  pinMode(DRV8825DIR, OUTPUT);
  pinMode(DRV8825STEP, OUTPUT);
  digitalWrite(DRV8825ENABLE, 1);
  setstepmode( smode );
  setmotorspeed( mspd);
}

// this is constructor for ULN2003, L298N, L9110S and L293DMINI driver boards
DriverBoard::DriverBoard(byte brdtype, String brdname, byte smode, byte mspd, byte pin1, byte pin2, byte pin3, byte pin4) : boardtype(brdtype), boardname(brdname)
{
  // Move the init of inputPins here before init of myStepper to prevent stepper motor jerk
  this->inputPins[0] = pin1;
  this->inputPins[1] = pin2;
  this->inputPins[2] = pin3;
  this->inputPins[3] = pin4;
  for (int inputCount = 0; inputCount < 4; inputCount++) {
    pinMode(this->inputPins[inputCount], OUTPUT);
  }
  switch ( boardtype )
  {
    case PRO2EULN2003:
    case PRO2ESP32ULN2003:
#if (DRVBRD == PRO2EULN2003 || DRVBRD == PRO2EL298N || DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2EL9110S \
 || DRVBRD == PRO2EESP32ULN2003 || DRVBRD == PRO2EESP32L298N || DRVBRD == PRO2ESP32L293DMINI \
 || DRVBRD == PRO2ESP32L9110S )
      mystepper = new HalfStepper(STEPSPERREVOLUTION, pin1, pin2, pin3, pin4);
#endif
      this->stepdelay = ULNFAST;
      break;
    case PRO2EL298N:
    case PRO2ESP32L298N:
#if (DRVBRD == PRO2EULN2003 || DRVBRD == PRO2EL298N || DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2EL9110S \
 || DRVBRD == PRO2EESP32ULN2003 || DRVBRD == PRO2EESP32L298N || DRVBRD == PRO2ESP32L293DMINI \
 || DRVBRD == PRO2ESP32L9110S )
      mystepper = new HalfStepper(STEPSPERREVOLUTION, pin1, pin2, pin3, pin4);
#endif
      this->stepdelay = L298NFAST;
      break;
    case PRO2EL293DMINI:
    case PRO2ESP32L293DMINI:
#if (DRVBRD == PRO2EULN2003 || DRVBRD == PRO2EL298N || DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2EL9110S \
 || DRVBRD == PRO2EESP32ULN2003 || DRVBRD == PRO2EESP32L298N || DRVBRD == PRO2ESP32L293DMINI \
 || DRVBRD == PRO2ESP32L9110S )
      mystepper = new HalfStepper(STEPSPERREVOLUTION, pin1, pin2, pin3, pin4);
#endif
      this->stepdelay = L293DMINIFAST;
      break;
    case PRO2EL9110S:
    case PRO2ESP32L9110S:
#if (DRVBRD == PRO2EULN2003 || DRVBRD == PRO2EL298N || DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2EL9110S \
 || DRVBRD == PRO2EESP32ULN2003 || DRVBRD == PRO2EESP32L298N || DRVBRD == PRO2ESP32L293DMINI \
 || DRVBRD == PRO2ESP32L9110S )
      mystepper = new HalfStepper(STEPSPERREVOLUTION, pin1, pin2, pin3, pin4);
#endif
      this->stepdelay = L9110SFAST;
      break;
    default:
      // do nothing
      break;
  }
  Step = 0;
  setstepmode( smode );
  setmotorspeed( mspd);
}

byte DriverBoard::getstepmode(void)
{
  return this->stepmode;
}

void DriverBoard::setstepmode(byte smode)
{
  switch (this->boardtype)
  {
    case PRO2EDRV8825:
      // for DRV8825 stepmode is set in hardware jumpers
      // cannot set by software
      smode = DRV8825TEPMODE;       // defined at beginning of myBoards.h
      break;
    case PRO2ESP32DRV8825:
      smode = (smode < STEP1 ) ? STEP1 : smode;
      smode = (smode > STEP32 ) ? STEP32 : smode;
      break;
    case PRO2EULN2003:
    case PRO2ESP32ULN2003:
    case PRO2EL298N:
    case PRO2ESP32L298N:
    case PRO2EL293DMINI:
    case PRO2ESP32L293DMINI:
    case PRO2EL9110S:
    case PRO2ESP32L9110S:
      switch ( smode )
      {
        case STEP1:
          this->stepmode = STEP1;
#if (DRVBRD == PRO2EULN2003 || DRVBRD == PRO2EL298N || DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2EL9110S \
 || DRVBRD == PRO2EESP32ULN2003 || DRVBRD == PRO2EESP32L298N || DRVBRD == PRO2ESP32L293DMINI \
 || DRVBRD == PRO2ESP32L9110S )
          // put this inside DRVBRD test else compiler throws a fit on next line
          mystepper->SetSteppingMode(SteppingMode::FULL);
#endif
          break;
        case STEP2:
          this->stepmode = STEP2;
#if (DRVBRD == PRO2EULN2003 || DRVBRD == PRO2EL298N || DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2EL9110S \
 || DRVBRD == PRO2EESP32ULN2003 || DRVBRD == PRO2EESP32L298N || DRVBRD == PRO2ESP32L293DMINI \
 || DRVBRD == PRO2ESP32L9110S )
          // put this inside DRVBRD test else compiler throws a fit on next line
          mystepper->SetSteppingMode(SteppingMode::HALF);
#endif
          break;
        default:
          smode = STEP1;
#if (DRVBRD == PRO2EULN2003 || DRVBRD == PRO2EL298N || DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2EL9110S \
 || DRVBRD == PRO2EESP32ULN2003 || DRVBRD == PRO2EESP32L298N || DRVBRD == PRO2ESP32L293DMINI \
 || DRVBRD == PRO2ESP32L9110S )
          // put this inside DRVBRD test else compiler throws a fit on next line
          mystepper->SetSteppingMode(SteppingMode::FULL);
#endif
          this->stepmode = smode;
          break;
      }
      break;
    default:
      smode = STEP1;
      this->stepmode = smode;
      break;
  }
}

String DriverBoard::getname(void)
{
  return this->boardname;
}

void DriverBoard::enablemotor(void)
{
  switch (this->boardtype)
  {
    case PRO2EDRV8825:
    case PRO2ESP32DRV8825:
      digitalWrite(DRV8825ENABLE, 0);
      break;
    default:
      // do nothing;
      break;
  }
  delay(1);                     // boards require 1ms before stepping can occur
}

void DriverBoard::releasemotor(void)
{
  switch (this->boardtype)
  {
    case PRO2EDRV8825:
    case PRO2ESP32DRV8825:
      digitalWrite(DRV8825ENABLE, 1);
      break;
    case PRO2EULN2003:
    case PRO2ESP32ULN2003:
    case PRO2EL298N:
    case PRO2ESP32L298N:
    case PRO2EL293DMINI:
    case PRO2ESP32L293DMINI:
    case PRO2EL9110S:
    case PRO2ESP32L9110S:
      digitalWrite(this->inputPins[0], 0 );
      digitalWrite(this->inputPins[1], 0 );
      digitalWrite(this->inputPins[2], 0 );
      digitalWrite(this->inputPins[3], 0 );
      break;
    default:
      // do nothing;
      break;
  }
}

void DriverBoard::movemotor(byte dir)
{
  // handling of inout leds when moving done in main code
  switch (this->boardtype)
  {
    case PRO2EDRV8825:
    case PRO2ESP32DRV8825:
      digitalWrite(DRV8825DIR, dir);        // set Direction of travel
      digitalWrite(DRV8825ENABLE, 0);       // Enable Motor Driver
      digitalWrite(DRV8825STEP, 1);         // Step pin on
      delayMicroseconds(MOTORPULSETIME);
      digitalWrite(DRV8825STEP, 0);
      switch ( this->stepmode )
      {
        case STEP1:
          delayMicroseconds(this->stepdelay);
          break;
        case STEP2:
          delayMicroseconds(this->stepdelay / 2);
          break;
        case STEP4:
          delayMicroseconds(this->stepdelay / 4);
          break;
        case STEP8:
        case STEP16:
        case STEP32:
          delayMicroseconds(this->stepdelay / 8);
          break;
        default:
          delayMicroseconds(this->stepdelay);
          break;
      }
      break;
    case PRO2EULN2003:
    case PRO2ESP32ULN2003:
    case PRO2EL298N:
    case PRO2ESP32L298N:
    case PRO2EL293DMINI:
    case PRO2ESP32L293DMINI:
    case PRO2EL9110S:
    case PRO2ESP32L9110S:
#if (DRVBRD == PRO2EULN2003 || DRVBRD == PRO2EL298N || DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2EL9110S \
 || DRVBRD == PRO2EESP32ULN2003 || DRVBRD == PRO2EESP32L298N || DRVBRD == PRO2ESP32L293DMINI \
 || DRVBRD == PRO2ESP32L9110S )
      // put this inside DRVBRD test else compiler throws a fit on next line
      (dir == 0 ) ? mystepper->step(1) : mystepper->step(-1);
#endif
      delayMicroseconds(this->stepdelay);
      break;
  }
}

void DriverBoard::setstepdelay(int sdelay)
{
  this->stepdelay = sdelay;
}

byte DriverBoard::getmotorspeed(void)
{
  byte retspd = FAST;

  switch (this->boardtype)
  {
    case PRO2EDRV8825:
    case PRO2ESP32DRV8825:
      switch ( this->stepdelay)
      {
        case DRVFAST : retspd = FAST;
          break;
        case DRVMED : retspd = MED;
          break;
        case DRVSLOW : retspd = SLOW;
      }
      break;
    case PRO2EULN2003:
    case PRO2ESP32ULN2003:
      switch ( this->stepdelay)
      {
        case ULNFAST : retspd = FAST;
          break;
        case ULNMED : retspd = MED;
          break;
        case ULNSLOW : retspd = SLOW;
      }
      break;
    case PRO2EL298N:
    case PRO2ESP32L298N:
      switch ( this->stepdelay)
      {
        case L298NFAST : retspd = FAST;
          break;
        case L298NMED : retspd = MED;
          break;
        case L298NSLOW : retspd = SLOW;
      }
      break;
    case PRO2EL293DMINI:
    case PRO2ESP32L293DMINI:
      switch ( this->stepdelay)
      {
        case L293DMINIFAST : retspd = FAST;
          break;
        case L293DMINIMED : retspd = MED;
          break;
        case L293DMINISLOW : retspd = SLOW;
      }
      break;
    case PRO2EL9110S:
    case PRO2ESP32L9110S:
      switch ( this->stepdelay)
      {
        case L9110SFAST : retspd = FAST;
          break;
        case L9110SMED : retspd = MED;
          break;
        case L9110SSLOW : retspd = SLOW;
      }
      break;
    default:
      switch ( this->stepdelay)
      {
        case ULNFAST : retspd = FAST;
          break;
        case ULNMED : retspd = MED;
          break;
        case ULNSLOW : retspd = SLOW;
      }
      break;
  }
  return retspd;
}

void DriverBoard::setmotorspeed(byte spd )
{
  switch ( spd )
  {
    case SLOW:
      switch (this->boardtype)
      {
        case PRO2EDRV8825:
        case PRO2ESP32DRV8825:
          this->stepdelay = DRVSLOW;
          break;
        case PRO2EULN2003:
        case PRO2ESP32ULN2003:
          this->stepdelay = ULNSLOW;
          break;
        case PRO2EL298N:
        case PRO2ESP32L298N:
          this->stepdelay = L298NSLOW;
          break;
        case PRO2EL293DMINI:
        case PRO2ESP32L293DMINI:
          this->stepdelay = L293DMINISLOW;
          break;
        case PRO2EL9110S:
        case PRO2ESP32L9110S:
          this->stepdelay = L9110SSLOW;
          break;
        default:
          this->stepdelay = ULNSLOW;
          break;
      }
      break;
    case MED:
      switch (this->boardtype)
      {
        case PRO2EDRV8825:
        case PRO2ESP32DRV8825:
          this->stepdelay = DRVMED;
          break;
        case PRO2EULN2003:
        case PRO2ESP32ULN2003:
          this->stepdelay = ULNMED;
          break;
        case PRO2EL298N:
        case PRO2ESP32L298N:
          this->stepdelay = L298NMED;
          break;
        case PRO2EL293DMINI:
        case PRO2ESP32L293DMINI:
          this->stepdelay = L293DMINIMED;
          break;
        case PRO2EL9110S:
        case PRO2ESP32L9110S:
          this->stepdelay = L9110SMED;
          break;
        default:
          this->stepdelay = ULNMED;
          break;
      }
      break;
    case FAST:
      switch (this->boardtype)
      {
        case PRO2EDRV8825:
        case PRO2ESP32DRV8825:
          this->stepdelay = DRVFAST;
          break;
        case PRO2EULN2003:
        case PRO2ESP32ULN2003:
          this->stepdelay = ULNFAST;
          break;
        case PRO2EL298N:
        case PRO2ESP32L298N:
          this->stepdelay = L298NFAST;
          break;
        case PRO2EL293DMINI:
        case PRO2ESP32L293DMINI:
          this->stepdelay = L293DMINIFAST;
          break;
        case PRO2EL9110S:
        case PRO2ESP32L9110S:
          this->stepdelay = L9110SFAST;
          break;
        default:
          this->stepdelay = ULNFAST;
          break;
      }
      break;
  }
}
