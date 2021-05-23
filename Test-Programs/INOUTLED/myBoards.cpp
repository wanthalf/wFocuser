// ======================================================================
// myBoards.cpp : myFP2ESP DRIVER BOARD CODE
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// (c) Copyright Paul P, 2021. All Rights Reserved. TMC22xx code
// ======================================================================

// ======================================================================
// Rules
// ======================================================================
// setstepmode()
// Write
// to change a stepmode, code must call driverboard->setstepmode(smval)
// which also updates mySetupData->set_brdstepmode(smval);
// Read
// code calls mySetupData->get_brdstepmode();

// setstallguard()
// Write
// to change a stall-guard value, code must call driverboard->setstallguard(sgval)
// which also updates mySetupData->set_stallguard(smgval);
// Read
// code calls mySetupData->get_stallguard();

// settmc2209current()
// Write
// to change tmc2209 current mA value, code must call driverboard->settmc2209current(val)
// which also updates mySetupData->set_tmc2209current(val);
// Read
// code calls mySetupData->get_tmc2209current();

// settmc2225current()
// Write
// to change tmc2225 current mA value, code must call driverboard->settmc2225current(val)
// which also updates mySetupData->set_tmc2225current(val);
// Read
// code calls mySetupData->get_tmc2225current();

// ======================================================================
// Includes
// ======================================================================
#include <Arduino.h>
#include "focuserconfig.h"                // boarddefs.h included as part of focuserconfig.h"
#include "generalDefinitions.h"
#include "myBoards.h"
#include "FocuserSetupData.h"

// ======================================================================
// Externs
// ======================================================================
extern SetupData   *mySetupData;
extern DriverBoard *driverboard;

extern volatile bool timerSemaphore;
extern volatile uint32_t stepcount;                // number of steps to go in timer interrupt service routine
#if defined(ESP8266)
// in esp8266, volatile data_type varname is all that is needed
#else
// in esp32, we should use a Mutex for access
extern portMUX_TYPE timerSemaphoreMux;
extern portMUX_TYPE  stepcountMux;
#endif


// ======================================================================
// Data
// ======================================================================
bool stepdir;                                 // direction of steps to move


// ======================================================================
// timer Interrupt
// ======================================================================
#if defined(ESP8266)
#include "ESP8266TimerInterrupt.h"
ESP8266Timer myfp2Timer;                      // use a unique name for the timer
#else
#include "esp32-hal-cpu.h"                    // so we can get CPU frequency
hw_timer_t * myfp2timer = NULL;               // use a unique name for the timer
#endif

/*
  if (stepcount  && !(driverboard->hpsw_alert() && stepdir == moving_in))

  stepcount   stepdir        hpsw_alert     action
  ----------------------------------------------------
    0           x             x             stop
    >0          moving_out    x             step
    >0          moving_in     False         step
    >0          moving_in     True          stop
*/

inline void asm2uS()  __attribute__((always_inline));

// On esp8266 with 80mHz clock a nop takes 1/80000000 second, i.e. one clock pulse, or 0.0000000125 of a second
// 2 Microseconds = 2 x 10-6 Seconds or 0.000001
// To get to 2uS for ESP8266 we will need 80 nop instructions
// On esp32 with 240mHz clock a nop takes ? 1/240000000 second or 0.000000004166 of a second
// To get to 2us for ESP32 we will need 240 nop instructions

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
  static bool mjob = false;                                     // motor job is running or not
  if (stepcount  && !(driverboard->hpsw_alert() && stepdir == moving_in))
  {
    driverboard->movemotor(stepdir, true);                      // move motor in direction and adjust position
    varENTER_CRITICAL(&stepcountMux);
    stepcount--;                                                // decrement steps to move
    varEXIT_CRITICAL(&stepcountMux);
    mjob = true;                                                // mark a running job
  }
  else
  {
    if (mjob == true)
    {
      varENTER_CRITICAL(&stepcountMux);
      stepcount = 0;                                            // just in case hps_alert was fired up
      varEXIT_CRITICAL(&stepcountMux);
      mjob = false;                                             // wait, and do nothing
      varENTER_CRITICAL(&timerSemaphoreMux);
      timerSemaphore = true;                                    // signal move complere
      varEXIT_CRITICAL(&timerSemaphoreMux);
    }
  }
}
#else
void IRAM_ATTR onTimer()
{
  static bool mjob = false;                                     // motor job is running or not
  if (stepcount  && !(driverboard->hpsw_alert() && stepdir == moving_in))
  {
    driverboard->movemotor(stepdir, true);
    varENTER_CRITICAL(&stepcountMux);
    stepcount--;
    varEXIT_CRITICAL(&stepcountMux);
    mjob = true;                                                // mark a running job
  }
  else
  {
    if (mjob == true)
    {
      varENTER_CRITICAL(&stepcountMux);
      stepcount = 0;              // just in case hps_alert was fired up
      varEXIT_CRITICAL(&stepcountMux);
      mjob = false;               // wait, and do nothing
      varENTER_CRITICAL(&timerSemaphoreMux);
      timerSemaphore = true;
      varEXIT_CRITICAL(&timerSemaphoreMux);
    }
  }
}
#endif

