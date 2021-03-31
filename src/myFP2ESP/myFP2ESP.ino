// ======================================================================
// myFP2ESP myp2esp.ino FIRMWARE OFFICIAL RELEASE 212
// ======================================================================
// myFP2ESP Firmware for ESP8266 and ESP32 myFocuserPro2 WiFi Controllers
// Supports Driver boards DRV8825, ULN2003, L298N, L9110S, L293DMINI, L293D
// ESP8266  OLED display, Temperature Probe
// ESP32    OLED display, Temperature Probe, Push Buttons, In/Out LED's,
//          Infrared Remote, Bluetooth
// Supports modes, ACCESSPOINT, STATIONMODE, BLUETOOTH, LOCALSERIAL, WEBSERVER,
//          ASCOMREMOTE
// Remember to change your target CPU depending on board selection
//
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// (c) Copyright Pieter P - OTA code and SPIFFs file handling/upload based on examples

// ======================================================================
// SPECIAL LICENSE
// ======================================================================
// This code is released under license. If you copy or write new code based
// on the code in these files. you MUST include to link to these files AND
// you MUST include references to the authors of this code.

// ======================================================================
// CONTRIBUTIONS
// ======================================================================
// It is costly to continue development and purchase boards and components.
// Your support is needed to continue development of this project. Please
// contribute to this project, and use PayPal to send your donation to user
// rbb1brown@gmail.com (Robert Brown). All contributions are gratefully accepted.

#include "boarddefs.h"                    // include driver board and motor high level definitions
#include "generalDefinitions.h"           // include global definitions
#include "myBoards.h"                     // include driverboard class definitions

// ======================================================================
// INCLUDES
// ======================================================================
#include "focuserconfig.h"

#undef DEBUG_ESP_HTTP_SERVER                  // needed sometimes to prevent serial output from WiFiServerlibrary

#include <WiFiServer.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#if defined(ESP8266)                        // this "define(ESP8266)" comes from Arduino IDE
#undef DEBUG_ESP_HTTP_SERVER                // prevent messages from WiFiServer 
#include <ESP8266WiFi.h>
#include <FS.h>                             // include the SPIFFS library  
#else                                       // otherwise assume ESP32
#include <WiFi.h>
#include "SPIFFS.h"
#endif
#include <SPI.h>
#include "FocuserSetupData.h"

// ======================================================================
// OVERVIEW: TO PROGRAM THE FIRMWARE
// ======================================================================
// 1. Set your DRVBRD in focuserconfig.h so the correct driver board is used
// 2. For specific boards set the FIXEDSTEPMODE in focuserconfig.h
// 3. For specific boards set the STEPSPERREVOLUTION in focuserconfig.h
// 4. Enable Display type [if fitted[ in focuserconfig.h
// 5. Set hardware options for JoyStick/IRRemote [esp32 only] in focuserconfig.h
// 6. Set the controller mode in focuserconfig.h
// 7. Set your target CPU to match the correct CPU for your board
// 8. Compile and upload to your controller
// 9. Upload the sketch data files

// ======================================================================
// 1: SPECIFY DRIVER BOARD in 1: focuserconfig.h
// ======================================================================
// Please specify your driver board [DRVBRD] in focuserconfig.h

// ======================================================================
// 2: SPECIFY FIXEDSTEPMODE in 2: focuserconfig.h
// ======================================================================
// For specific boards, specify the correct FIXEDSTEPMODE focuserconfig.h

// ======================================================================
// 3: SPECIFY STEPSPERREVOLUTION in 3: focuserconfig.h
// ======================================================================
// For specific boards, specify the correct STEPSPERREVOLUTION focuserconfig.h

// ======================================================================
// 4: SPECIFY DISPLAY OPTIONS IN 4: focuserconfig.h
// ======================================================================
// Specify your display options in focuserconfig.h, such as OLEDTEXT

// ======================================================================
// 5: SPECIFY HARDWARE OPTIONS IN 5: focuserconfig.h
// ======================================================================
// Specify your controller options in focuserconfig.h, such as INFRAREDREMOTE

// ======================================================================
// 6: SPECIFY THE CONTROLLER MODE IN 6: focuserconfig.h
// ======================================================================
// Please specify your controller mode in focuserconfig.h, such as ACCESSPOINT
// and MANAGEMENT

// ======================================================================
// 7: WIFI NETWORK CREDENTIALS: SSID AND PASSWORD
// ======================================================================
// 1. For access point mode this is the network you connect to
// 2. For station mode, change mySSID and myPASSWORD to match your network details

char mySSID[64]     = "myfp2eap";
char myPASSWORD[64] = "myfp2eap";

// Alternative network credentials if initial details above did not work
char mySSID_1[64]     = "FireFox";            // alternate network id
char myPASSWORD_1[64] = "AllYeWhoEnter";      // alternate network id


// ======================================================================
// 8: STATIC IP ADDRESS CONFIGURATION
// ======================================================================
// must use static IP if using duckdns or as an Access Point
#ifndef STATICIPON
#define STATICIPON    1
#endif
#ifndef STATICIPOFF
#define STATICIPOFF   0
#endif
//int staticip = STATICIPON;                  // IP address specified by controller - must be defined correctly
int staticip = STATICIPOFF;                   // IP address is generated by network device and is dynamic and can change

#ifdef ACCESSPOINT
// By default the Access point should be 192.168.4.1 - DO NOT CHANGE
IPAddress ip(192, 168, 4, 1);                 // AP static IP - you can change these values to change the IP
IPAddress dns(192, 168, 4, 1);                // just set it to the same IP as the gateway
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);
#endif // #ifdef ACCESSPOINT

#ifdef STATIONMODE
// These need to reflect your current network settings - 192.168.x.21 - change x
// This has to be within the range for dynamic ip allocation in the router
// No effect if staticip = STATICIPOFF
IPAddress ip(192, 168, 2, 21);                // station static IP - you can change these values to change the IP
IPAddress dns(192, 168, 2, 1);                // just set it to the same IP as the gateway
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);
#endif // #ifdef STATIONMODE


// ======================================================================
// 9: BLUETOOTH MODE NAME
// ======================================================================
#ifdef BLUETOOTHMODE
String BLUETOOTHNAME = "MYFP3ESP32BT";      // default name for Bluetooth controller, this name you can change
#endif // #ifdef BLUETOOTHMODE


// ======================================================================
// 10: mDNS NAME: Name must be alphabetic chars only, lowercase
// ======================================================================
#ifdef MDNSSERVER
char mDNSNAME[] = "myfp2eap";               // mDNS name will be myfp2eap.local
#endif // #ifdef MDNSSERVER


// ======================================================================
// 11: OTAUPDATES (OVER THE AIR UPDATE) SSID AND PASSWORD CONFIGURATION
// ======================================================================
// You can change the values for OTANAME and OTAPassword if required
#ifdef OTAUPDATES
#include <ArduinoOTA.h>
const char *OTAName = "ESP8266";            // the username and password for the OTA service
const char *OTAPassword = "esp8266";
#endif // #ifdef OTAUPDATES


// ======================================================================
// 12: DUCKDNS DOMAIN AND TOKEN CONFIGURATION
// ======================================================================
// if using DuckDNS you need to set these next two parameters, duckdnsdomain
// and duckdnstoken, BUT you cannot use DuckDNS with ACCESSPOINT, BLUETOOTHMODE
// or LOCALSERIAL mode
#ifdef USEDUCKDNS
const char* duckdnsdomain = "myfp2erobert.duckdns.org";
const char* duckdnstoken = "0a0379d5-3979-44ae-b1e2-6c371a4fe9bf";
#endif // #ifdef USEDUCKDNS


// ======================================================================
// 11: BLUETOOTH MODE - Do not change
// ======================================================================
#ifdef BLUETOOTHMODE
#include "BluetoothSerial.h"                  // needed for Bluetooth comms
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error "Bluetooth Not enabled"
#endif
BluetoothSerial SerialBT;                     // define BT adapter to use
String btline;                                // buffer for serial data
#endif // BLUETOOTHMODE

