// ======================================================================
// myBoards.cpp : myFP2ESP DRIVER BOARD CODE
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

#include <Arduino.h>
#include "boarddefs.h"
#include "generalDefinitions.h"
#include "focuserconfig.h"  
#include "myBoards.h"
#include "FocuserSetupData.h"

// ======================================================================
// Defines for TMCxxxx
// ======================================================================
#define STALL_VALUE         1             // [0... 255]
#define TMC2209CURRENT      600           // 600mA for 8HS15-0604S NEMA8 stepper motor
#define TMC2225CURRENT      300           // 300mA for recommended stepper NEMA motor - you can change this 
#define TMC2225SPEED        57600
#define TMC2209SPEED        57600

#define BTRX                10            // tmc2209 interface xxxxxxxxxxxxxxxxxx
#define BTTX                11            // xxxxxxxxxxxxxxxxx

#define DRIVER_ADDRESS      0b00          // TMC2209 Driver address according to MS1 and MS2
#define R_SENSE             0.11f         // Match to your driver
                                          // SilentStepStick series use 0.11
                                          // UltiMachine Einsy and Archim2 boards use 0.2
                                          // Panucatt BSD2660 uses 0.1
                                          // Watterott TMC5160 uses 0.075
                                          
// ======================================================================
// Externs
// ======================================================================
extern SetupData   *mySetupData;
extern DriverBoard *driverboard;
extern int  DefaultBoardNumber;
extern bool HPS_alert(void);

// ======================================================================
// Data
// ======================================================================
volatile bool     timerSemaphore = false;
volatile uint32_t stepcount = 0;
bool stepdir;

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
  if (stepcount  && !(HPS_alert() && stepdir == moving_in))
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
  if (stepcount  && !(HPS_alert() && stepdir == moving_in))
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