// ======================================================================
// Driverboard class
// ======================================================================
DriverBoard::DriverBoard(unsigned long startposition)
{
  do {
#if defined(ESP8266)
    // do nothing
#else
    // esp32
    clock_frequency = ESP.getCpuFreqMHz();                      // returns the CPU frequency in MHz as an unsigned 8-bit integer
    //DebugPrint("Clock Freq: ");
    //DebugPrintln(clock_frequency);
#endif

    varENTER_CRITICAL(&timerSemaphoreMux);                      // make sure timersemaphore is false when driverboard created
    timerSemaphore = false;
    varEXIT_CRITICAL(&timerSemaphoreMux);
    varENTER_CRITICAL(&stepcountMux);                           // make sure stepcount is 0 when driverboard created
    stepcount = 0;
    varEXIT_CRITICAL(&stepcountMux);

    boardnum = DefaultBoardNumber;

    // THESE NEED TO BE CREATED AT RUNTIME

    if ( boardnum == WEMOSDRV8825 || boardnum == PRO2EDRV8825 || boardnum == PRO2ESP32R3WEMOS || boardnum == WEMOSDRV8825H )
    {
      pinMode(mySetupData->get_brdenablepin(), OUTPUT);
      pinMode(mySetupData->get_brddirpin(), OUTPUT);
      pinMode(mySetupData->get_brdsteppin(), OUTPUT);
      digitalWrite(mySetupData->get_brdenablepin(), 1);
      digitalWrite(mySetupData->get_brdsteppin(), 0);
      // fixed step mode
    }
    else if ( boardnum == PRO2ESP32DRV8825 )
    {
      pinMode(mySetupData->get_brdenablepin(), OUTPUT);
      pinMode(mySetupData->get_brddirpin(), OUTPUT);
      pinMode(mySetupData->get_brdsteppin(), OUTPUT);
      digitalWrite(mySetupData->get_brdenablepin(), 1);
      digitalWrite(mySetupData->get_brdsteppin(), 0);
      pinMode(mySetupData->get_brdboardpins(0), OUTPUT);
      pinMode(mySetupData->get_brdboardpins(1), OUTPUT);
      pinMode(mySetupData->get_brdboardpins(2), OUTPUT);
      // restore step mode
      this->setstepmode( mySetupData->get_brdstepmode() );
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
      // restore step mode
      this->setstepmode( mySetupData->get_brdstepmode() );
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
      // restore step mode
      this->setstepmode( mySetupData->get_brdstepmode() );
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
      // restore step mode
      this->setstepmode( mySetupData->get_brdstepmode() );
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
      // restore step mode
      this->setstepmode( mySetupData->get_brdstepmode() );
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
      // restore step mode
      this->setstepmode( mySetupData->get_brdstepmode() );
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
      // restore step mode
      this->setstepmode( mySetupData->get_brdstepmode() );
    }
    else if ( boardnum == PRO2ESP32TMC2225 )
    {
#if (DRVBRD == PRO2ESP32TMC2225)
      // init tmc2225
      pinMode(mySetupData->get_brdenablepin(), OUTPUT);
      pinMode(mySetupData->get_brddirpin(), OUTPUT);
      pinMode(mySetupData->get_brdsteppin(), OUTPUT);
      digitalWrite(mySetupData->get_brdenablepin(), 1);         // high disables the driver chip
      digitalWrite(mySetupData->get_brdsteppin(), 0);
      pinMode(mySetupData->get_brdboardpins(0), OUTPUT);        // ms1
      pinMode(mySetupData->get_brdboardpins(1), OUTPUT);        // ms2
      this->init_TMC2225();                                     // set step mode handled by init_tmc2225
#endif
    }
    else if ( boardnum == PRO2ESP32TMC2209 || boardnum == PRO2ESP32TMC2209P )
    {
#if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P)
      // init tmc2209
      pinMode(mySetupData->get_brdenablepin(), OUTPUT);
      pinMode(mySetupData->get_brddirpin(), OUTPUT);
      pinMode(mySetupData->get_brdsteppin(), OUTPUT);
      digitalWrite(mySetupData->get_brdenablepin(), 1);         // high disables the driver chip
      digitalWrite(mySetupData->get_brdsteppin(), 0);
      pinMode(mySetupData->get_brdboardpins(0), OUTPUT);        // ms1
      pinMode(mySetupData->get_brdboardpins(1), OUTPUT);        // ms2
      this->init_tmc2209();                                     // set step mode handled by init_tmc2209()
#endif // #if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P)
    }
    // For all boards do the following
    this->focuserposition = startposition;                      // set default focuser position to same as mySetupData
    if ( init_homepositionswitch() == true )                    // initialize home position switch
    {
      Board_DebugPrintln("hpswpin enabled");
    }
    else
    {
      Board_DebugPrintln("hpswpin NOT enabled");
    }
  } while (0);
}