// ======================================================================
// FIRMWARE CODE START - INCLUDES AND LIBRARIES
// ======================================================================

// Project specific includes - DO NOT CHANGE
#if defined(LOCALSERIAL)
#include "ESPQueue.h"                         // by Steven de Salas
Queue queue(QUEUELENGTH);                     // receive serial queue of commands
String serialline;                            // buffer for serial data
#endif // #if defined(LOCALSERIAL)

#if defined(ACCESSPOINT) || defined(STATIONMODE)
IPAddress  ESP32IPAddress;
//String     ServerLocalIP;
WiFiServer myserver(SERVERPORT);
WiFiClient myclient;                          // only one client supported, multiple connections denied
IPAddress  myIP;
#endif // #if defined(ACCESSPOINT) || defined(STATIONMODE)

#include "temp.h"
TempProbe *myTempProbe;

#include "displays.h"
OLED_NON *myoled;

// ======================================================================
// GLOBAL DATA -- DO NOT CHANGE
// ======================================================================

//           reversedirection
//__________________________________
//               0   |   1
//__________________________________
//moving_out  1||  1   |   0
//moving_in   0||  0   |   1

int DefaultBoardNumber  = DRVBRD;           // use this to create a default board configuration
int brdfixedstepmode    = FIXEDSTEPMODE;    // only used by boards WEMOSDRV8825H, WEMOSDRV8825, PRO2EDRV8825BIG, PRO2EDRV8825
int brdstepsperrev      = STEPSPERREVOLUTION;

DriverBoard*  driverboard;
unsigned long ftargetPosition;              // target position
volatile bool halt_alert;

bool    displayfound;
byte    isMoving;                           // is the motor currently moving
char    ipStr[16] = "000.000.000.000";      // shared between BT mode and other modes

long    rssi;                               // network signal strength
int     packetsreceived;
int     packetssent;
bool    mdnsserverstate;                    // states for services, RUNNING | STOPPED
bool    webserverstate;
bool    ascomserverstate;
bool    ascomdiscoverystate;
bool    managementserverstate;
bool    tcpipserverstate;
bool    otaupdatestate;
bool    duckdnsstate;
bool    displaystate;                       // true if a display was found
bool    reboot;                             // flag used to indicate a reboot has occurred
int     tprobe1;                            // true if a temperature probe was detected
float   lasttemp;                           // last valid temp reading

SetupData *mySetupData;                     // focuser data

#if defined(ESP8266)
#undef DEBUG_ESP_HTTP_SERVER
#include <ESP8266WebServer.h>
extern ESP8266WebServer mserver;
extern ESP8266WebServer *ascomserver;
#else
#include <WebServer.h>
extern WebServer mserver;
extern WebServer *ascomserver;
#endif // if defined(esp8266)

extern void start_management(void);
extern void start_ascomremoteserver(void);
extern void checkASCOMALPACADiscovery(void);
extern void start_webserver(void);

// ======================================================================
// FIRMWARE CODE START - CHANGE AT YOUR OWN PERIL
// ======================================================================
#include "comms.h"                                // do not change or move

// check hpsw, switch uses internal pullup, pulled low when closed
// if hpsw is closed = low, !() closed means return high, return false if open
bool HPS_alert()
{
  // Basic assumption rule: If associated pin is -1 then cannot set enable
  if ( mySetupData->get_hpswitchenable() == 1)
  {
    return !((bool)digitalRead(mySetupData->get_brdhpswpin()));
  }
  else
  {
    return false;
  }
}

// ======================================================================
// INFRARED REMOTE CONTROLLER - CHANGE AT YOUR OWN PERIL
// ======================================================================
#ifdef INFRAREDREMOTE
#include <myfp2eIRremoteESP8266.h>                    // use cut down version to save spave
#include <myfp2eIRrecv.h>                             // unable to turn off all options by using a define
#include <myfp2eIRutils.h>
#include "irremotemappings.h"
const uint16_t RECV_PIN = IRPIN;
//TODO This is an issue - mySetupData->get_brdirpin()
IRrecv irrecv(RECV_PIN);
decode_results results;

void update_irremote()
{
  // check IR
  if (irrecv.decode(&results))
  {
    int adjpos = 0;
    static long lastcode;
    if ( results.value == KEY_REPEAT )
    {
      results.value = lastcode;                   // repeat last code
    }
    else
    {
      lastcode = results.value;
    }
    if ( (isMoving == 1) && (lastcode == IR_HALT))
    {
      halt_alert = true;
    }
    else
    {
      switch ( lastcode )
      {
        case IR_SLOW:
          mySetupData->set_motorSpeed(SLOW);
          break;
        case IR_MEDIUM:
          mySetupData->set_motorSpeed(MED);
          break;
        case IR_FAST:
          mySetupData->set_motorSpeed(FAST);
          break;
        case IR_IN1:
          adjpos = -1;
          break;
        case IR_OUT1:
          adjpos = 1;
          break;
        case IR_IN10:
          adjpos = -10;
          break;
        case IR_OUT10:
          adjpos = 10;
          break;
        case IR_IN50:
          adjpos = -50;
          break;
        case IR_OUT50:
          adjpos = 50;
          break;
        case IR_IN100:
          adjpos = -100;
          break;
        case IR_OUT100:
          adjpos = 100;
          break;
        case IR_SETPOSZERO:                         // 0 RESET POSITION TO 0
          adjpos = 0;
          ftargetPosition = 0;
          driverboard->setposition(0);
          mySetupData->set_fposition(0);
          break;
        case IR_PRESET0:
          ftargetPosition = mySetupData->get_focuserpreset(0);
          break;
        case IR_PRESET1:
          ftargetPosition = mySetupData->get_focuserpreset(1);
          break;
        case IR_PRESET2:
          ftargetPosition = mySetupData->get_focuserpreset(2);
          break;
        case IR_PRESET3:
          ftargetPosition = mySetupData->get_focuserpreset(3);
          break;
        case IR_PRESET4:
          ftargetPosition = mySetupData->get_focuserpreset(4);
          break;
      } // switch(lastcode)
    } // if ( (isMoving == 1) && (lastcode == IR_HALT))
    irrecv.resume();                              // Receive the next value
    long newpos;
    if ( adjpos < 0 )
    {
      newpos = mySetupData->get_fposition() + adjpos;
      newpos = (newpos < 0 ) ? 0 : newpos;
      ftargetPosition = newpos;
    }
    else if ( adjpos > 0)
    {
      newpos = mySetupData->get_fposition() + adjpos;
      newpos = (newpos > mySetupData->get_maxstep()) ? mySetupData->get_maxstep() : newpos;
      ftargetPosition = newpos;
    }
  }
}

void init_irremote(void)
{
  Setup_DebugPrintln("init_irremote");
  irrecv.enableIRIn();                            // Start the IR
}
#endif // #ifdef INFRAREDREMOTE

// ======================================================================
// JOYSTICK - CHANGE AT YOUR OWN PERIL
// ======================================================================
#if defined(JOYSTICK1) || defined(JOYSTICK2)
#include "joystick.h"

#ifdef JOYSTICK2
volatile int joy2swstate;
#endif

// 2-AXIS Analog Thumb Joystick for Arduino
#ifdef JOYSTICK1
void update_joystick1(void)
{
  int joyval;

  DebugPrintln("update joystick");
  joyval = analogRead(JOYINOUTPIN);
  DebugPrint("Raw joyval:");
  DebugPrintln(joyval);
  if ( joyval < (JZEROPOINT - JTHRESHOLD) )
  {
    ftargetPosition--;                            // move IN
    DebugPrint("X IN joyval:");
    DebugPrint(joyval);
  }
  else if ( joyval > (JZEROPOINT + JTHRESHOLD) )
  {
    ftargetPosition++;                            // move OUT
    if ( ftargetPosition > mySetupData->get_maxstep())
    {
      ftargetPosition = mySetupData->get_maxstep();
    }
    joyval = joyval - (JZEROPOINT + JTHRESHOLD);
    if ( joyval < 0 )
    {
      joyval = JZEROPOINT - joyval;
    }
    DebugPrint(JOYSTICKXOUTVALSTR);
    DebugPrint(joyval);
  }
}

