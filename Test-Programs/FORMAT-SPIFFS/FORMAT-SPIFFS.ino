// format spiffs

#if defined(ESP8266)                        // this "define(ESP8266)" comes from Arduino IDE
#include <FS.h>                             // include the SPIFFS library  
#else                                       // otherwise assume ESP32
#include "SPIFFS.h"
#endif
#include <SPI.h>


void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("SPIFFS filesystem");
  // mount SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("FS mounted");
  }
  else
  {
    Serial.println("FS not mounted");
    Serial.println("Formatting SPIFFS: please wait");
    SPIFFS.format();
    Serial.println("format complete");
  }
}

void loop()
{

}
