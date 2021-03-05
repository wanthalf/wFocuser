#include <Arduino.h>
#include <ArduinoJson.h>        // version 5

#if defined(ESP8266)
 #include <FS.h>
#else
 #include "SPIFFS.h"
#endif

// write and read a json file for WIFI
// header for wifi stuff in JSON file
// file looks like
/*
{ "SSID":"string value", "PASSWORD":"string value" }
*/

// Arduino json library is installed so lets use that to parse the json file

// For AccessPointMode, your computer will connect to this network to gain
// access to the controller
// For StationMode, You will need to change mySSID and myPASSWORD to that of
// your existing wifi network so that the controller will connect to that
// network when it powers on
const String mySSID = "myfp2esp8266ap";
const String myPASSWORD = "myfp2esp8266ap";
const String filename = "/wifi.jsn";

void setup() 
{
  String s;
  String rSSID;
  String rPASSWORD;
   
  Serial.begin(115200); // Serielle Ausgabe aktivieren
  Serial.println("");  
    
  SPIFFS.begin(); // Filesystem mounten
  Serial.println("start to format SPIFFS, wait some seconds....");  
  SPIFFS.format();

  Serial.println("start to write file");
  File f = SPIFFS.open(filename, "w"); // Datei zum schreiben öffnen
  if (!f)  
    Serial.println("file open failed");
  else
  {
    StaticJsonBuffer<500> jsonBuffer;
    // Create the root object
    JsonObject& root = jsonBuffer.createObject();
    root["mySSID"] = mySSID;
    root["myPASSWORD"] = myPASSWORD;
    root.printTo(s);
    Serial.println(s);
    f.print(s);
  }
  f.close(); // Wir schließen die Datei
  Serial.println("file closed");

  Serial.println("\nstart to read file");
  f = SPIFFS.open(filename, "r"); // Datei zum lesen öffnen
  if (!f)  
    Serial.println("file open failed");
  else
  {
    Serial.println("file open to read");
    String data = f.readString(); // Inhalt der Textdatei wird gelesen...
    Serial.println("Content of file:");
    Serial.println(data); // ... und wieder ausgegeben
    Serial.println("----------------------------------");
    // allocate json buffer
    const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_ARRAY_SIZE(2) + 120;
    DynamicJsonBuffer jsonBuffer(capacity);

   // Parse JSON object
    JsonObject& root = jsonBuffer.parseObject(s);
    if (!root.success()) {
      Serial.println(F("Parsing failed!"));
      return;
    }
  
    // Decode JSON/Extract values
    Serial.println(F("Response:"));
    Serial.println(root["mySSID"].as<char*>());
    Serial.println(root["myPASSWORD"].as<char*>());

    if (data == s)
        Serial.println("well done");
  }
  Serial.println("file closed");      
  f.close();
}

void loop() {}