DriverBoard::DriverBoard(unsigned long startposition)
{
  do {
#if defined(ESP8266)
    // do nothing
#else
    // esp32
    clock_frequency = ESP.getCpuFreqMHz();    // returns the CPU frequency in MHz as an unsigned 8-bit integer
    //DebugPrint("Clock Freq: ");
    //DebugPrintln(clock_frequency);
#endif
    boardnum = DefaultBoardNumber;

    // THESE NEED TO BE CREATED AT RUNTIME

    if ( boardnum == WEMOSDRV8825 || boardnum == PRO2EDRV8825 || boardnum == PRO2ESP32R3WEMOS || boardnum == WEMOSDRV8825H )
    {
      pinMode(mySetupData->get_brdenablepin(), OUTPUT);
      pinMode(mySetupData->get_brddirpin(), OUTPUT);
      pinMode(mySetupData->get_brdsteppin(), OUTPUT);
      digitalWrite(mySetupData->get_brdenablepin(), 1);
      digitalWrite(mySetupData->get_brdsteppin(), 0);
    }
    else if ( boardnum == PRO2ESP32DRV8825 || boardnum == PRO2ESP32TMC2225 || boardnum == PRO2ESP32TMC2209)
    {
      pinMode(mySetupData->get_brdenablepin(), OUTPUT);
      pinMode(mySetupData->get_brddirpin(), OUTPUT);
      pinMode(mySetupData->get_brdsteppin(), OUTPUT);
      digitalWrite(mySetupData->get_brdenablepin(), 1);
      digitalWrite(mySetupData->get_brdsteppin(), 0);
      pinMode(mySetupData->get_brdboardpins(0), OUTPUT);
      pinMode(mySetupData->get_brdboardpins(1), OUTPUT);
      pinMode(mySetupData->get_brdboardpins(2), OUTPUT);
    }
    else if ( boardnum == PRO2EULN2003 || boardnum == PRO2ESP32ULN2003)
    {
      // IN1, IN2, IN3, IN4
      this->inputPins[0] = mySetupData->get_brdboardpins(0);
      this->inputPins[1] = mySetupData->get_brdboardpins(1);
      this->inputPins[2] = mySetupData->get_brdboardpins(2);
      this->inputPins[3] = mySetupData->get_brdboardpins(3);
      for (int i = 0; i < 4; i++)
      {
        pinMode(this->inputPins[i], OUTPUT);
      }
      myhstepper = new HalfStepper(mySetupData->get_brdstepsperrev(), this->inputPins[0], this->inputPins[1], this->inputPins[2], this->inputPins[3]);  // ok
    }
    else if ( boardnum == PRO2EL298N || boardnum == PRO2ESP32L298N)
    {
      // IN1, IN2, IN3, IN4
      this->inputPins[0] = mySetupData->get_brdboardpins(0);
      this->inputPins[1] = mySetupData->get_brdboardpins(1);
      this->inputPins[2] = mySetupData->get_brdboardpins(2);
      this->inputPins[3] = mySetupData->get_brdboardpins(3);
      for (int i = 0; i < 4; i++)
      {
        pinMode(this->inputPins[i], OUTPUT);
      }
      myhstepper = new HalfStepper(mySetupData->get_brdstepsperrev(), this->inputPins[0], this->inputPins[1], this->inputPins[2], this->inputPins[3]);  // ok
    }
    else if ( boardnum == PRO2EL293DMINI || boardnum == PRO2ESP32L293DMINI)
    {
      // IN1, IN2, IN3, IN4
      this->inputPins[0] = mySetupData->get_brdboardpins(0);
      this->inputPins[1] = mySetupData->get_brdboardpins(1);
      this->inputPins[2] = mySetupData->get_brdboardpins(2);
      this->inputPins[3] = mySetupData->get_brdboardpins(3);
      for (int i = 0; i < 4; i++)
      {
        pinMode(this->inputPins[i], OUTPUT);
      }
      myhstepper = new HalfStepper(mySetupData->get_brdstepsperrev(), this->inputPins[0], this->inputPins[1], this->inputPins[2], this->inputPins[3]);  // ok
    }
    else if (boardnum == PRO2EL9110S || boardnum == PRO2ESP32L9110S)
    {
      // IN1, IN2, IN3, IN4
      this->inputPins[0] = mySetupData->get_brdboardpins(0);
      this->inputPins[1] = mySetupData->get_brdboardpins(1);
      this->inputPins[2] = mySetupData->get_brdboardpins(2);
      this->inputPins[3] = mySetupData->get_brdboardpins(3);
      for (int i = 0; i < 4; i++)
      {
        pinMode(this->inputPins[i], OUTPUT);
      }
      myhstepper = new HalfStepper(mySetupData->get_brdstepsperrev(), this->inputPins[0], this->inputPins[1], this->inputPins[2], this->inputPins[3]);  // ok
    }
    else if (boardnum == PRO2EL293DNEMA )
    {
      // IN2, IN3, IN1, IN4
      this->inputPins[0] = mySetupData->get_brdboardpins(1);
      this->inputPins[1] = mySetupData->get_brdboardpins(2);
      this->inputPins[2] = mySetupData->get_brdboardpins(0);
      this->inputPins[3] = mySetupData->get_brdboardpins(3);
      for (int i = 0; i < 4; i++)
      {
        pinMode(this->inputPins[i], OUTPUT);
      }
      mystepper = new Stepper(mySetupData->get_brdstepsperrev(), this->inputPins[0], this->inputPins[1], this->inputPins[2], this->inputPins[3]);  // DONE
      mySetupData->set_brdstepmode(STEP1);
    }
    else if ( boardnum == PRO2EL293D28BYJ48 )
    {
      // IN2, IN3, IN1, IN4 mystepper.h
      this->inputPins[0] = mySetupData->get_brdboardpins(0);
      this->inputPins[1] = mySetupData->get_brdboardpins(2);
      this->inputPins[2] = mySetupData->get_brdboardpins(1);
      this->inputPins[3] = mySetupData->get_brdboardpins(3);
      for (int i = 0; i < 4; i++)
      {
        pinMode(this->inputPins[i], OUTPUT);
      }
      mystepper = new Stepper(mySetupData->get_brdstepsperrev(), mySetupData->get_brdboardpins(1), mySetupData->get_brdboardpins(2), this->inputPins[0], this->inputPins[3]);  // DONE
      mySetupData->set_brdstepmode(STEP1);
    }
    // set default focuser position - ensure it is same as mySetupData when loaded
    this->focuserposition = startposition;
  } while (0);
}

// destructor
DriverBoard::~DriverBoard()
{
  String drvbrd = mySetupData->get_brdname();
  if ( boardnum == PRO2EULN2003      || boardnum == PRO2ESP32ULN2003  \
       || boardnum == PRO2EL298N     || boardnum == PRO2ESP32L298N    \
       || boardnum == PRO2EL293DMINI || boardnum == PRO2ESP32L293DMINI \
       || boardnum == PRO2EL9110S    || boardnum == PRO2ESP32L9110S)
  {
    delete myhstepper;
  }
  else if (boardnum == PRO2EL293DNEMA || boardnum == PRO2EL293D28BYJ48 )
  {
    delete mystepper;
  }
  else if (boardnum == PRO2ESP32TMC2225)
  {
    delete mystepper;
  }
  else if (boardnum == PRO2ESP32TMC2209)
  {
    delete mystepper;
  }
}

