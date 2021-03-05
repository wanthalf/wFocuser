// ---------------------------------------------------------------------------
// TITLE: myFP2ESP DRIVER BOARD DEFINITIONS
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// COPYRIGHT
// ---------------------------------------------------------------------------
// (c) Copyright Robert Brown 2014-2020. All Rights Reserved.
// (c) Copyright Holger M, 2019-2020. All Rights Reserved.
// ---------------------------------------------------------------------------

#include "generalDefinitions.h"
#include "myBoardTypes.h"

#ifndef myBoards_h
#define myBoards_h

// ---------------------------------------------------------------------------
// 1: BOARD DEFINES -- DO NOT CHANGE
// ---------------------------------------------------------------------------
// Uncomment only your board - ONLY ONE BOARD SHOULD BE UNCOMMENTED

// ESP8266 Boards
//#define DRVBRD WEMOSDRV8825
//#define DRVBRD PRO2EULN2003
//#define DRVBRD PRO2EDRV8825
//#define DRVBRD PRO2EDRV8825BIG
//#define DRVBRD PRO2EL293DNEMA
//#define DRVBRD PRO2EL293D28BYJ48
//#define DRVBRD PRO2EL298N
//#define DRVBRD PRO2EL293DMINI
//#define DRVBRD PRO2EL9110S
// ESP32 Boards
//#define DRVBRD PRO2ESP32DRV8825
#define DRVBRD PRO2ESP32ULN2003
//#define DRVBRD PRO2ESP32L298N
//#define DRVBRD PRO2ESP32L293DMINI
//#define DRVBRD PRO2ESP32L9110S
//#define DRVBRD PRO2ESP32R3WEMOS

// THIS MUST MATCH THE STEPMODE SET IN HARDWARE JUMPERS ON THE PCB ESP8266-DRV
#define DRV8825TEPMODE    STEP1         // jumpers MS1/2/3 on the PCB for ESP8266

// stepper motor steps per full revolution using full steps
// WARNING: USE THE CORRECT ONE - IF YOU THEN CHANGE STEPMODE THE STEPS MOVED WILL BE INVALID
#define STEPSPERREVOLUTION 2048        // 28BYJ-48 stepper motor unipolar with ULN2003 board
//#define STEPSPERREVOLUTION  200        // NEMA17 FULL STEPPED
//#define STEPSPERREVOLUTION  400        // NEMA14HM11-0404S 0.9 motor FULL STEPPED
//#define STEPSPERREVOLUTION 1028        // 17HS13-0404S-PG5
//#define STEPSPERREVOLUTION 5370        // NEMA17HS13-0404S-PG27
//#define STEPSPERREVOLUTION 1036        // NEMA14HS13-0804S-PG5
//#define STEPSPERREVOLUTION 1036        // NEMA16HS13-0604S-PG5

// do not change, required for L293D motor shield
#define UNIPOLAR28BYJ48   1
#define BIPOLARNEMA       2
#define SPEEDBIPOLAR      48            // RPM speed of 28BYJ48 is max of 48 rpm
#define SPEEDNEMA         100           // RPM speed for NEMA motor

// ---------------------------------------------------------------------------
// DEFINITIONS FOR BOARDS: DO NOT CHANGE
// ---------------------------------------------------------------------------

#if ( DRVBRD == PRO2EULN2003   || DRVBRD == PRO2ESP32ULN2003  \
   || DRVBRD == PRO2EL298N     || DRVBRD == PRO2ESP32L298N    \
   || DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2ESP32L293MINI \
   || DRVBRD == PRO2EL9110S    || DRVBRD == PRO2ESPL9110S)
#include <HalfStepper.h>
#endif

#if (DRVBRD == PRO2EL293DNEMA || DRVBRD == PRO2EL293D28BYJ48 )
#include <Stepper.h>                    // needed for stepper motor and L293D shield, see https://github.com/adafruit/Adafruit-Motor-Shield-library
#endif

