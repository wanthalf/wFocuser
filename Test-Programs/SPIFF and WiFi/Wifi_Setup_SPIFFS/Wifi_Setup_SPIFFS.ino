#include <Arduino.h>
#include <ArduinoJson.h>        // version 6

#if defined(ESP8266)
 #include <FS.h>
#else
 #include "SPIFFS.h"
#endif

// wite and read a json file for WIFI
// header for wifi stuff in JSON file is "wifi"
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
const String mySSID = "my_local_SSID";          // put in here your WIFI SSID
const String myPASSWORD = "my_local_PASSWORD";  // put in here your WIFI PASSWORD 
const String filename = "/wifi.jsn";

void setup() 
{
  String s;
  String rSSID;
  String rPASSWORD;
   
  Serial.begin(115200); // Seriell interface activated
  Serial.println("");  
    
  SPIFFS.begin(); //  mount filesystem
  Serial.println("start to format SPIFFS, wait some seconds....");  
  SPIFFS.format();

  Serial.println("start to write file");
  File f = SPIFFS.open(filename, "w"); // open file to write
  if (!f)  
    Serial.println("file open failed");
  else
  {
    StaticJsonDocument<500> doc;
    doc["mySSID"] = mySSID;
    doc["myPASSWORD"] = myPASSWORD;
    serializeJson(doc,s);
    Serial.println(s);
    f.print(s);
  }
  f.close(); // close file
  Serial.println("file closed");

  Serial.println("\nstart to read file");
  f = SPIFFS.open(filename, "r"); // file open to read
  if (!f)  
    Serial.println("file open failed");
  else
  {
    Serial.println("file open to read");
    String data = f.readString(); // read content of the text file
    Serial.println("Content of file:");
    Serial.println(data); // ... and print on serial 
    Serial.println("----------------------------------");
    // allocate json buffer
    const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_ARRAY_SIZE(2) + 120;
    DynamicJsonDocument doc(capacity);

   // Parse JSON object
    DeserializationError error = deserializeJson(doc,data);
    if (error) {
      Serial.println("Deserialization failed!");
      Serial.println(error.c_str());
      return;
    }
  
    // Decode JSON/Extract values
    Serial.println("Response:");
    Serial.println(doc["mySSID"].as<char*>());
    Serial.println(doc["myPASSWORD"].as<char*>());

    if (data == s)
        Serial.println("well done");
  }
  Serial.println("file closed");      
  f.close();
}

void loop() {}