void init_joystick1(void)
{
  // perform any inititalisations necessary
  // for future use
  pinMode(JOYINOUTPIN, INPUT);
  pinMode(JOYOTHERPIN, INPUT);
}
#endif // #ifdef JOYSTICK1

// Keyes KY-023 PS2 style 2-Axis Joystick
#ifdef JOYSTICK2
void IRAM_ATTR joystick2sw_isr()
{
  joy2swstate = !(digitalRead(JOYOTHERPIN));      // joy2swstate will be 1 when switch is pressed
}

void update_joystick2(void)
{
  int joyval;

  joyval = analogRead(JOYINOUTPIN);               // range is 0 - 4095, midpoint is 2047
  DebugPrint("Raw joyval:");
  DebugPrintln(joyval);
  if ( joyval < (JZEROPOINT - JTHRESHOLD) )
  {
    ftargetPosition--;                            // move IN
    DebugPrint("X IN joyval:");
    DebugPrint(joyval);
  }
  else if ( joyval > (JZEROPOINT + JTHRESHOLD) )
  {
    ftargetPosition++;                            // move OUT
    if ( ftargetPosition > mySetupData->get_maxstep())
    {
      ftargetPosition = mySetupData->get_maxstep();
    }
    joyval = joyval - (JZEROPOINT + JTHRESHOLD);
    if ( joyval < 0 )
    {
      joyval = JZEROPOINT - joyval;
    }
    DebugPrint("X OUT joyval:");
    DebugPrint(joyval);
  }

  if ( joy2swstate == 1)                          // switch is pressed
  {
    // user defined code here
    // could be a halt
    // could be a home
    // could be a preset
    // insert code here

    joy2swstate = 0;                              // finally reset joystick switch state
  }
}

void init_joystick2(void)
{
  pinMode(JOYINOUTPIN, INPUT);
  pinMode(JOYOTHERPIN, INPUT_PULLUP);
  // setup interrupt, falling, when switch is pressed, pin falls from high to low
  attachInterrupt(JOYOTHERPIN, joystick2sw_isr, FALLING);
  joy2swstate = 0;
}
#endif // #ifdef JOYSTICK2

#endif // #if defined(JOYSTICK1) || defined(JOYSTICK2)

// ======================================================================
// PUSHBUTTONS - CHANGE AT YOUR OWN PERIL
// ======================================================================
bool init_pushbuttons(void)
{
  Setup_DebugPrint("initPB: ");
  if ( (mySetupData->get_brdpb1pin() == 1) && (mySetupData->get_brdpb2pin() == 1) )
  {
    // Basic assumption rule: If associated pin is -1 then cannot set enable
    if ( mySetupData->get_pbenable() == 1)
    {
      pinMode(mySetupData->get_brdpb1pin(), INPUT);
      pinMode(mySetupData->get_brdpb2pin(), INPUT);
      Setup_DebugPrintln("enabled");
      return true;
    }
    else
    {
      Setup_DebugPrintln("disabled");
    }
  }
  else
  {
    Setup_DebugPrintln("not permitted");
  }
  return false;
}

