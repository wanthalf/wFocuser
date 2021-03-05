// myFP2ESP - Firmware for ESP8266 and ESP32 myFocuserPro2 Controllers
// TEST TEMPERATURE PROBE
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
// ----------------------------------------------------------------------------------------------
// PCB BOARDS
// ----------------------------------------------------------------------------------------------
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
#define DRVBRD PRO2EDRV8825
//#define DRVBRD PRO2EULN2003
//#define DRVBRD PRO2EL298N
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
// Remember to set CHIPMODEL to the correct chip you using in chipModels.h

// ----------------------------------------------------------------------------------------------
// 4: SPECIFY HARDWARE OPTIONS HERE
// ----------------------------------------------------------------------------------------------

#define TEMPERATUREPROBE 1

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

#include <OneWire.h>                        // https://github.com/PaulStoffregen/OneWire
#include <DallasTemperature.h>              // https://github.com/milesburton/Arduino-Temperature-Control-Library

// ----------------------------------------------------------------------------------------------
// 12. BLUETOOTH MODE - Do not change
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------
// 13. GLOBAL DATA -- DO NOT CHANGE
// ----------------------------------------------------------------------------------------------------------

byte tprobe1;                           // indicate if there is a probe attached to myFocuserPro2

OneWire oneWirech1(TEMPPIN);            // setup temperature probe
DallasTemperature sensor1(&oneWirech1);
DeviceAddress tpAddress;                // holds address of the temperature probe

// ----------------------------------------------------------------------------------------------
// 16. CODE START - CHANGE AT YOUR OWN PERIL
// ----------------------------------------------------------------------------------------------

float readtemp(byte new_measurement)
{
  static float lasttemp = 20.0;                 // start temp value
  if (!new_measurement)
    return lasttemp;                            // return latest measurement

  float result = sensor1.getTempCByIndex(0);    // get channel 1 temperature, always in celsius
  DebugPrint(F("Temperature = "));
  DebugPrintln(result);
  if (result > -40.0 && result < 80.0)
    lasttemp = result;
  else
    result = lasttemp;
  return result;
}

void settempprobeprecision(byte precision)
{
  sensor1.setResolution(tpAddress, precision); // set probe resolution, tpAddress must be global
}

// find the address of the DS18B20 sensor probe
byte findds18b20address()
{
  // look for probes, search the wire for address
  DebugPrintln(F("Searching for temperature probe"));
  if (sensor1.getAddress(tpAddress, 0))
  {
    DebugPrint(F("Temperature probe address found"));
    tprobe1 = 1;
  }
  else
  {
    DebugPrintln(F("Temperature probe NOT found"));
    tprobe1 = 0;
  }
  return tprobe1;
}

void Update_Temp(void)
{
  if (tprobe1 == 1)
  {
    static byte requesttempflag = 0;              // start with request
    static float tempval;

    if (requesttempflag)
    {
      tempval = readtemp(1);
      Serial.print("Temperature is : ");
      Serial.println(tempval, 3);
    }
    else
    {
      sensor1.requestTemperatures();
    }
    requesttempflag ^= 1; // toggle flag
  }
}

void setup()
{
  Serial.begin(SERIALPORTSPEED);

  pinMode(TEMPPIN, INPUT);                    // Configure GPIO pin for temperature probe
  Serial.println("Start temperature sensor");
  sensor1.begin();                            // start the temperature sensor1
  Serial.println("Get number of temperature sensors");
  tprobe1 = sensor1.getDeviceCount();         // should return 1 if probe connected
  Serial.print("Sensors found: ");
  Serial.println(tprobe1);
  Serial.println("Find temperature probe address");
  if (findds18b20address() == 1)
  {
    settempprobeprecision(TEMP_PRECISION); // set probe resolution
    Serial.print("Sensors found: ");
    Serial.println(tprobe1);
    sensor1.requestTemperatures();
  }
  else
  {
    Serial.println("Temperature probe address not found");
  }
  delay(1000);
}

//_____________________ loop()___________________________________________

void loop()
{
  Update_Temp();
  delay(1000);
}