#if (DRVBRD == PRO2ESP32TMC2209)
void DriverBoard::init_TMC2209(void)
{
  mystepper = new TMC2209Stepper(BTRX, BTTX, R_SENSE, DRIVER_ADDRESS);     // Specify the serial interface to the tmc2225
  mystepper->beginSerial(TMC2209SPEED);
  mystepper->pdn_disable(1);                      // Use PDN/UART pin for communication
  mystepper->mstep_reg_select(true);              // Adjust stepMode from the registers

  // if you want to adjust current by software, uncomment these lines
  mystepper->I_scale_analog(0);                   // Adjust current from the registers
  mystepper->rms_current( TMC2209CURRENT );       // Set driver current
  mystepper->toff(3);                             // Use TMC2225 Calculations sheet to get these.
  mystepper->tbl(2);                              // Use TMC2225 Calculations sheet to get these.
  mystepper->hysteresis_start(7);                 // Use TMC2225 Calculations sheet to get these.
  mystepper->hysteresis_end(7);                   // Use TMC2225 Calculations sheet to get these.

  // StallGuard settings

  // Lower threshold velocity for switching on smart energy CoolStep and StallGuard to DIAG output
  mystepper->TCOOLTHRS(0xFFFFF); // 20bit max

  // CoolStep lower threshold [0... 15].
  // If SG_RESULT goes below this threshold, CoolStep increases the current to both coils.
  // 0: disable CoolStep
  mystepper->semin(0);

  // CoolStep upper threshold [0... 15].
  // If SG is sampled equal to or above this threshold enough times,
  // CoolStep decreases the current to both coils.
  mystepper->semax(0);

  // Sets the number of StallGuard2 readings above the upper threshold necessary
  // for each current decrement of the motor current.
  mystepper->sedn(0b00);

  // StallGuard4 threshold [0... 255] level for stall detection. It compensates for
  // motor specific characteristics and controls sensitivity. A higher value gives a higher
  // sensitivity. A higher value makes StallGuard4 more sensitive and requires less torque to
  // indicate a stall. The double of this value is compared to SG_RESULT.
  // The stall output becomes active if SG_RESULT falls below this value.
  mystepper->SGTHRS(STALL_VALUE);
}
#endif // #if (DRVBRD == PRO2ESP32TMC2209)

#if (DRVBRD == PRO2ESP32TMC2225)
void DriverBoard::init_TMC2225(void)
{
  mystepper= new TMC2208Stepper(BTRX, BTTX);      // Specify the serial interface to the tmc2225
  mystepper->beginSerial(TMC2225SPEED);
  mystepper->pdn_disable(1);                      // Use PDN/UART pin for communication
  mystepper->mstep_reg_select(true);

  mystepper->I_scale_analog(0);                   // Adjust current from the registers
  mystepper->rms_current(TMC2225CURRENT);         // Set driver current [recommended NEMA = 400mA, set to 300mA]
  mystepper->toff(0x2);                           // Enable driver
}
#endif // #if (DRVBRD == PRO2ESP32TMC2225)