// destructor
DriverBoard::~DriverBoard()
{
  if ( boardnum    == PRO2EULN2003   || boardnum == PRO2ESP32ULN2003 || boardnum == PRO2EL298N  \
       || boardnum == PRO2ESP32L298N || boardnum == PRO2EL293DMINI   || boardnum == PRO2ESP32L293DMINI \
       || boardnum == PRO2EL9110S    || boardnum == PRO2ESP32L9110S)
  {
    delete myhstepper;
  }
  else if (boardnum == PRO2EL293DNEMA || boardnum == PRO2EL293D28BYJ48 )
  {
    delete mystepper;
  }
  else if (boardnum == PRO2ESP32TMC2225 || boardnum == PRO2ESP32TMC2209 )
  {
#if DRVBRD == PRO2ESP32TMC2225 || DRVBRD == PRO2ESP32TMC2209
    //delete mytmcstepper;  // tmc2209Stepper has no destructor
#endif
  }
}

bool DriverBoard::init_homepositionswitch(void)
{
  // Basic assumption rule: If associated pin is -1 then cannot set enable
  Board_DebugPrintln("init_homepositionswitch");
  if ( mySetupData->get_hpswitchenable() == 1)
  {
    pinMode(mySetupData->get_brdhpswpin(), INPUT_PULLUP);       // initialize the pin as
    return true;
  }
  return false;
}