#if (DRVBRD == WEMOSDRV8825 )
#define TEMPPIN       4
#define I2CDATAPIN    2
#define I2CCLKPIN     1
#define DIRPIN        13                // D7 GPIOP13
#define STEPPIN       12                // D6 GPIO12
#define ENABLEPIN     14                // D5 GPIO14
#define MSPEED        8000
#endif
#if (DRVBRD == PRO2EDRV8825 )           // 
#define TEMPPIN       10
#define I2CDATAPIN    5
#define I2CCLKPIN     4
#define DIRPIN        13                // D7 GPIOP13
#define STEPPIN       12                // D6 GPIO12
#define ENABLEPIN     14                // D5 GPIO14
#define MSPEED        8000
#endif
#if (DRVBRD == PRO2EDRV8825BIG )        // DONE, TESTED WITH NEMA14 STEPPER MOTOR
#define TEMPPIN       2                 // temp probe does not work on SD3 for BigChip
#define I2CDATAPIN    5
#define I2CCLKPIN     4
#define DIRPIN        13                // D7 GPIOP13
#define STEPPIN       12                // D6 GPIO12
#define ENABLEPIN     14                // D5 GPIO14
#define MSPEED        8000
#endif
#if (DRVBRD == PRO2EULN2003 )           //
#define TEMPPIN       10
#define I2CDATAPIN    5
#define I2CCLKPIN     4
#define IN1           13
#define IN2           12
#define IN3           14
#define IN4           2
#define MSPEED        16500
#endif
#if (DRVBRD == PRO2EL293DNEMA)          // 
#define TEMPPIN       10                // Temperature somehows does not work now for this shield
#define I2CDATAPIN    12                // GPIO12 is D6
#define I2CCLKPIN     14                // GPIO14 is D5
#define IN1           5                 // PWM_A    
#define IN2           0                 // DIR B
#define IN3           4                 // DIR A
#define IN4           2                 // PWM_B 
#define MSPEED        16500
#endif
#if (DRVBRD == PRO2EL293D28BYJ48)   // 
#define TEMPPIN       10
#define I2CDATAPIN    12
#define I2CCLKPIN     14
#define IN1           5             // PWM_A    
#define IN2           0             // DIR B
#define IN3           4             // DIR A
#define IN4           2             // PWM_B 
#define MSPEED        16500
#endif
#if (DRVBRD == PRO2EL298N)          // DONE
#define TEMPPIN       10
#define I2CDATAPIN    5
#define I2CCLKPIN     4
#define IN1           13
#define IN2           12
#define IN3           14
#define IN4           2
#define MSPEED        16500         // crashed on 15000 for long move, 16500 for move of 30000 at fast = 0k
#endif
#if (DRVBRD == PRO2EL293DMINI)
#define TEMPPIN       10
#define I2CDATAPIN    5
#define I2CCLKPIN     4
#define IN1           13
#define IN2           12
#define IN3           14
#define IN4           2
#define MSPEED        16500
#endif
#if (DRVBRD == PRO2EL9110S)
#define TEMPPIN       10
#define I2CDATAPIN    5
#define I2CCLKPIN     4
#define IN1           13
#define IN2           12
#define IN3           14
#define IN4           2
#define MSPEED        16500
#endif
#if (DRVBRD == PRO2ESP32DRV8825 )   // DONE
#define TEMPPIN       13
#define I2CDATAPIN    21            // D21 is SDA
#define I2CCLKPIN     22            // D22 is SCL
#define DIRPIN        32
#define STEPPIN       33
#define ENABLEPIN     14
#define INPBPIN       34
#define OUTPBPIN      35
#define INLEDPIN      18
#define OUTLEDPIN     19
#define IRPIN         15
#define HPSWPIN       4
#define MS1           27
#define MS2           26
#define MS3           25
#define MSPEED        4000
#endif
#if (DRVBRD == PRO2ESP32ULN2003 || DRVBRD == PRO2ESP32L298N || DRVBRD == PRO2ESP32L293DMINI || DRVBRD == PRO2ESP32L9110S)
#define TEMPPIN       13
#define I2CDATAPIN    21            // D21 is SDA
#define I2CCLKPIN     22            // D22 is SCL
#define IN1           14
#define IN2           27
#define IN3           26
#define IN4           25
#define INPBPIN       34
#define OUTPBPIN      35
#define INLEDPIN      18
#define OUTLEDPIN     19
#define IRPIN         15
#define HPSWPIN       4
#define MSPEED        20000       // 20ms
#endif
#if (DRVBRD == PRO2ESP32R3WEMOS )
#define TEMPPIN       13
#define I2CDATAPIN    21
#define I2CCLKPIN     22
#define DIRPIN        26
#define STEPPIN       27
#define ENABLEPIN     14
#define IRPIN         15
#define HPSWPIN       4
#define MSPEED        8000
#endif

// ---------------------------------------------------------------------------
// DRIVER BOARD CLASS : DO NOT CHANGE
// ---------------------------------------------------------------------------
extern volatile bool timerSemaphore;
extern const char* DRVBRD_ID;
#if ( DRVBRD == PRO2EULN2003   || DRVBRD == PRO2ESP32ULN2003)
    //    HalfStepper* mystepper;
    HalfStepper mystepper(2048, IN1, IN3, IN4, IN2);
#endif
#if ( DRVBRD == PRO2EL298N     || DRVBRD == PRO2ESP32L298N    \
   || DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2ESP32L293MINI \
   || DRVBRD == PRO2EL9110S    || DRVBRD == PRO2ESPL9110S)
    HalfStepper* mystepper;
#endif
#if (DRVBRD == PRO2EL293DNEMA || DRVBRD == PRO2EL293D28BYJ48 )
    Stepper* mystepper;
#endif

class DriverBoard
{
  public:
    DriverBoard(byte);          // constructor
    ~DriverBoard(void);         // finalizer
    void      initmove(bool, unsigned long, byte, bool);
    uint32_t  halt(void);

    // getter
    byte getmotorspeed(void);
    byte getstepmode(void);
    int getstepdelay(void);

    // setter
    void setstepdelay(int);
    void setstepmode(byte);
    void movemotor(byte);
    void enablemotor(void);
    void releasemotor(void);

  private:
    int inputPins[4];                             // The input pin numbers
    byte boardtype;
    byte stepmode;
    int stepdelay;                                // time in milliseconds to wait between pulses when moving
    unsigned int clock_frequency;
    bool drvbrdleds;
};
#endif
