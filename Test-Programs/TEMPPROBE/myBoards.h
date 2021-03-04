#include <Arduino.h>

#ifndef myBoards_h
#define myBoards_h

// YOU MUST CHANGE THIS TO MATCH THE STEPMODE SET IN HARDWARE JUMPERS ON THE PCB ESP8266-DRV
#define DRV8825TEPMODE        STEP16    // jumpers MS1/2/3 on the PCB for ESP8266

// stepper motor steps per full revolution using full steps - Non DRV8825 boards require this to be set
#define STEPSPERREVOLUTION 2048          // 28BYJ-48 stepper motor unipolar with ULN2003 board  
// #define STEPSPERREVOLUTION  200        // NEMA17
// #define STEPSPERREVOLUTION  400        // NEMA14HM11-0404S 0.9 motor
// #define STEPSPERREVOLUTION 1028        // 17HS13-0404S-PG5
// #define STEPSPERREVOLUTION 5370        // NEMA17HS13-0404S-PG27
// #define STEPSPERREVOLUTION 1036        // NEMA14HS13-0804S-PG5
// #define STEPSPERREVOLUTION 1036        // NEMA16HS13-0604S-PG5

#if (DRVBRD == PRO2EULN2003 || DRVBRD == PRO2EL298N || DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2EL9110S \
 || DRVBRD == PRO2EESP32ULN2003 || DRVBRD == PRO2EESP32L298N || DRVBRD == PRO2ESP32L293DMINI \
 || DRVBRD == PRO2ESP32L9110S )
#include "HalfStepperESP32.h"
#endif

class DriverBoard
{
  public:
    DriverBoard(byte, String, byte, byte);                          // driver drv8825 constructor
    DriverBoard(byte, String, byte, byte, byte, byte, byte, byte);  // driver uln2003, L298N, L293DMini

    // getter
    String getname(void);
    byte getmotorspeed(void);
    byte getstepmode(void);

    // setter
    void setstepdelay(int);
    void setstepmode(byte);
    void movemotor(byte);
    void enablemotor(void);
    void releasemotor(void);
    void setmotorspeed(byte);

  private:
#if (DRVBRD == PRO2EULN2003 || DRVBRD == PRO2EL298N || DRVBRD == PRO2EL293DMINI || DRVBRD == PRO2EL9110S \
 || DRVBRD == PRO2EESP32ULN2003 || DRVBRD == PRO2EESP32L298N || DRVBRD == PRO2ESP32L293DMINI \
 || DRVBRD == PRO2ESP32L9110S )
    HalfStepper* mystepper;
#endif
    int inputPins[4];                             // The input pin numbers
    byte boardtype;
    byte stepmode;
    int stepdelay;                                // time in milliseconds to wait between pulses when moving
    String boardname;
    int Step;                                     // used to control step count
};
#endif