void DriverBoard::init_tmc2209(void)
{
#if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P)
  // Specify the serial3 interface to the tmc2225
  mytmcstepper = new TMC2209Stepper(&SERIAL_PORT2, R_SENSE, DRIVER_ADDRESS);
  Serial2.begin(TMC2209SPEED);
  mytmcstepper->begin();
  mytmcstepper->pdn_disable(1);                                 // Use PDN/UART pin for communication
  mytmcstepper->mstep_reg_select(1);                            // Adjust stepMode from the registers
  mytmcstepper->I_scale_analog(0);                              // Adjust current from the registers
  mytmcstepper->toff(TOFF_VALUE);                               // use TMC22xx Calculations sheet to get these
  mytmcstepper->tbl(2);
  mytmcstepper->rms_current(mySetupData->get_tmc2209current()); // set driver current mA
  int sm = mySetupData->get_brdstepmode();                      // stepmode set according to mySetupData->get_brdstepmode()
  sm = (sm == STEP1) ? 0 : sm;                                  // handle full steps
  mytmcstepper->microsteps(sm);                                 // set micro stepping

  // stall guard settings
  mytmcstepper->semin(0);
  // lower threshold velocity for switching on smart energy CoolStep and StallGuard to DIAG output
  mytmcstepper->TCOOLTHRS(0xFFFFF);                             // 20bit max
  mytmcstepper->hysteresis_end(0);                              // use TMC22xx Calculations sheet to get these
  mytmcstepper->hysteresis_start(0);                            // use TMC22xx Calculations sheet to get these
  // StallGuard4 threshold [0... 255] level for stall detection. It compensates for
  // motor specific characteristics and controls sensitivity. A higher value gives a higher
  // sensitivity. A higher value makes StallGuard4 more sensitive and requires less torque to
  // indicate a stall. The double of this value is compared to SG_RESULT.
  // The stall output becomes active if SG_RESULT falls below this value.
  mytmcstepper->SGTHRS(mySetupData->get_stallguard());
  Board_DebugPrint("TMC2209 Status: ");
  Board_DebugPrintln( driver.test_connection() == 0 ? "OK" : "NOT OK" );
  Board_DebugPrint("Motor is ");
  Board_DebugPrintln(digitalRead(mySetupData->get_brdenablepin()) ? "DISABLED" : "ENABLED");
  Board_DebugPrint("stepMode is ");
  Board_DebugPrintln(driver.microsteps());
#endif // #if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P)
}

void DriverBoard::init_tmc2225(void)
{
#if (DRVBRD == PRO2ESP32TMC2225)
  mytmcstepper = new TMC2208Stepper(&SERIAL_PORT2);               // specify the serial2 interface to the tmc2225
  Serial2.begin(TMC2225SPEED);
  mytmcstepper.begin();
  mytmcstepper->pdn_disable(1);                                   // use PDN/UART pin for communication
  mytmcstepper->mstep_reg_select(true);
  mytmcstepper->I_scale_analog(0);                                // adjust current from the registers
  mytmcstepper->rms_current(mySetupData->get_tmc2225current());   // set driver current [recommended NEMA = 400mA, set to 300mA]
  mytmcstepper->toff(2);                                          // enable driver
  int sm = mySetupData->get_brdstepmode();                        // stepmode set according to mySetupData->get_brdstepmode();
  sm = (sm == STEP1) ? 0 : sm;                                    // handle full steps
  mytmcstepper->microsteps(sm);                                   // step mode = 1/4 - default specified in boardfile.jsn
  mytmcstepper->hysteresis_end(0);
  mytmcstepper->hysteresis_start(0);
  Board_DebugPrint("TMC2225 Status: ");
  Board_DebugPrintln( driver.test_connection() == 0 ? "OK" : "NOT OK" );
  Board_DebugPrint("Motor is ");
  Board_DebugPrintln(digitalRead(mySetupData->get_brdenablepin()) ? "DISABLED" : "ENABLED");
  Board_DebugPrint("stepMode is ");
  Board_DebugPrintln(driver.microsteps());
#endif // #if (DRVBRD == PRO2ESP32TMC2225)
}

// check hpsw state - switch or stall guard
// switch uses internal pullup, if hpsw is closed = low, so we need to invert state to return high when switch is closed
// if tmc2209 check stall guard
// if stall guard high then stall guard detected on DIAG pin - HIGH = activated
// hps_alert must return high if hpsw or stall guard is activated
bool DriverBoard::hpsw_alert(void)
{
  // if moving out OR hpsw is not enabled then return
  if ( stepdir == moving_out )
  {
    return false;
  }
  // if hpsw is not enabled then return false
  if ( mySetupData->get_hpswitchenable() == 0 )
  {
    return false;
  }
  // hpsw is enabled so check it
  else
  {
    // check tmc2209 boards for stall guard
    if ( DefaultBoardNumber == PRO2ESP32TMC2209 || DefaultBoardNumber == PRO2ESP32TMC2209P )
    {
      // DIAG pin, High if stall guard is detected
      return ( (bool) digitalRead(mySetupData->get_brdhpswpin()) );
    }
    // check physical HOMEPOSITIONSWITCH for all other boards
    else
    {
      // switch uses internal pullup, if hpsw is closed = low, so we need to invert state to return high when switch is closed
      return !( (bool)digitalRead(mySetupData->get_brdhpswpin()) );
    }
  }
}

