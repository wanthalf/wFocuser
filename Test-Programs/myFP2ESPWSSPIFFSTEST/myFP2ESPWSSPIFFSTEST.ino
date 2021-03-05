// esp32

// https://github.com/me-no-dev/ESPAsyncWebServer
// https://github.com/me-no-dev/AsyncTCP
// https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/
// https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
//
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

// accesspoint
IPAddress ip(192, 168, 4, 1);                     // AP static IP - you can change these values to change the IP
IPAddress dns(192, 168, 4, 1);                    // just set it to the same IP as the gateway
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

IPAddress myIP;
String ipStr;
WiFiServer myserver(4040);
WiFiClient myclient;                              // only one client supported, multiple connections denied
long rssi;


AsyncWebServer wserver(80);                       // webserver

const char* mySSID = "myfp2eap";                  // access point
const char* myPASSWORD = "myfp2eap";

char programName[] = "DRV8825ESP32";
char programVersion[] = "111";
char ProgramAuthor[]  = "(c) R BROWN 2019";
unsigned long fcurrentPosition = 5000;            // current focuser position
unsigned long ftargetPosition = 10000;            // target position

int webserverport = 80;

void wssendnotfound()
{
  
}

String processor(const String& var) {
  Serial.println(var);
  if (var == "WS_IPSTR")
  {
    return ipStr;
  }
  if (var == "WEBSERVERPORT")
  {
    return String(webserverport);
  }
  if ( var == "WSPROGRAMVERSION")
  {
    return String(programVersion);
  }
  if ( var == "WSPROGRAMNAME")
  {
    return String(programName);
  }
  if ( var == "WSCURRENTPOSITION")
  {
    return String(fcurrentPosition);
  }
  if ( var == "WSTARGETPOSITION" )
  {
    return String(ftargetPosition);
  }
  return String();
}

void setup()
{
  Serial.begin(115200);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // accesspoint
  WiFi.config(ip, dns, gateway, subnet);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(mySSID, myPASSWORD);
  myserver.begin();
  Serial.println(F("Get local IP address"));
  myIP = WiFi.localIP();
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  ipStr = String(myIP[0]) + "." + String(myIP[1]) + "." + String(myIP[2]) + "." + String(myIP[3]);

  // Route for root / web page
  wserver.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/wsindex.html", String(), false, processor);
  });

  wserver.on("/", HTTP_POST, [](AsyncWebServerRequest * request)
  {
    // code here to handle a put request
    // handle root
    request->send(SPIFFS, "/wsindex.html", String(), false, processor);
  });

  wserver.onNotFound([](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/wsnotfound.html", String(), false, processor);
  });


  // Start webserver
  wserver.begin();


}

void loop() {

}
