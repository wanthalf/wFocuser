// ======================================================================
// myBoards.h : myFP2ESP DRIVER BOARD DEFINITIONS
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

#include "focuserconfig.h"              // because of DRVBRD
#include "generalDefinitions.h"

#ifndef myBoards_h
#define myBoards_h

#if (DRVBRD == PRO2ESP32TMC2225) || (DRVBRD == PRO2ESP32TMC2209)
#if (DRVBRD == PRO2ESP32TMC2225)
#include <TMC2208Stepper.h>
#endif
#if (DRVBRD == PRO2ESP32TMC2209)
#include <TMCStepper.h>
#endif
#else
#include <myHalfStepperESP32.h>
#include <myStepperESP32.h>
#endif


// ======================================================================
// DRIVER BOARD CLASS : DO NOT CHANGE
// ======================================================================
extern volatile bool timerSemaphore;

class DriverBoard
{
  public:
    DriverBoard(unsigned long);                 // constructor
    ~DriverBoard(void);                         // destructor
    void initmove(bool, unsigned long);
    void movemotor(byte, bool);
    void halt(void);
#if (DRVBRD == PRO2ESP32TMC2225)
    void init_TMC2225(void);
#endif
#if (DRVBRD == PRO2ESP32TMC2209)
    void init_TMC2209(void);
#endif
    // getter
    unsigned long getposition(void);

    // setter
    void enablemotor(void);
    void setposition(unsigned long);
    void releasemotor(void);
    void setstepmode(int);

  private:
#if (DRVBRD == PRO2ESP32TMC2225) || (DRVBRD == PRO2ESP32TMC2209)
#if (DRVBRD == PRO2ESP32TMC2225)
    TMC2208Stepper* mystepper;
#endif
#if (DRVBRD == PRO2ESP32TMC2209)
    TMC2209Stepper* mystepper;
#endif
#else
    HalfStepper*  myhstepper;
    Stepper*      mystepper;
#endif    
    unsigned long focuserposition;                  // current focuser position
    int           inputPins[4];                     // input pins for driving stepper boards
    unsigned int  clock_frequency;                  // clock frequency used to generate 2us delay for ESP32 160Mhz/240Mhz
    int           boardnum;
};

#endif
