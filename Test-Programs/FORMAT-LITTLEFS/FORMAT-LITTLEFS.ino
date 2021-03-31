// format littlefs [esp8266 only]


#if defined(ESP8266)                        // this "define(ESP8266)" comes from Arduino IDE
#include <LittleFS.h>
#define SPIFFS LittleFS
#else                                       // otherwise assume ESP32
#include "SPIFFS.h"
#endif


void setup()
{
  Serial.begin(115200);
  Serial.println("Format LITTLEFS on esp8266 and SPIFFS on esp32");
  // mount SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("FS mounted");
  }
  else
  {
    Serial.println("FS not mounted");
    Serial.println("Formatting FS: please wait");
    SPIFFS.format();
    Serial.println("format complete");
  }
}