int DriverBoard::getboardnumber(void)
{
  return this->DefaultBoardNumber;          // DRVBRD
}

int DriverBoard::getfixedstepmode(void)
{
  return this->brdfixedstepmode;            // FIXEDSTEPMODE
}

int DriverBoard::getstepsperrev(void)
{
  return this->brdstepsperrev;              // STEPSPERREVOLUTION
}

// ======================================================================
// Basic rule for setting stepmode in this order
// 1. Set mySetupData->set_brdstepmode(xx);       // this saves config setting
// 2. Set driverboard->setstepmode(xx);           // this sets the physical pins
// ======================================================================
void DriverBoard::setstepmode(int smode)
{
  do {
    if (boardnum == WEMOSDRV8825 || boardnum == PRO2EDRV8825 || boardnum == PRO2ESP32R3WEMOS || boardnum == WEMOSDRV8825H)
    {
      // for PRO2EDRV8825 stepmode is set in hardware jumpers, cannot set by software
      // mySetupData->set_brdstepmode(mySetupData->get_brdfixedstepmode());
      // ignore request
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
          break;
      }
      // update boardconfig.jsn
      mySetupData->set_brdstepmode(smode);
    }
    else if (boardnum    == PRO2EULN2003   || boardnum == PRO2ESP32ULN2003 \
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
          smode = STEP1;
          myhstepper->SetSteppingMode(SteppingMode::FULL);
          break;
      }
      // update boardconfig.jsn
      mySetupData->set_brdstepmode(smode);
    }
    else if (boardnum == PRO2EL293DNEMA || boardnum == PRO2EL293D28BYJ48 )
    {
      // update boardconfig.jsn
      mySetupData->set_brdstepmode(STEP1);
    }
    else if ( boardnum == PRO2ESP32TMC2225 || boardnum == PRO2ESP32TMC2209 || boardnum == PRO2ESP32TMC2209P )
    {
#if (DRVBRD == PRO2ESP32TMC2225 || DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P )
      smode = (smode < STEP1)   ? STEP1   : smode;
      smode = (smode > STEP256) ? STEP256 : smode;
      // handle full stepmode
      smode = (smode == STEP1) ? 0 : smode;
      mytmcstepper->microsteps(smode);
      // update boardconfig.jsn
      if (smode == 0)
      {
        smode = STEP1;
      }
      mySetupData->set_brdstepmode( smode );
      //Serial.print("sMode= "); Serial.println(driver.microsteps());
#endif // #if (DRVBRD == PRO2ESP32TMC2225 || DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P )
    }
  } while (0);
}

void DriverBoard::enablemotor(void)
{
  if (boardnum == WEMOSDRV8825     || boardnum == PRO2EDRV8825     || boardnum == PRO2ESP32DRV8825 || boardnum == PRO2ESP32R3WEMOS \
      || boardnum == WEMOSDRV8825H || boardnum == PRO2ESP32TMC2225 || boardnum == PRO2ESP32TMC2209 || boardnum == PRO2ESP32TMC2209P )
  {
    digitalWrite(mySetupData->get_brdenablepin(), 0);
    delay(1);                     // boards require 1ms before stepping can occur
  }
}

