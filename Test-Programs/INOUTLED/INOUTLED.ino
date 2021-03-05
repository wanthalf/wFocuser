// myFP2ESP - Firmware for ESP8266 and ESP32 myFocuserPro2 Controllers
// IN OUT LED TEST [ESP32 ONLY]
//
// (c) Copyright Robert Brown 2014-2019. All Rights Reserved.
// (c) Copyright Holger M, 2019, who wrote large portions of code for state machine and esp comms
//
// CONTRIBUTIONS
// If you wish to make a small contribution in thanks for this project, please use PayPal and send the amount
// to user rbb1brown@gmail.com (Robert Brown). All contributions are gratefully accepted.
//
// 1. Set your CHIPMODEL [section 1] based on selected chipType matching your PCB
// 2. Set your DRVBRD [section 2] in this file so the correct driver board is used
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
//#define DRVBRD PRO2ESP32DRV8825
#define DRVBRD PRO2ESP32ULN2003
//#define DRVBRD PRO2ESP32L298N
//#define DRVBRD PRO2ESP32L293DMINI
//#define DRVBRD PRO2ESP32L9110S

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
// Remember to set CHIPMODEL to the correct chip you using in chipModels.h

// ----------------------------------------------------------------------------------------------
// 4: SPECIFY HARDWARE OPTIONS HERE
// ----------------------------------------------------------------------------------------------

#define INOUTLEDS 1


// DO NOT CHANGE
#if (DRVBRD == PRO2EDRV8825 || DRVBRD == PRO2EULN2003 || DRVBRD == PRO2EL298N \
  || DRVBRD == PRO2EL293DMINI || DSRVBRD == PRO2EL9110S)
// no support for pushbuttons, inout leds, irremote
#ifdef INOUTLEDS
#halt // ERROR - INOUTLEDS not supported for WEMOS or NODEMCUV1 ESP8266 chips
#endif
#endif

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

// ----------------------------------------------------------------------------------------------
// 16. CODE START - CHANGE AT YOUR OWN PERIL
// ----------------------------------------------------------------------------------------------

void setup()
{
  Serial.begin(SERIALPORTSPEED);

  Serial.println("Setup LED's");
  pinMode(INLED, OUTPUT);
  pinMode(OUTLED, OUTPUT);
  digitalWrite(INLED, 0);
  digitalWrite(OUTLED, 0);
}

//_____________________ loop()___________________________________________

void loop()
{
  Serial.println("Both LEDs OFF");
  digitalWrite(INLED, 0);
  digitalWrite(OUTLED, 0);
  delay(2000);
  Serial.println("IN LED ON");
  Serial.println("OUT LED OFF");
  digitalWrite(INLED, 1);
  digitalWrite(OUTLED, 0);
  delay(2000);
  Serial.println("IN LED OFF");
  Serial.println("OUT LED ON");
  digitalWrite(INLED, 0);
  digitalWrite(OUTLED, 1);
  delay(2000);
  Serial.println("IN LED ON");
  Serial.println("OUT LED ON");
  digitalWrite(INLED, 1);
  digitalWrite(OUTLED, 1);
  delay(2000);
}
