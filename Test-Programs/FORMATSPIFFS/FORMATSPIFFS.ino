// ======================================================================
// myFP2ESP FORMATSPIFFS
// ======================================================================
//
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.

#include <Arduino.h>

#if defined(ESP8266)                        // this "define(ESP8266)" comes from Arduino IDE
#include <FS.h>                             // include the SPIFFS library  
#else                                       // otherwise assume ESP32
#include "SPIFFS.h"
#endif
//#include <SPI.h>

void setup()
{
  Serial.begin(115200);

  Serial.println("Formatting SPIFFS now, please wait...");
  SPIFFS.format();
  Serial.println("Formatting complete");
}

void loop()
{
  
}