void DriverBoard::setstepmode(int smode)
{
  String drvbrd = mySetupData->get_brdname();
  do {
    if (boardnum == WEMOSDRV8825 || boardnum == PRO2EDRV8825 || boardnum == PRO2ESP32R3WEMOS || boardnum == WEMOSDRV8825H)
    {
      // for PRO2EDRV8825 stepmode is set in hardware jumpers, cannot set by software
      mySetupData->set_brdstepmode(mySetupData->get_brdfixedstepmode());
    }
    else if (boardnum == PRO2ESP32DRV8825 )
    {
      switch (smode)
      {
        case STEP1:
          digitalWrite(mySetupData->get_brdboardpins(0), 0);
          digitalWrite(mySetupData->get_brdboardpins(1), 0);
          digitalWrite(mySetupData->get_brdboardpins(2), 0);
          break;
        case STEP2:
          digitalWrite(mySetupData->get_brdboardpins(0), 1);
          digitalWrite(mySetupData->get_brdboardpins(1), 0);
          digitalWrite(mySetupData->get_brdboardpins(2), 0);
          break;
        case STEP4:
          digitalWrite(mySetupData->get_brdboardpins(0), 0);
          digitalWrite(mySetupData->get_brdboardpins(1), 1);
          digitalWrite(mySetupData->get_brdboardpins(2), 0);
          break;
        case STEP8:
          digitalWrite(mySetupData->get_brdboardpins(0), 1);
          digitalWrite(mySetupData->get_brdboardpins(1), 1);
          digitalWrite(mySetupData->get_brdboardpins(2), 0);
          break;
        case STEP16:
          digitalWrite(mySetupData->get_brdboardpins(0), 0);
          digitalWrite(mySetupData->get_brdboardpins(1), 0);
          digitalWrite(mySetupData->get_brdboardpins(2), 1);
          break;
        case STEP32:
          digitalWrite(mySetupData->get_brdboardpins(0), 1);
          digitalWrite(mySetupData->get_brdboardpins(1), 0);
          digitalWrite(mySetupData->get_brdboardpins(2), 1);
          break;
        default:
          digitalWrite(mySetupData->get_brdboardpins(0), 0);
          digitalWrite(mySetupData->get_brdboardpins(1), 0);
          digitalWrite(mySetupData->get_brdboardpins(2), 0);
          smode = STEP1;
          mySetupData->set_brdstepmode(smode);
          break;
      }
    }
    else if (boardnum == PRO2EULN2003      || boardnum == PRO2ESP32ULN2003 \
             || boardnum == PRO2EL298N     || boardnum == PRO2ESP32L298N  \
             || boardnum == PRO2EL9110S    || boardnum == PRO2ESP32L9110S  \
             || boardnum == PRO2EL293DMINI || boardnum == PRO2ESP32L293DMINI )
    {
      switch ( smode )
      {
        case STEP1:
          myhstepper->SetSteppingMode(SteppingMode::FULL);
          break;
        case STEP2:
          myhstepper->SetSteppingMode(SteppingMode::HALF);
          break;
        default:
          myhstepper->SetSteppingMode(SteppingMode::FULL);
          mySetupData->set_brdstepmode(1);
          break;
      }
    }
    else if (boardnum == PRO2EL293DNEMA || boardnum == PRO2EL293D28BYJ48 )
    {
      mySetupData->set_brdstepmode(STEP1);
    }
  } while (0);
}

void DriverBoard::enablemotor(void)
{
  String drvbrd = mySetupData->get_brdname();
  if (boardnum == WEMOSDRV8825 || boardnum == PRO2EDRV8825 || boardnum == PRO2ESP32DRV8825 || boardnum == PRO2ESP32R3WEMOS || boardnum == WEMOSDRV8825H)
  {
    digitalWrite(mySetupData->get_brdenablepin(), 0);
    delay(1);                     // boards require 1ms before stepping can occur
  }
}

void DriverBoard::releasemotor(void)
{
  String drvbrd = mySetupData->get_brdname();
  if (boardnum == WEMOSDRV8825 || boardnum == PRO2EDRV8825 || boardnum == PRO2ESP32DRV8825 || boardnum == PRO2ESP32R3WEMOS || boardnum == WEMOSDRV8825H)
  {
    digitalWrite(mySetupData->get_brdenablepin(), 1);
  }
  else if (boardnum == PRO2EULN2003      || boardnum == PRO2ESP32ULN2003 \
           || boardnum == PRO2EL298N     || boardnum == PRO2ESP32L298N \
           || boardnum == PRO2EL293DMINI || boardnum == PRO2ESP32L293DMINI \
           || boardnum == PRO2EL9110S    || boardnum == PRO2ESP32L9110S \
           || boardnum == PRO2EL293DNEMA || boardnum == PRO2EL293D28BYJ48)
  {
    digitalWrite(this->inputPins[0], 0 );
    digitalWrite(this->inputPins[1], 0 );
    digitalWrite(this->inputPins[2], 0 );
    digitalWrite(this->inputPins[3], 0 );
  }
}

