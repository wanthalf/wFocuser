// myFP2ESP - Firmware for ESP8266 and ESP32 myFocuserPro2 Controllers
// TEST FOR OLED DISPLAY
// Remember to change your target CPU depending on board selection
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
// ----------------------------------------------------------------------------------------------------------
// PCB BOARDS
// ----------------------------------------------------------------------------------------------------------
// ESP8266
//    ULN2003    https://aisler.net/p/QVXMBSWW
//    DRV8825    https://aisler.net/p/QVXMBSWW
// ESP32
//    ULN2003
//    DRV8825
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
// ONLY NEEDED FOR L293D MOTOR SHIELD - ALL OTHER BOARDS PLEASE IGNORE

// ----------------------------------------------------------------------------------------------
// 3: SPECIFY ESP32/ESP8266 CHIP TYPE
// ----------------------------------------------------------------------------------------------
// Remember to set CHIPMODEL to the correct chip you using in chipModels.h

// For ESP8266, remember to set DRV8825TEPMODE to the correct value if using WEMOS or NODEMCUV1 in myBoards.h

// ----------------------------------------------------------------------------------------------------------
// 4: SPECIFY HARDWARE OPTIONS HERE
// ----------------------------------------------------------------------------------------------------------

#define OLEDDISPLAY 1

#define SHOWSTARTSCRN 1

// ----------------------------------------------------------------------------------------------
// 5: SPECIFY THE TYPE OF OLED DISPLAY HERE
// ----------------------------------------------------------------------------------------------

//#define OLEDGRAPHICS 1
#define OLEDTEXT 2

// DO NOT CHANGE
#ifndef OLEDGRAPHICS
#ifndef OLEDTEXT
#halt //ERROR - you must have either OLEDGRAPHICS or OLEDTEXT defined
#endif
#endif

#ifdef OLEDGRAPHICS
#ifdef OLEDTEXT
#halt //ERROR - you must have either OLEDGRAPHICS or OLEDTEXT defined, not both
#endif
#endif

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

#include <Wire.h>                           // needed for I2C

#ifdef OLEDDISPLAY
#ifdef OLEDGRAPHICS
#include "SSD1306Wire.h"                    // TODO Holger need to put url of library here
#include "images.h"                         // TODO Holger need to provide file
#endif
#ifdef OLEDTEXT
#include <mySSD1306Ascii.h>
#include <mySSD1306AsciiWire.h>
#endif
#endif

// ----------------------------------------------------------------------------------------------------------
// 12. BLUETOOTH MODE - Do not change
// ----------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------
// 13. GLOBAL DATA -- DO NOT CHANGE
// ----------------------------------------------------------------------------------------------------------

#ifdef OLEDDISPLAY
#ifdef OLEDGRAPHICS
SSD1306* myoled;
#endif
#ifdef OLEDTEXT
SSD1306AsciiWire* myoled;
#endif
#endif

// ----------------------------------------------------------------------------------------------------------
// 16. CODE START - CHANGE AT YOUR OWN PERIL
// ----------------------------------------------------------------------------------------------------------

void setup()
{
  Serial.begin(SERIALPORTSPEED);

#ifdef OLEDDISPLAY
#ifdef OLEDGRAPHICS
  // TODO Holger to check for graphics OLED
  Wire.begin();
  // should check chiptype here
  myoled = new SSD1306Wire(OLED_ADDR , I2CDATAPIN, I2CCLOCKPIN);
  myoled->init();
  myoled->flipScreenVertically();
  myoled->setFont(ArialMT_Plain_10);
  myoled->setTextAlignment(TEXT_ALIGN_LEFT);
  myoled.clear();
#endif // oledgraphics
#ifdef OLEDTEXT
#if (CHIPMODEL == NODEMCUV1)
  Wire.begin();
#endif
#if (CHIPMODEL == WEMOS)
  Wire.begin();
#endif
#if (CHIPMODEL == ESP32WROOM)
  Wire.begin();
#endif
  myoled = new SSD1306AsciiWire();
  delay(5);
  // Setup the OLED
  myoled->begin(&Adafruit128x64, OLED_ADDR);
  delay(5);
  myoled->set400kHz();
  myoled->setFont(Adafruit5x7);
  myoled->clear();                                 // clrscr OLED
  myoled->Display_Normal();                        // black on white
  delay(5);
  myoled->Display_On();                            // display ON
  myoled->Display_Rotate(0);                       // portrait, not rotated
  myoled->Display_Bright();
  delay(5);
#ifdef SHOWSTARTSCRN
  myoled->println("Start OLED"); 
  myoled->println("Oled Test");
#endif // showstartscreen
#endif // oledtext
#endif // oleddisplay

  delay(2500);

#ifdef OLEDDISPLAY
#ifdef OLEDGRAPHICS
  // TODO Holger
#endif
#ifdef OLEDTEXT
  myoled->clear();
#endif
#endif
}

//_____________________ loop()___________________________________________

void loop()
{
#ifdef OLEDDISPLAY
#ifdef OLEDGRAPHICS
  // TODO Holger
#endif
#ifdef OLEDTEXT

  myoled->clear();
  myoled->Display_Normal();                  // black on white
  myoled->Display_On();                      // display ON
  myoled->Display_Rotate(0);                 // portrait, not rotated
  myoled->Display_Bright();
  myoled->set1X();
  myoled->println("Normal text");
  myoled->InverseCharOn();
  myoled->println("Inverse Text");
  myoled->InverseCharOff();
  delay(2000);
  myoled->clear();
  myoled->set2X();
  myoled->println("Big");
  myoled->set1X();
  myoled->println("Small");
  delay(4000);  
#endif
#endif


}