void DriverBoard::releasemotor(void)
{
  if (boardnum == WEMOSDRV8825     || boardnum == PRO2EDRV8825     || boardnum == PRO2ESP32DRV8825 || boardnum == PRO2ESP32R3WEMOS \
      || boardnum == WEMOSDRV8825H || boardnum == PRO2ESP32TMC2225 || boardnum == PRO2ESP32TMC2209 || boardnum == PRO2ESP32TMC2209P )
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

void DriverBoard::movemotor(byte ddir, bool updatefpos)
{
  //DebugPrint("movemotor() : ");
  //DebugPrintln(dir);
  // only ESP32 boards have in out leds
  stepdir = ddir;
  if (boardnum == PRO2ESP32ULN2003    || boardnum == PRO2ESP32L298N   || boardnum == PRO2ESP32L293DMINI || boardnum == PRO2ESP32L9110S \
      || boardnum == PRO2ESP32DRV8825 || boardnum == PRO2ESP32TMC2225 || boardnum == PRO2ESP32TMC2209   || boardnum == PRO2ESP32TMC2209P )
  {
    // Basic assumption rule: If associated pin is -1 then cannot set enable
    if ( mySetupData->get_inoutledstate() == 1)
    {
      ( stepdir == moving_in ) ? digitalWrite(mySetupData->get_brdinledpin(), 1) : digitalWrite(mySetupData->get_brdoutledpin(), 1);
    }
  }

  // do direction, enable and step motor
  if (boardnum == WEMOSDRV8825     || boardnum == PRO2EDRV8825     || boardnum == PRO2ESP32DRV8825 || boardnum == PRO2ESP32R3WEMOS \
      || boardnum == WEMOSDRV8825H || boardnum == PRO2ESP32TMC2225 || boardnum == PRO2ESP32TMC2209 || boardnum == PRO2ESP32TMC2209P )
  {
    if ( mySetupData->get_reversedirection() == 1 )
    {
      digitalWrite(mySetupData->get_brddirpin(), !stepdir);     // set Direction of travel
    }
    else
    {
      digitalWrite(mySetupData->get_brddirpin(), stepdir);      // set Direction of travel
    }
    digitalWrite(mySetupData->get_brdenablepin(), 0);           // Enable Motor Driver
    digitalWrite(mySetupData->get_brdsteppin(), 1);             // Step pin on
#if defined(ESP8266)
    asm1uS();                                                   // ESP8266 must be 2uS delay for DRV8825 chip
    asm1uS();
    asm1uS();
#else
    // ESP32
    if ( clock_frequency == 160 )
    {
      asm1uS();                                                 // ESP32 must be 2uS delay for DRV8825 chip
      asm1uS();
      asm1uS();
      asm1uS();
    }
    else
    {
      // assume clock frequency is 240mHz
      asm1uS();                                                 // ESP32 must be 2uS delay for DRV8825 chip
      asm1uS();
      asm1uS();
      asm1uS();
      asm1uS();
      asm1uS();
    }
#endif // #if defined(ESP8266)
    digitalWrite(mySetupData->get_brdsteppin(), 0);             // Step pin off
  }
  else if ( boardnum    == PRO2EULN2003   || boardnum == PRO2ESP32ULN2003   || boardnum == PRO2EL298N  || boardnum == PRO2ESP32L298N \
            || boardnum == PRO2EL293DMINI || boardnum == PRO2ESP32L293DMINI || boardnum == PRO2EL9110S || boardnum == PRO2ESP32L9110S )
  {
    if ( stepdir == moving_in )
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
    if ( stepdir == moving_in )
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
  if (boardnum    == PRO2ESP32ULN2003 || boardnum == PRO2ESP32L298N   || boardnum == PRO2ESP32L293DMINI || boardnum == PRO2ESP32L9110S \
      || boardnum == PRO2ESP32DRV8825 || boardnum == PRO2ESP32TMC2225 || boardnum == PRO2ESP32TMC2209   || boardnum == PRO2ESP32TMC2209P )
  {
    // Basic assumption rule: If associated pin is -1 then cannot set enable
    if ( mySetupData->get_inoutledstate() == 1)
    {
      ( stepdir == moving_in ) ? digitalWrite(mySetupData->get_brdinledpin(), 0) : digitalWrite(mySetupData->get_brdoutledpin(), 0);
    }
  }
  // update focuser position
  if ( updatefpos )
  {
    ( stepdir == moving_in ) ? this->focuserposition-- : this->focuserposition++;
  }
}

// when a move has completed [or halted], we need to detach/disable the interrupt timers
void DriverBoard::end_move(void)
{
#if defined(ESP8266)
  myfp2Timer.detachInterrupt();
#else
  timerAlarmDisable(myfp2timer);      // stop alarm
  timerDetachInterrupt(myfp2timer);   // detach interrupt
  timerEnd(myfp2timer);               // end timer
#endif
  Board_DebugPrintln("halt:");
  delay(10);
}

void DriverBoard::initmove(bool mdir, unsigned long steps)
{
  stepdir = mdir;
  varENTER_CRITICAL(&stepcountMux);        // make sure stepcount is 0 when driverboard created
  stepcount = steps;
  varEXIT_CRITICAL(&stepcountMux);
  DriverBoard::enablemotor();
  varENTER_CRITICAL(&timerSemaphoreMux);
  timerSemaphore = false;
  varEXIT_CRITICAL(&timerSemaphoreMux);

  Board_DebugPrint("initmove: ");
  Board_DebugPrint(mdir);
  Board_DebugPrint(" : ");
  Board_DebugPrint(steps);
  Board_DebugPrint(" : ");
  Board_DebugPrint(motorspeed);
  Board_DebugPrint(" : ");
  Board_DebugPrintln(leds);

#if defined(ESP8266)
  // ESP8266
  unsigned long curspd = mySetupData->get_brdmsdelay();     // get current board speed delay value
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
    Board_DebugPrintln("Cannot set myfp2Timer correctly. Select another freq. or interval");
  }
#else
  // ESP32
  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual)
  myfp2timer = timerBegin(0, 80, true);                     // timer-number, prescaler, count up (true) or down (false)
  timerAttachInterrupt(myfp2timer, &onTimer, true);         // our handler name, address of function int handler, edge=true

  unsigned long curspd = mySetupData->get_brdmsdelay();     // get current board speed delay value
  // handle the board step delays for TMC22xx steppers differently
  if ( this->boardnum == PRO2ESP32TMC2225 || this->boardnum == PRO2ESP32TMC2209 || this->boardnum == PRO2ESP32TMC2209P)
  {
    switch ( mySetupData->get_brdstepmode() )
    {
      case STEP1:
        curspd = curspd;
        break;
      case STEP2:
        curspd = curspd / 2;
        break;
      case STEP4:
        curspd = curspd / 4;
        break;
      case STEP8:
        curspd = curspd / 8;
        break;
      case STEP16:
        curspd = curspd / 16;
        break;
      case STEP32:
        curspd = curspd / 32;
        break;
      case STEP64:
        curspd = curspd / 64;
        break;
      case STEP128:
        curspd = curspd / 128;
        break;
      case STEP256:
        curspd = curspd / 256;
        break;
      default:
        curspd = curspd / 4;
        break;
    }
  }

  Board_DebugPrint("cursp: ");
  Board_DebugPrintln(curspd);

  // for TMC2209 stall guard setting varies with speed seeting so we need to adjust for best results
  // handle different speeds
  byte sgval = mySetupData->get_stallguard();
  Board_DebugPrint("stallguard: ");
  Board_DebugPrintln(sgval);
  Board_DebugPrint("motorspeed: ");
  Board_DebugPrintln(mySetupData->get_motorspeed());
  switch ( mySetupData->get_motorspeed() )
  {
    case 0: // slow, 1/3rd the speed
      curspd *= 3;
      // no need to change stall guard
      break;
    case 1: // med, 1/2 the speed
      curspd *= 2;
      sgval = sgval / 2;
      break;
    case 2: // fast, 1/1 the speed
      //curspd *= 1;               // obviously not needed
      sgval = sgval / 6;
      break;
  }
  Board_DebugPrint("SG value to write: "); Board_DebugPrintln(sgval);
#if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P )
  mytmcstepper->SGTHRS(sgval);
#endif
  // Set alarm to call onTimer function every interval value curspd (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(myfp2timer, curspd, true);   // timer for ISR, interval time, reload=true
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

byte DriverBoard::getstallguard(void)
{
  byte sgval = STALL_VALUE;                     // we must set it to something in case the next lines are not enabled
#if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P )
  sgval = mytmcstepper->SGTHRS();
#endif
  return sgval;
}

void DriverBoard::setstallguard(byte newval)
{
#if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P )
  mytmcstepper->SGTHRS(newval);
#endif
  mySetupData->set_stallguard(newval);
}

void DriverBoard::settmc2209current(int newval)
{
#if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P )
  mytmcstepper->rms_current(mySetupData->get_tmc2209current());     // Set driver current
#endif
  mySetupData->set_tmc2209current(newval);
}

void DriverBoard::settmc2225current(int newval)
{
#if (DRVBRD == PRO2ESP32TMC2225)
  mytmcstepper->rms_current(mySetupData->get_tmc2225current());     // Set driver current
#endif
  mySetupData->set_tmc2225current(newval);
}
