// ======================================================================
// myBoards.h : myFP2ESP DRIVER BOARD DEFINITIONS
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// (c) Copyright Paul P, 2021. All Rights Reserved. TMC22xx code
// ======================================================================

#include "focuserconfig.h"          // because of DRVBRD
#include "generalDefinitions.h"

#ifndef myBoards_h
#define myBoards_h

#include <myHalfStepperESP32.h>
#include <myStepperESP32.h>

#if (DRVBRD == PRO2ESP32TMC2225) || (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P)
#define SERIAL_PORT2  Serial2       // TMC2225/TMC2209 HardwareSerial port
#if (DRVBRD == PRO2ESP32TMC2225)
#include <TMC2208Stepper.h>         // tmc2225
#endif
#if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P)
#include <TMCStepper.h>             // tmc2209
#endif
#endif

// ======================================================================
// DRIVER BOARD CLASS : DO NOT CHANGE
// ======================================================================
class DriverBoard
{
  public:
    DriverBoard(unsigned long);                   // constructor
    ~DriverBoard(void);                           // destructor
    void initmove(bool, unsigned long);           // prepare to move
    void movemotor(byte, bool);                   // move the motor
    void halt(void);                              // halt the motor
    bool init_homepositionswitch(void);           // initialize home position switch
    void init_tmc2209(void);
    void init_tmc2225(void);
    bool hpsw_alert(void);                        // check for HPSW
    bool checkStall(void);                        // check for TMC2209 stall guard
    void end_move(void);                          // end a move

    // getter
    unsigned long getposition(void);
    byte getstallguard(void);
    int getboardnumber(void);

    // setter
    void enablemotor(void);
    void releasemotor(void);
    void setposition(unsigned long);
    void setstepmode(int);
    void setstallguard(byte);
    void settmc2209current(int);
    void settmc2225current(int);

  private:
    HalfStepper*  myhstepper;
    Stepper*      mystepper;
#if (DRVBRD == PRO2ESP32TMC2225 )
    // protection around mytmcstepper - it is not defined if not using tmc2209 or tmc2225
    TMC2208Stepper* mytmcstepper;
#endif // #if (DRVBRD == PRO2ESP32TMC2225)
#if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P )
    // protection around mytmcstepper - it is not defined if not using tmc2209 or tmc2225
    TMC2209Stepper* mytmcstepper;
#endif // DRVBRD == PRO2ESP32TMC2209  || DRVBRD == PRO2ESP32TMC2209P 

    unsigned long focuserposition;                  // current focuser position
    int           inputPins[4];                     // input pins for driving stepper boards
    unsigned int  clock_frequency;                  // clock frequency used to generate 2us delay for ESP32 160Mhz/240Mhz
    int boardnum;                                   // get the board number from mySetupData
};

#endif