void update_pushbuttons(void)
{
  DebugPrint("updatePB: ");
  if ( (mySetupData->get_brdpb1pin() == 1) && (mySetupData->get_brdpb2pin() == 1) )
  {
    // Basic assumption rule: If associated pin is -1 then cannot set enable
    if ( mySetupData->get_pbenable() == 1)
    {
      long newpos;
      // PB are active high - pins float low if unconnected
      if ( digitalRead(mySetupData->get_brdpb1pin()) == 1 )                // is pushbutton pressed?
      {
        newpos = ftargetPosition - 1;
        newpos = (newpos < 0 ) ? 0 : newpos;
        ftargetPosition = newpos;
        DebugPrintln("pb1 updated");
      }
      if ( digitalRead(mySetupData->get_brdpb2pin()) == 1 )
      {
        newpos = ftargetPosition + 1;
        // an unsigned long range is 0 to 4,294,967,295
        // when an unsigned long decrements from 0-1 it goes to largest +ve value, ie 4,294,967,295
        // which would in likely be much much greater than maxstep
        newpos = (newpos > (long) mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : newpos;
        ftargetPosition = newpos;
        DebugPrintln("pb2 updated");
      }
    }
    else
    {
      DebugPrintln("disabled");
    }
  }
  else
  {
    DebugPrintln("not permitted");
  }
}

// ======================================================================
// mDNS SERVER - CHANGE AT YOUR OWN PERIL
// ======================================================================
#ifdef MDNSSERVER
#if defined(ESP8266)
#include <ESP8266mDNS.h>
#else
#include <ESPmDNS.h>
#endif

// MDNS service. find the device using dnsname.local
void start_mdns_service(void)
{
  // Set up mDNS responder:
  // the fully-qualified domain name is "mDNSNAME.local"
  Setup_DebugPrintln("MDNS: ");
#if defined(ESP8266)
  if (!MDNS.begin(mDNSNAME, WiFi.localIP()))      // ESP8266 supports additional parameter for IP
#else
  if (!MDNS.begin(mDNSNAME))                      // ESP32 does not support IPaddress parameter
#endif
  {
    Setup_DebugPrintln("Not started");
    mdnsserverstate = STOPPED;
  }
  else
  {
    Setup_DebugPrintln("Started");
    // Add service to MDNS-SD, MDNS.addService(service, proto, port)
    MDNS.addService("http", "tcp", MDNSSERVERPORT);
    mdnsserverstate = RUNNING;
  }
  delay(10);                      // small pause so background tasks can run
}

void stop_mdns_service(void)
{
  Setup_DebugPrint("mdns: ");
  if ( mdnsserverstate == RUNNING )
  {
#if defined(ESP8266)
    // ignore
    // esp8266 library has no end() function to release mdns
#else
    MDNS.end();
#endif
    Setup_DebugPrintln("stopped");
    mdnsserverstate = STOPPED;
  }
  else
  {
    Setup_DebugPrintln("Not running");
  }
  delay(10);                      // small pause so background tasks can run
}
#endif // #ifdef MDNSSERVER

// ======================================================================
// WEBSERVER - CHANGE AT YOUR OWN PERIL
// ======================================================================
#if defined(ESP8266)
#undef DEBUG_ESP_HTTP_SERVER
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif // if defined(esp8266)

#include "webserver.h"
#if defined(ESP8266)
#undef DEBUG_ESP_HTTP_SERVER
extern ESP8266WebServer *webserver;
#else
extern WebServer *webserver;
#endif // if defined(esp8266)

// ======================================================================
// OTAUPDATES - CHANGE AT YOUR OWN PERIL
// ======================================================================
#if defined(OTAUPDATES)
#include <ArduinoOTA.h>

void start_otaservice()
{
  Setup_DebugPrint("Start OTA:");
  myoled->oledtextmsg("Start OTA", -1, false, true);
  ArduinoOTA.setHostname(OTAName);                      // Start the OTA service
  ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.onStart([]()
  {
    Setup_DebugPrintln("Started");
  });
  ArduinoOTA.onEnd([]()
  {
    Setup_DebugPrintln("End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
  {
    DebugPrint("Progress:");
    DebugPrintln((progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error)
  {
    Setup_DebugPrint("Err:");
    Setup_DebugPrintln(error);
    if (error == OTA_AUTH_ERROR)
    {
      Setup_DebugPrintln("err Auth");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      Setup_DebugPrintln("err Begin");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      Setup_DebugPrintln("err Connect");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      Setup_DebugPrintln("err Receive");
    }
    else if (error == OTA_END_ERROR)
    {
      Setup_DebugPrintln("err End");
    }
  });
  ArduinoOTA.begin();
  Setup_DebugPrintln("Ready");
  otaupdatestate = RUNNING;
}
#endif // #if defined(OTAUPDATES)

// ======================================================================
// DUCKDNS - CHANGE AT YOUR OWN PERIL
// ======================================================================
#ifdef USEDUCKDNS
#include <EasyDDNS.h>                           // https://github.com/ayushsharma82/EasyDDNS

void init_duckdns(void)
{
  Setup_DebugPrintln("initDuckDNS:");
  myoled->oledtextmsg("Start DuckDNS", -1, false, true);
  EasyDDNS.service("duckdns");                  // Enter your DDNS Service Name - "duckdns" / "noip"
  delay(5);
  EasyDDNS.client(duckdnsdomain, duckdnstoken); // Enter ddns Domain & Token | Example - "esp.duckdns.org","1234567"
  delay(5);
  EasyDDNS.update(DUCKDNS_REFRESHRATE);         // Check for New Ip Every 60 Seconds.
  delay(5);
  duckdnsstate = RUNNING;
}
#endif // #ifdef USEDUCKSDNS

// ======================================================================
// FIRMWARE - CHANGE AT YOUR OWN PERIL
// ======================================================================
byte TimeCheck(unsigned long x, unsigned long Delay)
{
  unsigned long y = x + Delay;
  unsigned long z = millis();                           // pick current time

  if ((x > y) && (x < z))
    return 0;                                           // overflow y
  if ((x < y) && ( x > z))
    return 1;                                           // overflow z

  return (y < z);                                       // no or (z and y) overflow
}

extern void stop_management(void);

void software_Reboot(int Reboot_delay)
{
  myoled->oledtextmsg("Reboot", -1, true, false);
  mySetupData->SaveNow();                       // save the focuser settings immediately

  // a reboot causes everything to reset, so code to stop services etc is not really needed
  delay(Reboot_delay);
  ESP.restart();
}

// move motor without updating position, used by sethomeposition
void steppermotormove(byte ddir )               // direction moving_in, moving_out ^ reverse direction
{
  // Basic assumption rule: If associated pin is -1 then cannot set enable
  if ( mySetupData->get_inoutledstate() == 1)
  {
    ( ddir == moving_in ) ? digitalWrite(mySetupData->get_brdinledpin(), 1) : digitalWrite(mySetupData->get_brdoutledpin(), 1);
  }
  driverboard->movemotor(ddir, false);
  if ( mySetupData->get_inoutledstate() == 1)
  {
    ( ddir == moving_in ) ? digitalWrite(mySetupData->get_brdinledpin(), 0) : digitalWrite(mySetupData->get_brdoutledpin(), 0);
  }
}

bool init_leds()
{
  // Basic assumption rule: If associated pin is -1 then cannot set enable
  Setup_DebugPrintln("initleds");
  if ( mySetupData->get_inoutledstate() == 1)
  {
    pinMode(mySetupData->get_brdinledpin(), OUTPUT);
    pinMode(mySetupData->get_brdoutledpin(), OUTPUT);
    digitalWrite(mySetupData->get_brdinledpin(), 1);
    digitalWrite(mySetupData->get_brdoutledpin(), 1);
    Setup_DebugPrintln("enabled");
    return true;
  }
  else
  {
    Setup_DebugPrintln("disabled");
  }
  return false;
}

long getrssi()
{
  long strength = WiFi.RSSI();
  return strength;
}

bool init_homepositionswitch()
{
  // Basic assumption rule: If associated pin is -1 then cannot set enable
  Setup_DebugPrintln("init_homepositionswitch");
  if ( mySetupData->get_hpswitchenable() == 1)
  {
    pinMode(mySetupData->get_brdhpswpin(), INPUT_PULLUP);
    return true;
  }
  return false;
}

void heapmsg()
{
  HDebugPrint("Heap = ");
  HDebugPrintf("%u\n", ESP.getFreeHeap());
}

#ifdef READWIFICONFIG
bool readwificonfig( char* xSSID, char* xPASSWORD, bool retry )
{
  const String filename = "/wificonfig.json";
  String SSID_1, SSID_2;
  String PASSWORD_1, PASSWORD_2;
  bool   mstatus = false;

  Setup_DebugPrintln("readwificonfig");
  // SPIFFS may have failed to start
  if ( !SPIFFS.begin() )
  {
    TRACE();
    Setup_DebugPrintln("err: read file");
    return mstatus;
  }
  File f = SPIFFS.open(filename, "r");                  // file open to read
  if (!f)
  {
    TRACE();
    Setup_DebugPrintln("err not found");
  }
  else
  {
    String data = f.readString();                       // read content of the text file
    Setup_DebugPrint("Config data: ");
    Setup_DebugPrintln(data);                                 // ... and print on serial
    f.close();

    // DynamicJsonDocument doc( (const size_t) (JSON_OBJECT_SIZE(1) + JSON_ARRAY_SIZE(2) + 120));  // allocate json buffer
    // Using JSON assistant - https://arduinojson.org/v5/assistant/ - we need at least 372 additional bytes for esp32
    // Remember that each of the arrays have UP TO 64 chars each
    DynamicJsonDocument doc( (const size_t) (JSON_OBJECT_SIZE(4) + 372));
    DeserializationError error = deserializeJson(doc, data);    // Parse JSON object
    if (error)
    {
      TRACE();
      Setup_DebugPrintln("err: deserialize");
    }
    else
    {
      // Decode JSON/Extract values
      SSID_1     =  doc["mySSID"].as<char*>();
      PASSWORD_1 =  doc["myPASSWORD"].as<char*>();
      SSID_2     =  doc["mySSID_1"].as<char*>();
      PASSWORD_2 =  doc["myPASSWORD_1"].as<char*>();

      Setup_DebugPrint("SSID:");
      Setup_DebugPrintln(SSID_1);
      Setup_DebugPrint("PASSWORD:");
      Setup_DebugPrintln(PASSWORD_1);
      Setup_DebugPrint("SSID_1:");
      Setup_DebugPrintln(SSID_2);
      Setup_DebugPrint("PASSWORD_1:");
      Setup_DebugPrintln(PASSWORD_2);

      if ( retry == false )
      {
        // get first pair
        SSID_1.toCharArray(xSSID, SSID_1.length() + 1);
        PASSWORD_1.toCharArray(xPASSWORD, PASSWORD_1.length() + 1);
        mstatus = true;
      }
      else
      {
        // get second pair
        SSID_2.toCharArray(xSSID, SSID_2.length() + 1);
        PASSWORD_2.toCharArray(xPASSWORD, PASSWORD_2.length() + 1);
        mstatus = true;
      }
    }
  }
  return mstatus;
}
#endif

#if defined(ACCESSPOINT) || defined(STATIONMODE)
void start_tcpipserver()
{
  Setup_DebugPrintln("start tcpipserver");
#if defined(ESP8266)
  myserver.begin();                       // esp8266 cannot define a port when starting
#else
  myserver.begin(mySetupData->get_tcpipport());
#endif
  tcpipserverstate = RUNNING;
}

void stop_tcpipserver()
{
  Setup_DebugPrintln("stop_tcipserver");
  myserver.stop();
  tcpipserverstate = STOPPED;
}
#endif

//_______________________________________________ setup()

void setup()
{
  Serial.begin(SERIALPORTSPEED);
#if defined(DEBUG)
  //  Serial.begin(SERIALPORTSPEED);
  DebugPrintln("Serialbegin");
  DebugPrintln("Debug on");
#endif

#ifdef LOCALSERIAL
  Serial.begin(SERIALPORTSPEED);
  serialline = "";
  clearSerialPort();
#endif // if defined(LOCALSERIAL)

  delay(100);                                   // go on after statement does appear

#ifdef TIMESETUP
  Serial.print("setup(): ");
  Serial.println(millis());
#endif

  heapmsg();
  Setup_DebugPrintln("setup(): mySetupData()");
  mySetupData = new SetupData();                // instantiate object SetUpData with SPIFFS file
  heapmsg();

#ifdef BLUETOOTHMODE                            // open Bluetooth port, set bluetooth device name
  Setup_DebugPrintln("Start Bluetooth");
  SerialBT.begin(BLUETOOTHNAME);                // Bluetooth device name
  btline = "";
  clearbtPort();
#endif

  reboot = true;                                // booting up

  // Setup LEDS, use as controller power up indicator
  // Basic assumption rule: If associated pin is -1 then cannot set enable
  if ( mySetupData->get_inoutledstate() == 1)
  {
    Setup_DebugPrintln("IN OUT LEDS");
    bool result = init_leds();
    if ( result == true )
    {
      Setup_DebugPrintln("enabled");
    }
    else
    {
      Setup_DebugPrintln("disabled");
    }
  }
  else
  {
    Setup_DebugPrintln("disabled");
  }

  // Setup Pushbuttons, active high when pressed
  // Basic assumption rule: If associated pin is -1 then cannot set enable
  if ( mySetupData->get_pbenable() == 1)
  {
    bool result = init_pushbuttons();
    Setup_DebugPrintln("Push Buttons");
    if ( result == true )
    {
      Setup_DebugPrintln("enabled");
    }
    else
    {
      Setup_DebugPrintln("disabled");
    }
  }
  else
  {
    Setup_DebugPrintln("disabled");
  }

  heapmsg();

  displayfound = false;
#ifdef OLED_MODE
  if (CheckOledConnected())
  {
    Setup_DebugPrintln("init OLED_MODE");
    myoled = new OLED_MODE;                       // Start configured OLED display object
    displaystate = true;
  }
  else
  {
    Setup_DebugPrintln("init OLED_NON");
    myoled = new OLED_NON;
    displaystate = false;
  }
#else
  Setup_DebugPrintln("init OLED_NON");
  myoled = new OLED_NON;
  displaystate = false;
#endif // #ifdef OLED_MODE
  Setup_DebugPrintln("Display state:");
  Setup_DebugPrintln(displaystate);

  heapmsg();

  DebugPrint("fposition=");                 // Print Loaded Values from SPIFF
  DebugPrintln(mySetupData->get_fposition());
  DebugPrint("focuserdirection=");
  DebugPrintln(mySetupData->get_focuserdirection());
  DebugPrint("maxstep=");
  DebugPrintln(mySetupData->get_maxstep());
  DebugPrint("stepsize= ");
  DebugPrintln(mySetupData->get_stepsize());
  DebugPrint("DelayAfterMove=");
  DebugPrintln(mySetupData->get_DelayAfterMove());
  DebugPrint("backlashsteps_in=");
  DebugPrintln(mySetupData->get_backlashsteps_in());
  DebugPrint("backlashsteps_out=");
  DebugPrintln(mySetupData->get_backlashsteps_out());
  DebugPrint("tempcoefficient=");
  DebugPrintln(mySetupData->get_tempcoefficient());
  DebugPrint("get_tempresolution=");
  DebugPrintln(mySetupData->get_tempresolution());
  DebugPrint("coilpower=");
  DebugPrintln(mySetupData->get_coilpower());
  DebugPrint("reversedirection=");
  DebugPrintln(mySetupData->get_reversedirection());
  DebugPrint("stepsizeenabled=");
  DebugPrintln(mySetupData->get_stepsizeenabled());
  DebugPrint("tempmode=");
  DebugPrintln(mySetupData->get_tempmode());
  DebugPrint("lcdupdateonmove=");
  DebugPrintln(mySetupData->get_lcdupdateonmove());
  DebugPrint("lcdpagedisplaytime=");
  DebugPrintln(mySetupData->get_lcdpagetime());
  DebugPrint("tempcompenabled=");
  DebugPrintln(mySetupData->get_tempcompenabled());
  DebugPrint("tcdirection=");
  DebugPrintln(mySetupData->get_tcdirection());
  DebugPrint("motorspeed=");
  DebugPrintln(mySetupData->get_motorspeed());
  DebugPrint("displayenabled=");
  DebugPrintln(mySetupData->get_displayenabled());
  DebugPrint("webserverport=");
  DebugPrintln(mySetupData->get_webserverport());
  DebugPrint("ascomalpacaport=");
  DebugPrintln(mySetupData->get_ascomalpacaport());
  DebugPrint("webpagerefreshrate=");
  DebugPrintln(mySetupData->get_webpagerefreshrate());
  DebugPrint("mdnsport=");
  DebugPrintln(mySetupData->get_mdnsport());
  DebugPrint("tcpipport=");
  DebugPrintln(mySetupData->get_tcpipport());
  DebugPrint("showstartscreen=");
  DebugPrintln(mySetupData->get_showstartscreen());
  DebugPrint("wp_backcolor=");
  DebugPrintln(mySetupData->get_wp_backcolor());
  DebugPrint("wp_textcolor=");
  DebugPrintln(mySetupData->get_wp_textcolor());
  DebugPrint("wp_headercolor=");
  DebugPrintln(mySetupData->get_wp_headercolor());
  DebugPrint("wp_titlecolor=");
  DebugPrintln(mySetupData->get_wp_titlecolor());
  DebugPrint("ascomserverstate=");
  DebugPrintln(mySetupData->get_ascomserverstate());
  DebugPrint("webserverstate=");
  DebugPrintln(mySetupData->get_webserverstate());
  DebugPrint("temperatureprobestate=");
  DebugPrintln(mySetupData->get_temperatureprobestate());
  DebugPrint("inoutledstate=");
  DebugPrintln(mySetupData->get_inoutledstate());

  tprobe1 = 0;
  lasttemp = 20.0;
  // Basic assumption rule: If associated pin is -1 then cannot set enable
  Setup_DebugPrint("Temp probe:");
  if ( mySetupData->get_temperatureprobestate() == 1)   // if temperature probe enabled then try to start new probe
  {
    Setup_DebugPrintln("enabled");
    myTempProbe = new TempProbe;                        // create temp probe - should set tprobe1=true if probe found
  }
  else
  {
    tprobe1 = 0;
    Setup_DebugPrintln("disabled");
  }

  // set packet counts to 0
  packetsreceived = 0;
  packetssent = 0;
  rssi = -100;

#if defined(READWIFICONFIG)
#if defined(ACCESSPOINT) || defined(STATIONMODE)
  Setup_DebugPrintln("Call readwificonfig");
  readwificonfig(mySSID, myPASSWORD, false);                // read mySSID,myPASSWORD from FS if exist, otherwise use defaults
#endif
#endif

  heapmsg();

#ifdef ACCESSPOINT
  myoled->oledtextmsg("Start AP", -1, true, true);
  Setup_DebugPrintln("Start AP");
  WiFi.config(ip, dns, gateway, subnet);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(mySSID, myPASSWORD);
#endif // end ACCESSPOINT

  heapmsg();

  // this is setup as a station connecting to an existing wifi network
#ifdef STATIONMODE
  Setup_DebugPrintln("Start Stationmode");
  myoled->oledtextmsg("Start Stationmode", -1, false, true);

  // Log on to LAN
  WiFi.mode(WIFI_STA);
  if (staticip == STATICIPON)                   // if staticip then set this up before starting
  {
    Setup_DebugPrintln("Set Static IP");
    myoled->oledtextmsg("Set Static IP", -1, false, true);
    WiFi.config(ip, dns, gateway, subnet);
    delay(5);
  }

  // attempt to connect using mySSID and myPASSWORD
  WiFi.begin(mySSID, myPASSWORD); // attempt to start the WiFi
  //DebugPrint("Status: ");
  //DebugPrintln(String(connstatus));
  delay(1000);                                      // wait 1s
  for (int attempts = 0; WiFi.status() != WL_CONNECTED; attempts++)
  {
    DebugPrint("Connect: ");
    DebugPrintln(mySSID);
    DebugPrint("Try=");
    DebugPrint(attempts);
    delay(1000);                                    // wait 1s

    myoled->oledtextmsg("Try", attempts, false, true);
    if (attempts > 9)                               // if this attempt is 10 or more tries
    {
      DebugPrintln("Reboot");
      myoled->oledtextmsg("Reboot" + String(mySSID), -1, true, true);
      delay(2000);
      break;                                        // jump out of this for loop
    }
  }

  // check if connected after using first set of credentials - if not connected try second pair of credentials
  if ( WiFi.status() != WL_CONNECTED )
  {
#ifdef READWIFICONFIG
    // try alternative credentials, mySSID_1, myPASSWORD_1 in the wificonfig.json file
    Setup_DebugPrintln("Call readwificonfig");
    readwificonfig(mySSID, myPASSWORD, true);
#else
    // there was no wificonfig.json file specified
    // so we will try again with 2nd pair of credentials
    // and reboot after 10 attempts to log on
    memset( mySSID, 0, 64);
    memset( myPASSWORD, 0, 64);
    memcpy( mySSID, mySSID_1, (sizeof(mySSID_1) / sizeof(mySSID_1[0]) ));
    memcpy( myPASSWORD, myPASSWORD_1, (sizeof(myPASSWORD_1) / sizeof(myPASSWORD_1[0])) );
#endif
    WiFi.begin(mySSID, myPASSWORD);  // attempt to start the WiFi
    //DebugPrint("Status: ");
    //DebugPrintln(String(connstatus));
    delay(1000);                                  // wait 1s
    for (int attempts = 0; WiFi.status() != WL_CONNECTED; attempts++)
    {
      DebugPrint("Connect: ");
      DebugPrintln(mySSID);
      DebugPrint("Try=");
      DebugPrint(attempts);
      delay(1000);                                // wait 1s

      myoled->oledtextmsg("Try", attempts, false, true);
      if (attempts > 9)                           // if this attempt is 10 or more tries
      {
        DebugPrintln("Reboot");
        myoled->oledtextmsg("Reboot" + String(mySSID), -1, true, true);
        delay(2000);
        software_Reboot(2000);                    // GPIO0 must be HIGH and GPIO15 LOW when calling ESP.restart();
      }
    }
  }
#endif // end STATIONMODE

#if !defined(ESP8266)
  DebugPrint("WifiHostname: ");
  DebugPrintln(WiFi.getHostname());             // esp8266 has no getHostname()
#endif

  myoled->oledtextmsg("Connected", -1, true, true);
  delay(10);                                    // keep delays small else issue with ASCOM

  tcpipserverstate = STOPPED;
  mdnsserverstate = STOPPED;
  webserverstate = STOPPED;
  ascomserverstate = STOPPED;
  ascomdiscoverystate = STOPPED;
  managementserverstate = STOPPED;
  otaupdatestate = STOPPED;
  duckdnsstate = STOPPED;

  heapmsg();

#if defined(ACCESSPOINT) || defined(STATIONMODE)
  rssi = getrssi();                             // get network strength
  // Starting TCP Server
  myoled->oledtextmsg("Start tcp/ip server", -1, false, true);
  Setup_DebugPrintln("Start tcp/ip server");
  start_tcpipserver();
  delay(10);                                    // keep delays small else issue with ASCOM
  Setup_DebugPrintln("TCP/IP started");
  myoled->oledtextmsg("TCP/IP started", -1, false, true);

  heapmsg();

  // connection established
  Setup_DebugPrint("SSID:");
  Setup_DebugPrintln(mySSID);
  Setup_DebugPrintln("Ready");
  Setup_DebugPrintln("Get IP");
  ESP32IPAddress = WiFi.localIP();
  snprintf(ipStr, sizeof(ipStr), "%i.%i.%i.%i",  ESP32IPAddress[0], ESP32IPAddress[1], ESP32IPAddress[2], ESP32IPAddress[3]);
  Setup_DebugPrint("IP:");
  Setup_DebugPrintln(ipStr);
  Setup_DebugPrint("Port:");
  Setup_DebugPrintln(SERVERPORT);
#endif // if defined(ACCESSPOINT) || defined(STATIONMODE)

  // assign to current working values
  //ftargetPosition = fcurrentPosition = mySetupData->get_fposition();
  ftargetPosition = mySetupData->get_fposition();
  
  Setup_DebugPrint("Start drvbrd:");
  Setup_DebugPrintln(DRVBRD);
  myoled->oledtextmsg("Start drvbrd:", DRVBRD, true, true);

  heapmsg();

  // DebugPrintln("setup(): driverboard");
  // ensure targetposition will be same as focuser position
  // otherwise after loading driverboard focuser will start moving immediately
  Setup_DebugPrintln("driver board: start");
  ftargetPosition = mySetupData->get_fposition();
  driverboard = new DriverBoard(mySetupData->get_fposition() );
  Setup_DebugPrintln("driver board: end");

  delay(5);
  heapmsg();

  // range check focuser variables
  mySetupData->set_brdstepmode((mySetupData->get_brdstepmode() < 1 ) ? 1 : mySetupData->get_brdstepmode());
  mySetupData->set_coilpower((mySetupData->get_coilpower() >= 1) ?  1 : 0);
  mySetupData->set_reversedirection((mySetupData->get_reversedirection() >= 1) ?  1 : 0);
  mySetupData->set_lcdpagetime((mySetupData->get_lcdpagetime() < LCDPAGETIMEMIN) ? mySetupData->get_lcdpagetime() : LCDPAGETIMEMIN);
  mySetupData->set_lcdpagetime((mySetupData->get_lcdpagetime() > LCDPAGETIMEMAX) ? LCDPAGETIMEMAX : mySetupData->get_lcdpagetime());
  mySetupData->set_maxstep((mySetupData->get_maxstep() < FOCUSERLOWERLIMIT) ? FOCUSERLOWERLIMIT : mySetupData->get_maxstep());
  mySetupData->set_stepsize((float)(mySetupData->get_stepsize() < 0.0 ) ? 0 : mySetupData->get_stepsize());
  mySetupData->set_stepsize((float)(mySetupData->get_stepsize() > MAXIMUMSTEPSIZE ) ? MAXIMUMSTEPSIZE : mySetupData->get_stepsize());

  // ensure driverboard position is same as setupData
  // set focuser position in DriverBoard
  driverboard->setposition(mySetupData->get_fposition());

  // set coilpower
  Setup_DebugPrintln("Check CP");
  if (mySetupData->get_coilpower() == 0)
  {
    driverboard->releasemotor();
    Setup_DebugPrintln("CP off");
  }

  delay(5);

  // setup home position switch input pin
  // Basic assumption rule: If associated pin is -1 then cannot set enable
  Setup_DebugPrint("hpsw:");
  if ( mySetupData->get_hpswitchenable() == 1)
  {
    DebugPrintln("enabled");
    bool result = init_homepositionswitch();
    if ( result == true )
    {
      Setup_DebugPrintln("enabled");
    }
    else
    {
      Setup_DebugPrintln("disabled");
    }
  }
  else
  {
    Setup_DebugPrintln("disabled");
  }

  // Setup infra red remote
#ifdef INFRAREDREMOTE
  // Basic assumption rule: If associated pin is -1 then cannot set enable
  if ( mySetupData->get_irremoteenable() == 1)
  {
    Setup_DebugPrintln("ir-remote enabled");
    init_irremote();
  }
  else
  {
    Setup_DebugPrintln("ir-remote disabled");
  }
#endif

  // setup joystick
#ifdef JOYSTICK1
  init_joystick1();
#endif

#ifdef JOYSTICK2
  init_joystick2();
#endif

  isMoving = 0;

  // Basic assumption rule: If associated pin is -1 then cannot set enable
  if ( mySetupData->get_temperatureprobestate() == 1)     // if temp probe "enabled" state
  {
    if ( tprobe1 != 0 )                                   // if a probe was found
    {
      Setup_DebugPrintln("tprobe1 != 0. read_temp");
      myTempProbe->read_temp(1);                          // read the temperature
    }
    else
    {
      Setup_DebugPrintln("tprobe1 is 0");
      // disable temperature probe
      mySetupData->set_temperatureprobestate(0);
    }
  }

#ifdef OTAUPDATES
  Setup_DebugPrintln("Start otaservice");
  start_otaservice();                       // Start the OTA service
#endif // if defined(OTAUPDATES)

  heapmsg();
  HDebugPrintln("setup(): management server");

#ifdef MANAGEMENT
  Setup_DebugPrintln("setup(): management server");
  start_management();
#endif

  heapmsg();

  if ( mySetupData->get_webserverstate() == 1)
  {
    Setup_DebugPrintln("start web server");
    start_webserver();
  }

  if ( mySetupData->get_ascomserverstate() == 1)
  {
      Setup_DebugPrintln("start ascom server");
      start_ascomremoteserver();
  }

#ifdef MDNSSERVER
  Setup_DebugPrintln("start mdns server");
  start_mdns_service();
#endif

  // setup duckdns
#ifdef USEDUCKDNS
  Setup_DebugPrintln("start duckdns");
  init_duckdns();
#endif

  Setup_DebugPrint("Position:");
  Setup_DebugPrintln(driverboard->getposition());
  Setup_DebugPrint("Target Position");
  Setup_DebugPrintln(ftargetPosition);
  Setup_DebugPrintln("Setup done");
  myoled->oledtextmsg("Setup done", -1, false, true);

  // Basic assumption rule: If associated pin is -1 then cannot set enable
  if ( mySetupData->get_inoutledstate() == 1)
  {
    digitalWrite(mySetupData->get_brdinledpin(), 0);
    digitalWrite(mySetupData->get_brdoutledpin(), 0);
  }

  halt_alert = false;
  reboot = false;                                           // we have finished the reboot now

#ifdef TIMESETUP
  Serial.print("setup(): ");
  Serial.println(millis());
#endif
}

//_____________________ loop()___________________________________________

extern volatile uint32_t stepcount;     // number of steps to go in timer interrupt service routine
extern volatile bool     timerSemaphore;

void loop()
{
  static StateMachineStates MainStateMachine = State_Idle;
  static uint32_t backlash_count = 0;
  static bool     DirOfTravel = (bool) mySetupData->get_focuserdirection();
  static uint32_t TimeStampDelayAfterMove = 0;
  static uint32_t TimeStampPark = millis();
  static bool     Parked = true;                  // focuser cannot be moving as it was just started
  static uint8_t  updatecount = 0;
  static uint32_t steps = 0;

  static connection_status ConnectionStatus = disconnected;
  static oled_state oled = oled_on;

  int stepstaken = 0;
  bool hpswstate = false;

#ifdef TIMELOOP
  Serial.print("loop(): ");
  Serial.println(millis());
#endif

#if defined(ACCESSPOINT) || defined(STATIONMODE)
  if (ConnectionStatus == disconnected)
  {
    myclient = myserver.available();
    if (myclient)
    {
      if (myclient.connected())
      {
        DebugPrintln("tcp client connected");
        ConnectionStatus = connected;
      }
    }
  }
  else
  {
    // is data available from the client request
    if (myclient.connected())
    {
      if (myclient.available())
      {
        ESP_Communication(); // Wifi communication
      }
    }
    else
    {
      DebugPrintln("tcp client disconnectd");
      myclient.stop();
      ConnectionStatus = disconnected;
      oled = oled_on;
    }
  }
#endif // defined(ACCESSPOINT) || defined(STATIONMODE)

#ifdef BLUETOOTHMODE
  if ( SerialBT.available() )
  {
    processbt();
  }
  // if there is a command from Bluetooth
  if ( queue.count() >= 1 )                 // check for serial command
  {
    ESP_Communication();
  }
#endif // ifdef Bluetoothmode

#ifdef LOCALSERIAL
  // if there is a command from Serial port
  if ( Serial.available() )
  {
    processserial();
  }
  if ( queue.count() >= 1 )                 // check for serial command
  {
    ESP_Communication();
  }
#endif // ifdef LOCALSERIAL

#ifdef OTAUPDATES
  if ( otaupdatestate == RUNNING )
  {
    ArduinoOTA.handle();                      // listen for OTA events
  }
#endif // ifdef OTAUPDATES

  if ( ascomserverstate == RUNNING)
  {
    ascomserver->handleClient();
    checkASCOMALPACADiscovery();
  }

  if ( webserverstate == RUNNING )
  {
    webserver->handleClient();
  }

#ifdef MANAGEMENT
  if ( managementserverstate == RUNNING )
  {
    mserver.handleClient();
  }
#endif

  //_____________________________MainMachine _____________________________

  switch (MainStateMachine)
  {
    case State_Idle:
      if (driverboard->getposition() != ftargetPosition)
      {
        isMoving = 1;
        driverboard->enablemotor();
        MainStateMachine = State_InitMove;
        DebugPrint("go init_move");
        DebugPrint("Position:");
        DebugPrintln(driverboard->getposition());
        DebugPrint("Target:");
        DebugPrintln(ftargetPosition);
      }
      else
      {
        // focuser stationary. isMoving is 0
        if (mySetupData->SaveConfiguration(driverboard->getposition(), DirOfTravel)) // save config if needed
        {
          oled = oled_off;
          DebugPrint("Save config");
        }

        update_pushbuttons();

#ifdef JOYSTICK1
        update_joystick1();
#endif
#ifdef JOYSTICK2
        update_joystick2();
#endif
#ifdef INFRAREDREMOTE
        update_irremote();
#endif
        if (mySetupData->get_displayenabled() == 1)
        {
          myoled->update_oledtextdisplay();
        }
        else
        {
          oled = oled_off;
        }
        myoled->Update_Oled(oled, ConnectionStatus);

        if ( mySetupData->get_temperatureprobestate() == 1)           // if probe is enabled
        {
          if ( tprobe1 != 0 )                                         // if probe was found
          {
            DebugPrintln("tprobe1 = 1");
            DebugPrintln("update_temp()");
            myTempProbe->update_temp();
          }
          else
          {
            DebugPrintln("tprobe1 = 0");
          }
        }

        // Parked is set false after State_DelayAfterMove is ended
        if (Parked == false)
        {
          if (TimeCheck(TimeStampPark, MotorReleaseDelay))   // Power off after MotorReleaseDelay
          {
            // need to obey rule - can only release motor if coil power is disabled
            if ( mySetupData->get_coilpower() == 0 )
            {
              driverboard->releasemotor();
              DebugPrintln("CP=off");
            }
            Parked = true;
          } // if (TimeCheck(TimeStampPark, MotorReleaseDelay))
        } // if (Parked == false)
      } // if (driverboard->getposition() != ftargetPosition)
      break;

    case State_InitMove:
      isMoving = 1;
      backlash_count = 0;
      DirOfTravel = (ftargetPosition > driverboard->getposition()) ? moving_out : moving_in;
      driverboard->enablemotor();
      if (mySetupData->get_focuserdirection() != DirOfTravel)
      {
        mySetupData->set_focuserdirection(DirOfTravel);
        // move is in opposite direction, check for backlash enabled
        // get backlash settings
        if ( DirOfTravel == moving_in)
        {
          if (mySetupData->get_backlash_in_enabled())
          {
            backlash_count = mySetupData->get_backlashsteps_in();
          }
        }
        else
        {
          if (mySetupData->get_backlash_out_enabled())
          {
            backlash_count = mySetupData->get_backlashsteps_out();
          }
        } // if ( DirOfTravel == moving_in)
        /*
                if (DirOfTravel != moving_main && backlash_count)
                {
                  uint32_t sm = mySetupData->get_stepmode();
                  uint32_t bl = backlash_count * sm;
                  DebugPrint("bl: ");
                  DebugPrint(bl);
                  DebugPrint(" ");

                  if (DirOfTravel == moving_out)
                  {
                    backlash_count = bl + sm - ((ftargetPosition + bl) % sm); // Trip to tuning point should be a fullstep position
                  }
                  else
                  {
                    backlash_count = bl + sm + ((ftargetPosition - bl) % sm); // Trip to tuning point should be a fullstep position
                  }

                  DebugPrint("backlash_count: ");
                  DebugPrint(backlash_count);
                  DebugPrint(" ");
                } // if (DirOfTravel != moving_main && backlash_count)
                else
                {
                   DebugPrintln("false");
                }
        */
      } // if (mySetupData->get_focuserdirection() != DirOfTravel)

      // if target pos > current pos then steps = target pos - current pos
      // if target pos < current pos then steps = current pos - target pos
      steps = (ftargetPosition > driverboard->getposition()) ? ftargetPosition - driverboard->getposition() : driverboard->getposition() - ftargetPosition;

      // Error - cannot combine backlash steps to steps because that alters position
      // Backlash move SHOULD NOT alter focuser position as focuser is not actually moving
      // backlash is taking up the slack in the stepper motor/focuser mechanism, so position is not actually changing
      if ( backlash_count != 0 )
      {
        DebugPrintln("go backlash");
        MainStateMachine = State_Backlash;
      }
      else
      {
        // if target pos > current pos then steps = target pos - current pos
        // if target pos < current pos then steps = current pos - target pos
        driverboard->initmove(DirOfTravel, steps);
        DebugPrint("Steps: ");
        DebugPrintln(steps);
        DebugPrintln("go moving");
        MainStateMachine = State_Moving;
      }
      break;

    case State_Backlash:
      // apply backlash
      DebugPrint("Apply Backlash: Steps=");
      DebugPrintln(backlash_count);
      while ( backlash_count != 0 )
      {
        steppermotormove(DirOfTravel);                            // take 1 step and do not adjust position
        delayMicroseconds(mySetupData->get_brdmsdelay());          // ensure delay between steps
        backlash_count--;
        if (HPS_alert() )                                         // check if home position sensor activated?
        {
          DebugPrintln("HPS_alert() during backlash move");
          timerSemaphore = false;                                 // move finished
          backlash_count = 0;                                     // drop out of while loop
          MainStateMachine = State_Moving;                        // change state to State_Moving and handle HPSW
        }
      }
      if ( MainStateMachine == State_Backlash )                   // finished backlash move, so now move motor #steps
      {
        DebugPrintln("Backlash done");
        DebugPrint("Initiate motor move- steps: ");
        DebugPrintln(steps);
        driverboard->initmove(DirOfTravel, steps);
        DebugPrintln("go moving");
        MainStateMachine = State_Moving;
      }
      else
      {
        // MainStateMachine is State_Moving - timerSemaphore is false. is then caught by if(HPS_alert() ) and HPSW is processed
      }
      break;

    //_______________________________State_Moving

    case State_Moving:
      //DebugPrintln("S_M");
      if ( timerSemaphore == true )
      {
        // move has completed, the driverboard keeps track of focuser position
        DebugPrintln("Move completed");
        DebugPrintln("go DelayAfterMove");
        MainStateMachine = State_DelayAfterMove;
      }
      else
      {
        // timer semaphore is false. still moving, we need to check for halt and hpsw closure
        if ( halt_alert )
        {
          DebugPrintln("halt_alert");
          halt_alert = false;                             // reset alert flag
          ftargetPosition = driverboard->getposition();
          mySetupData->set_fposition(driverboard->getposition());
          driverboard->halt();                            // disable interrupt timer that moves motor
          // we no longer need to keep track of steps here or halt because driverboard updates position on every move
          DebugPrintln("go DelayAfterMove");
          MainStateMachine = State_DelayAfterMove;
        } // if ( halt_alert )
        if (HPS_alert() )                                 // check if home position sensor activated?
        {
          if (driverboard->getposition() > 0)
          {
            DebugPrintln("HP Sw=1, Pos not 0");
          }
          else
          {
            DebugPrintln("HP Sw=1, Pos=0");
          } // if (driverboard->getposition() > 0)
          ftargetPosition = 0;
          driverboard->setposition(0);
          mySetupData->set_fposition(0);

          if ( mySetupData->get_showhpswmsg() == 1)     // check if display home position messages is enabled
          {
            if (mySetupData->get_displayenabled() == 1)
            {
              myoled->oledtextmsg("HP Sw=1, Pos=0", -1, true, true);
            }
          }
          // we should jump to
          DebugPrintln("go SetHomePosition");
          MainStateMachine = State_SetHomePosition;
        } // if (HPS_alert() )

        // if the update position on display when moving is enabled, then update the display
        if ( mySetupData->get_lcdupdateonmove() == 1)
        {
          // update position counter on display if there is an enabled display
          if (mySetupData->get_displayenabled() == 1)
          {
            updatecount++;
            if ( updatecount > LCDUPDATEONMOVE )        // only update every 15th move to avoid overhead
            {
              updatecount = 0;
              myoled->update_oledtext_position();
            }
          } // if (mySetupData->get_displayenabled() == 1)
        } // if ( get_lcdupdateonmove() == 1)
      }
      break;

    case State_SetHomePosition:                         // move out till home position switch opens
      DebugPrintln("State_SetHomePosition");
      if ( mySetupData->get_hpswitchenable() == 1)
      {
        // check if display home position switch messages is enabled
        if ( mySetupData->get_showhpswmsg() == 1)
        {
          if (mySetupData->get_displayenabled() == 1)
          {
            myoled->oledtextmsg("HP Sw=0, Mov out", -1, false, true);
          }
        }
        // HOME POSITION SWITCH IS CLOSED - Step out till switch opens then set position = 0
        stepstaken = 0;                                   // Count number of steps to prevent going too far
        DebugPrintln("HP Sw=0, Mov out");
        DirOfTravel = !DirOfTravel;                       // We were going in, now we need to reverse and go out
        hpswstate = HPSWCLOSED;                           // We know we got here because switch was closed
        while ( hpswstate == HPSWCLOSED )                 // while hpsw = closed = true = 1
        {
          if ( mySetupData->get_reversedirection() == 0 )
          {
            steppermotormove(DirOfTravel);                // take 1 step
          }
          else
          {
            steppermotormove(!DirOfTravel);
          }

          delayMicroseconds(mySetupData->get_brdmsdelay()); // Ensure delay between steps

          stepstaken++;                                   // increment steps taken
          if ( stepstaken > HOMESTEPS )                   // this prevents the endless loop if the hpsw is not connected or is faulty
          {
            DebugPrintln("HP Sw=0, Mov out err");
            hpswstate = HPSWOPEN;
          }
          else
          {
            //hpswstate = !(digitalRead(HPSWPIN));        // read state of HPSW
            hpswstate = HPS_alert();                      // hps_alert returns true if closed, false = open
          }
        }
        DebugPrint("HP Sw, Mov out steps:");
        DebugPrintln(stepstaken);
        DebugPrintln("HP Sw=0, Mov out ok");
        ftargetPosition = 0;
        driverboard->setposition(0);
        mySetupData->set_fposition(0);

        mySetupData->set_focuserdirection(DirOfTravel);   // set direction of last move

        if ( mySetupData->get_showhpswmsg() == 1)         // check if display home position switch messages is enabled
        {
          if (mySetupData->get_displayenabled() == 1)
          {
            myoled->oledtextmsg("HP Sw=0, Mov out ok", -1, true, true);
          }
        }
      } //  if( mySetupData->get_homepositionswitch() == 1)
      MainStateMachine = State_DelayAfterMove;
      TimeStampDelayAfterMove = millis();
      DebugPrintln("go DelayAfterMove");
      break;

    //_______________________________State_DelayAfterMove

    case State_DelayAfterMove:
      heapmsg();
      // apply Delayaftermove, this MUST be done here in order to get accurate timing for DelayAfterMove
      if (TimeCheck(TimeStampDelayAfterMove , mySetupData->get_DelayAfterMove()))
      {
        oled = oled_on;
        isMoving = 0;
        TimeStampPark  = millis();                      // catch current time
        Parked = false;                                 // mark to park the motor in State_Idle
        DebugPrintln("go idle");
        MainStateMachine = State_Idle;
      }
      break;

    default:
      DebugPrintln("Err: wrong State");
      DebugPrintln("go idle");
      MainStateMachine = State_Idle;
      break;
  }

#ifdef TIMELOOP
  Serial.print("loop(): ");
  Serial.println(millis());
#endif
} // end Loop()
