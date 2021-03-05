// myFP2ESP - Firmware for ESP8266 and ESP32 myFocuserPro2 Controllers
// L293N STEPPER MOTOR TEST
// Remember to change your target CPU depending on board selection
//
// (c) Copyright Robert Brown 2014-2019. All Rights Reserved.
// (c) Copyright Holger M, 2019, who wrote large portions of code for state machine and esp comms
//
// CONTRIBUTIONS
// If you wish to make a small contribution in thanks for this project, please use PayPal and send the amount
// to user rbb1brown@gmail.com (Robert Brown). All contributions are gratefully accepted.
//
// 1. Set your DRVBRD [section 1] in this file so the correct driver board is used
// 2. Set your chipmodel in chipModels.h so that pins are mapped correctly
// 3. Set your target CPU to match the chipModel you defined
// 4. Set the correct hardware options [section 4] in this file to match your hardware
// 5. Compile and upload to your controller
//
// ----------------------------------------------------------------------------------------------
// 1: DEFINE CHIP MODEL
// ----------------------------------------------------------------------------------------------
#include "generalDefinitions.h"
#include "chipModels.h"             // include chip definitions and hardware mappings

// GOTO FILE chipModels.h and select the correct chip model that matches your PCB

// DO NOT CHANGE
#ifndef CHIPMODEL                   // error checking, please do NOT change
#halt // ERROR you must have CHIPMODEL defined in chipModels.h
#endif

// ----------------------------------------------------------------------------------------------
// 2: SPECIFY DRIVER BOARD HERE
// ----------------------------------------------------------------------------------------------
// DRIVER BOARDS - Please specify your driver board here, only 1 can be defined, see DRVBRD line
#include "myBoardTypes.h"

//Set DRVBRD to the correct driver board above, ONLY ONE!!!!
//#define DRVBRD PRO2EDRV8825
//#define DRVBRD PRO2EULN2003
#define DRVBRD PRO2EL298N
//#define DRVBRD PRO2EL293DMINI
//#define DRVBRD PRO2EL9110S
//#define DRVBRD PRO2ESP32DRV8825
//#define DRVBRD PRO2ESP32ULN2003
//#define DRVBRD PRO2ESP32L298N
//#define DRVBRD PRO2ESP32L293DMINI
//#define DRVBRD PRO2ESP32L9110S

// FOR ESP8266 DRV8825 YOU MUST CHANGE DRV8825TEPMODE TO MATCH MS1/2/3 JUMPERS ON PCB
// YOU DO THIS IN myBoards.h file

#include "myBoards.h"

// DO NOT CHANGE
#ifndef DRVBRD    // error checking, please do NOT change
#halt // ERROR you must have DRVBRD defined
#endif

// ----------------------------------------------------------------------------------------------
// 2: SPECIFY STEPPER MOTOR HERE
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 3: SPECIFY ESP32/ESP8266 CHIP TYPE
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 4: SPECIFY HARDWARE OPTIONS HERE
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 5: SPECIFY THE TYPE OF OLED DISPLAY HERE
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 6: SPECIFY THE CONTROLLER MODE HERE - ONLY ONE OF THESE MUST BE DEFINED
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 7. INCLUDES FOR WIFI
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 8. WIFI NETWORK SSID AND PASSWORD CONFIGURATION
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 9. DUCKDNS DOMAIN AND TOKEN CONFIGURATION
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 10. STATIC IP ADDRESS CONFIGURATION
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 11. FIRMWARE CODE START - INCLUDES AND LIBRARIES
// ----------------------------------------------------------------------------------------------
// Compile this with Arduino IDE 1.8.9 with ESP8266 Core library installed v2.5.2 [for ESP8266]
// Make sure target board is set to Node MCU 1.0 (ESP12-E Module) [for ESP8266]

// ----------------------------------------------------------------------------------------------
// 12. BLUETOOTH MODE - Do not change
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 13. GLOBAL DATA -- DO NOT CHANGE
// ----------------------------------------------------------------------------------------------

char programName[]  = "myFP2E.L298N";

DriverBoard* driverboard;

// ----------------------------------------------------------------------------------------------
// 14. CODE START - CHANGE AT YOUR OWN PERIL
// ----------------------------------------------------------------------------------------------

// STEPPER MOTOR ROUTINES
void setstepperspeed( byte spd )
{
  driverboard->setmotorspeed(spd);
}

void setsteppermode(byte smode)
{
  driverboard->setstepmode(smode);
}

void enablesteppermotor(void)
{
  driverboard->enablemotor();
}

void releasesteppermotor(void)
{
  driverboard->releasemotor();
}

void steppermotormove(byte dir )           // direction move_in, move_out ^ reverse direction
{
  driverboard->movemotor(dir);
}

void setup()
{
  Serial.begin(SERIALPORTSPEED);

  driverboard = new DriverBoard(PRO2EL298N, String(programName), STEP1, FAST, IN1L298N, IN2L298N, IN3L298N, IN4L298N);

  delay(5);
}

//_____________________ loop()___________________________________________

void loop()
{
  int i;
  Serial.println("L298N Stepper Motor Test");
  Serial.println("Full Steps: 400 steps: FAST");
  setsteppermode(STEP1);
  setstepperspeed(FAST);
  Serial.println("Motor spin clockwise");
  for ( i = 0; i < 400; i++ )
  {
    steppermotormove(0);
    delay(10);
  }
  delay(2000);
  Serial.println("Motor spin anti-clockwise");
  for ( i = 0; i < 400; i++ )
  {
    steppermotormove(1);
    delay(10);
  }
  delay(2000);

  Serial.println("L298N Stepper Motor Test");
  Serial.println("HALF Steps: 400 steps: SLOW");
  setsteppermode(STEP2);
  setstepperspeed(SLOW);
  Serial.println("Motor spin clockwise");
  for ( i = 0; i < 400; i++ )
  {
    steppermotormove(0);
    delay(10);
  }
  delay(2000);
  Serial.println("Motor spin anti-clockwise");
  for ( i = 0; i < 400; i++ )
  {
    steppermotormove(1);
    delay(10);
  }

  delay(2000);
}