void DriverBoard::movemotor(byte dir, bool updatefpos)
{
  String drvbrd = mySetupData->get_brdname();
  //DebugPrint("movemotor() : ");
  //DebugPrintln(dir);
  // only ESP32 boards have in out leds
  if (boardnum == PRO2ESP32ULN2003 || boardnum == PRO2ESP32L298N || boardnum == PRO2ESP32L293DMINI || boardnum == PRO2ESP32L9110S || boardnum == PRO2ESP32DRV8825 )
  {
    // Basic assumption rule: If associated pin is -1 then cannot set enable
    if ( mySetupData->get_inoutledstate() == 1)
    {
      ( dir == moving_in ) ? digitalWrite(mySetupData->get_brdinledpin(), 1) : digitalWrite(mySetupData->get_brdoutledpin(), 1);
    }
  }

  else if (boardnum == WEMOSDRV8825 || boardnum == PRO2EDRV8825 || boardnum == PRO2ESP32DRV8825 || boardnum == PRO2ESP32R3WEMOS || boardnum == WEMOSDRV8825H || boardnum == PRO2ESP32TMC2225 || boardnum == PRO2ESP32TMC2209 )
  {
    if ( mySetupData->get_reversedirection() == 1 )
    {
      digitalWrite(mySetupData->get_brddirpin(), !dir);         // set Direction of travel
    }
    else
    {
      digitalWrite(mySetupData->get_brddirpin(), dir);          // set Direction of travel
    }
    digitalWrite(mySetupData->get_brdenablepin(), 0);           // Enable Motor Driver
    digitalWrite(mySetupData->get_brdsteppin(), 1);             // Step pin on
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
    digitalWrite(mySetupData->get_brdsteppin(), 0);               // Step pin off
  }

  else if (boardnum == PRO2EULN2003 || boardnum == PRO2ESP32ULN2003  \
      || boardnum == PRO2EL298N     || boardnum == PRO2ESP32L298N \
      || boardnum == PRO2EL293DMINI || boardnum == PRO2ESP32L293DMINI \
      || boardnum == PRO2EL9110S    || boardnum == PRO2ESP32L9110S)
  {
    if ( dir == moving_in )
    {
      if ( mySetupData->get_reversedirection() == 1 )
      {
        myhstepper->step(1);
      }
      else
      {
        myhstepper->step(-1);
      }
    }
    else
    {
      if ( mySetupData->get_reversedirection() == 1 )
      {
        myhstepper->step(-1);
      }
      else
      {
        myhstepper->step(1);
      }
    }
    asm1uS();
    asm1uS();
  }
  else if ( boardnum == PRO2EL293DNEMA || boardnum == PRO2EL293D28BYJ48 )
  {
    if ( dir == moving_in )
    {
      if ( mySetupData->get_reversedirection() == 1 )
      {
        mystepper->step(1);
      }
      else
      {
        mystepper->step(-1);
      }
    }
    else
    {
      if ( mySetupData->get_reversedirection() == 1 )
      {
        mystepper->step(-1);
      }
      else
      {
        mystepper->step(1);
      }
    }
    asm1uS();
    asm1uS();
  }

  // turn off leds
  if (boardnum == PRO2ESP32ULN2003 || boardnum == PRO2ESP32L298N || boardnum == PRO2ESP32L293DMINI ||boardnum == PRO2ESP32L9110S || boardnum == PRO2ESP32DRV8825 )
  {
    // Basic assumption rule: If associated pin is -1 then cannot set enable
    if ( mySetupData->get_inoutledstate() == 1)
    {
      ( dir == moving_in ) ? digitalWrite(mySetupData->get_brdinledpin(), 0) : digitalWrite(mySetupData->get_brdoutledpin(), 0);
    }
  }
  // update focuser position
  if ( updatefpos )
  {
    ( stepdir == moving_in ) ? this->focuserposition-- : this->focuserposition++;
  }
}

void DriverBoard::halt(void)
{
#if defined(ESP8266)
  myfp2Timer.detachInterrupt();
#else
  timerAlarmDisable(myfp2timer);      // stop alarm
  timerDetachInterrupt(myfp2timer);   // detach interrupt
  timerEnd(myfp2timer);               // end timer
#endif
  Board_DebugPrintln(">halt_alert ");
  delay(10);
}

void DriverBoard::initmove(bool mdir, unsigned long steps)
{
  stepcount = steps;
  stepdir = mdir;
  DriverBoard::enablemotor();
  timerSemaphore = false;

  Board_DebugPrint(">initmove ");
  Board_DebugPrint(mdir);
  Board_DebugPrint(":");
  Board_DebugPrint(steps);
  Board_DebugPrint(" ");

  //DebugPrint("initmove: ");
  //DebugPrint(dir);
  //DebugPrint(" : ");
  //DebugPrint(steps);
  //DebugPrint(" : ");
  //DebugPrint(motorspeed);
  //DebugPrint(" : ");
  //DebugPrintln(leds);
#if defined(ESP8266)
  unsigned long curspd = mySetupData->get_brdmsdelay();
  switch ( mySetupData->get_motorspeed() )
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
    Board_DebugPrintln("Can't set myfp2Timer correctly. Select another freq. or interval");
  }
#else
  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual)
  myfp2timer = timerBegin(0, 80, true);
  timerAttachInterrupt(myfp2timer, &onTimer, true);  // Attach onTimer function to our timer.

  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter)
  unsigned long curspd = mySetupData->get_brdmsdelay();
  switch ( mySetupData->get_motorspeed() )
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

unsigned long DriverBoard::getposition(void)
{
  return this->focuserposition;
}

void DriverBoard::setposition(unsigned long pos)
{
  this->focuserposition = pos;
}
