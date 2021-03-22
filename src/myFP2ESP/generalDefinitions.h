// ======================================================================
// generalDefinitions.h : myFP2ESP GENERAL DEFINITIONS
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

#ifndef generalDefinitions_h
#define generalDefinitions_h

#include <Arduino.h>

// ======================================================================
// 1: GENERAL DEFINES -- DO NOT CHANGE
// ======================================================================

enum oled_state { oled_off, oled_on };
enum connection_status { disconnected, connected };
//  StateMachine definition
enum StateMachineStates { State_Idle, State_InitMove, State_Backlash, State_Moving, State_DelayAfterMove, State_FinishedMove, State_SetHomePosition };

#if defined(ESP8266)                        // this "define(ESP8266)" comes from Arduino IDE
#include <LittleFS.h>
#define SPIFFS LittleFS
#else                                       // otherwise assume ESP32
#include "SPIFFS.h"
#endif

#define DEFAULTPOSITION       5000L
#define DEFAULTMAXSTEPS       80000L

#define DEFAULTSAVETIME       30000         // default time to wait before saving data to FS

#define ALPACAPORT            4040          // ASCOM Remote port
#define WEBSERVERPORT         80            // Web server port
#define MSSERVERPORT          6060          // Management interface - cannot be changed
#define MDNSSERVERPORT        7070          // mDNS service
#define WS_REFRESHRATE        60            // web server page refresh time 60s
#define MINREFRESHPAGERATE    10            // 10s - too low and the overhead becomes too much for the controller
#define MAXREFRESHPAGERATE    900           // 15m
#define DUCKDNS_REFRESHRATE   60000         // duck dns, check ip address every 60s for an update
#define RUNNING               true          // service state running
#define STOPPED               false         // service state stopped
#define MSREBOOTPAGEDELAY     20000         // management service reboot page, time (s) between next page refresh
#define REBOOTDELAY           2000          // When rebooting controller, delay (2s) from msg to actual reboot
#define MotorReleaseDelay     120*1000      // motor release power after 120s

#define MINOLEDPAGETIME       2000L
#define MAXOLEDPAGETIME       10000L
#define OLEDPGOPTIONALL       "111"         // oled page enable, ALL pages

#define MOTORPULSETIME        2             // DO NOT CHANGE
#define SERVERPORT            2020          // TCPIP port for myFP2ESP
#define TEMPREFRESHRATE       3000L         // refresh rate between temperature conversions unless an update is requested via serial command
#define SERIALPORTSPEED       115200        // 9600, 14400, 19200, 28800, 38400, 57600, 115200
#define ESPDATA               0             // command has come from tcp/ip
#define BTDATA                1             // command has come from bluetooth
#define SERIALDATA            2             // command has come from serial port
#define QUEUELENGTH           20            // number of commands that can be saved in the serial queue

#define DEFAULTSTEPSIZE       50.0          // This is the default setting for the step size in microns
#define MINIMUMSTEPSIZE       0.0
#define MAXIMUMSTEPSIZE       100.0
#define TEMPRESOLUTION        10            // Set the default DS18B20 precision to 0.25 of a degree 9=0.5, 10=0.25, 11=0.125, 12=0.0625
#define LCDUPDATEONMOVE       15            // defines how many steps before refreshing position when moving if lcdupdateonmove is 1
#define FOCUSERUPPERLIMIT     2000000000L   // arbitary focuser limit up to 2000000000
#define FOCUSERLOWERLIMIT     1024L         // lowest value that maxsteps can be
#define LCDPAGETIMEMIN        2             // 2s minimum lcd page display time
#define LCDPAGETIMEMAX        10            // 10s maximum lcd page display time
#define HOMESTEPS             200           // Prevent searching for home position switch never returning, this should be > than # of steps between closed and open
#define HPSWOPEN              0             // hpsw states refelect status of switch
#define HPSWCLOSED            1

#define MAXWEBPAGESIZE        4100          // largest = / = 3943
#define MAXASCOMPAGESIZE      2200          // largest = /setuppage = 2042
#define MAXMANAGEMENTPAGESIZE 3700          // largest = /msindex2 = 3568
#define MAXCUSTOMBRDJSONSIZE   300

// ======================================================================
// 2: DO NOT CHANGE
// ======================================================================

#define moving_in             false
#define moving_out            !moving_in
#define moving_main           moving_in               

#define EOFSTR                '#'
#define STARTCMDSTR           ':'

extern const char* programVersion;
extern const char* ProgramAuthor;

// web page color messages
extern const char* BACKCOLORINVALIDSTR;
extern const char* NEWTITLECOLORSTR;
extern const char* TITLECOLORINVALIDSTR;
extern const char* NEWHEADERCOLORSTR;
extern const char* HEADERCOLORINVALIDSTR;
extern const char* NEWTEXTCOLORSTR;
extern const char* TEXTCOLORINVALIDSTR;

// oled messages
extern const char* CURRENTPOSSTR;
extern const char* TARGETPOSSTR;
extern const char* COILPWRSTR;
extern const char* REVDIRSTR;
extern const char* STEPMODESTR;
extern const char* TEMPSTR;
extern const char* MOTORSPEEDSTR;
extern const char* MAXSTEPSSTR;
extern const char* TCOMPSTEPSSTR;
extern const char* TCOMPSTATESTR;
extern const char* TCOMPDIRSTR;
extern const char* BACKLASHINSTR;
extern const char* BACKLASHOUTSTR;
extern const char* BACKLASHINSTEPSSTR;
extern const char* BACKLASHOUTSTEPSSTR;
extern const char* BLUETOOTHSTR;
extern const char* LOCALSERIALSTR;
extern const char* FSFILENOTFOUNDSTR;
extern const char* SETPGOPTIONSTR;
extern const char* SENDPAGESTR;
extern const char* SETPGTIMESTR;

extern const char* CREATEFILEFAILSTR;
extern const char* WRITEFILEFAILSTR;
extern const char* WRITEFILESUCCESSSTR;


// defines for ASCOMSERVER, MDNSSERVER, WEBSERVER
#define ASCOMREMOTESTR            "ASCOM Remote: "
#define WEBSERVERSTR              "Webserver: "
#define NORMALWEBPAGE             200
#define FILEUPLOADSUCCESS         300
#define BADREQUESTWEBPAGE         400
#define NOTFOUNDWEBPAGE           404
#define INTERNALSERVERERROR       500

#define TEXTPAGETYPE              "text/html"
#define PLAINTEXTPAGETYPE         "text/plain"
#define JSONTEXTPAGETYPE          "text/json"
#define JSONPAGETYPE              "application/json"
#define FILENOTFOUNDSTR           "Not found"
#define FILEFOUNDSTR              "Found"
#define NOTDEFINEDSTR             "Not defined"

#define MANAGEMENTISMOVINGSTR     "<html><head><title>Management Server</title></head><body><p>Focuser is Moving. Please try again once focuser has stopped</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOMEPAGE\"></form></p></body></html>"
#define MANAGEMENTNOTFOUNDSTR     "<html><head><title>Management Server</title></head><body><p>URL not found</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOMEPAGE\"></form></p></body></html>"
#define WEBSERVERNOTFOUNDSTR      "<html><head><title>Web Server</title></head><body><p>URL not found</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOMEPAGE\"></form></p></body></html>"
#define ASCOMSERVERNOTFOUNDSTR    "<html><head><title>ASCOM REMOTE Server</title></head><body><p>FS not started</p><p><p><a href=\"/setup/v1/focuser/0/setup\">Setup page</a></p></body></html>";

#define MDNSSTARTFAILSTR          "Err starting MDNS responder"
#define MDNSSTARTEDSTR            "mDNS responder started"

#define CREBOOTSTR                "<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"srestart\" value=\"true\"><input type=\"submit\" onclick=\"return confirm('Are you sure?')\" value=\"REBOOT CONTROLLER\"></form>"

#define ENABLEBKINSTR             "<form action=\"/msindex3\" method=\"post\"><b>BL-IN State</b> [%STI%]: <input type=\"hidden\" name=\"enin\" value=\"true\"><input type=\"submit\" value=\"ENABLE\"></form>"
#define DISABLEBKINSTR            "<form action=\"/msindex3\" method=\"post\"><b>BL-IN State</b> [%STI%]: <input type=\"hidden\" name=\"diin\" value=\"true\"><input type=\"submit\" value=\"DISABLE\"></form>"
#define ENABLEBKOUTSTR            "<form action=\"/msindex3\" method=\"post\"><b>BL-OUT State</b> [%STO%]: <input type=\"hidden\" name=\"enou\" value=\"true\"><input type=\"submit\" value=\"ENABLE\"></form>"
#define DISABLEBKOUTSTR           "<form action=\"/msindex3\" method=\"post\"><b>BL-OUT State</b> [%STO%]: <input type=\"hidden\" name=\"diou\" value=\"true\"><input type=\"submit\" value=\"DISABLE\"></form>"
#define BLINSTEPSTR               "<form action=\"/msindex3\" method =\"post\"><b>BL-In &nbsp;Steps:</b> <input type=\"text\" name=\"bis\" size=\"6\" value=\"%bins%\"> <input type=\"submit\" name=\"setbis\" value=\"Set\"></form>"
#define BLOUTSTEPSTR              "<form action=\"/msindex3\" method =\"post\"><b>BL-Out Steps:</b> <input type=\"text\" name=\"bos\" size=\"6\" value=\"%bous%\"> <input type=\"submit\" name=\"setbos\" value=\"Set\"></form>"

#define ENABLEINDISTR             "<form action=\"/msindex3\" method=\"post\"><b>State</b> [%INI%]: <input type=\"hidden\" name=\"indion\" value=\"true\"><input type=\"submit\" value=\"ENABLE\"></form>"
#define DISABLEINDISTR            "<form action=\"/msindex3\" method=\"post\"><b>State</b> [%INI%]: <input type=\"hidden\" name=\"indioff\" value=\"true\"><input type=\"submit\" value=\"DISABLE\"></form>"

#define ENABLEPBSTR               "<form action=\"/msindex3\" method=\"post\"><b>State</b> [%PBL%]: <input type=\"hidden\" name=\"pbon\" value=\"true\"><input type=\"submit\" value=\"ENABLE\"></form>"
#define DISABLEPBSTR              "<form action=\"/msindex3\" method=\"post\"><b>State</b> [%PBL%]: <input type=\"hidden\" name=\"pboff\" value=\"true\"><input type=\"submit\" value=\"DISABLE\"></form>"

#define ENABLEHPSWSTR             "<form action=\"/msindex2\" method=\"post\"><b>State</b> [%HPL%]: <input type=\"hidden\" name=\"hpswon\" value=\"true\"><input type=\"submit\" value=\"ENABLE\"></form>"
#define DISABLEHPSWSTR            "<form action=\"/msindex2\" method=\"post\"><b>State</b> [%HPL%]: <input type=\"hidden\" name=\"hpswoff\" value=\"true\"><input type=\"submit\" value=\"DISABLE\"></form>"

#define ENABLELEDSTR              "<form action=\"/msindex2\" method=\"post\"><b>State</b> [%INL%]: <input type=\"hidden\" name=\"startle\" value=\"true\"><input type=\"submit\" value=\"ENABLE\"></form>"
#define DISABLELEDSTR             "<form action=\"/msindex2\" method=\"post\"><b>State</b> [%INL%]: <input type=\"hidden\" name=\"stople\" value=\"true\"><input type=\"submit\" value=\"DISABLE\"></form>"
#define ENABLETEMPSTR             "<form action=\"/msindex2\" method=\"post\"><b>State</b> [%TPE%]: <input type=\"hidden\" name=\"starttp\" value=\"true\"><input type=\"submit\" value=\"ENABLE\"></form>"
#define DISABLETEMPSTR            "<form action=\"/msindex2\" method=\"post\"><b>State</b> [%TPE%]: <input type=\"hidden\" name=\"stoptp\" value=\"true\"><input type=\"submit\" value=\"DISABLE\"></form>"
#define STOPTSSTR                 "<form action=\"/msindex2\" method=\"post\"><b>Status</b> [%TST%]: <input type=\"hidden\" name=\"stopts\" value=\"true\"><input type=\"submit\" value=\"STOP\"></form>"
#define STARTTSSTR                "<form action=\"/msindex2\" method=\"post\"><b>Status</b> [%TST%]: <input type=\"hidden\" name=\"startts\" value=\"true\"><input type=\"submit\" value=\"START\"></form>"
#define STOPWSSTR                 "<form action=\"/msindex2\" method=\"post\"><b>Status</b> [%WST%]: <input type=\"hidden\" name=\"stopws\" value=\"true\"><input type=\"submit\" value=\"STOP\"></form>"
#define STARTWSSTR                "<form action=\"/msindex2\" method=\"post\"><b>Status</b> [%WST%]: <input type=\"hidden\" name=\"startws\" value=\"true\"><input type=\"submit\" value=\"START\"></form>"
#define STOPASSTR                 "<form action=\"/msindex2\" method=\"post\"><b>Status</b> [%ABT%]: <input type=\"hidden\" name=\"stopas\" value=\"true\"><input type=\"submit\" value=\"STOP\"></form>"
#define STARTASSTR                "<form action=\"/msindex2\" method=\"post\"><b>Status</b> [%ABT%]: <input type=\"hidden\" name=\"startas\" value=\"true\"><input type=\"submit\" value=\"START\"></form>"
#define DISPLAYCSTR               "<form action=\"/msindex2\" method=\"post\"><b>Temp Mode: </b><input type=\"hidden\" name=\"tm\" value=\"cel\" Checked><input type=\"submit\" value=\"Enable Celsius\"></form>"
#define DISPLAYFSTR               "<form action=\"/msindex2\" method=\"post\"><b>Temp Mode: </b><input type=\"hidden\" name=\"tm\" value=\"fah\"><input type=\"submit\" value=\"Enable Fahrenheit\"></form>"

#define MDNSTOPSTR                "<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"stopmdns\" value=\"true\"><input type=\"submit\" value=\"STOP\"></form>"
#define MDNSSTARTSTR              "<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"startmdns\" value=\"true\"><input type=\"submit\" value=\"START\"></form>"

#define DISPLAYONSTR              "<form action=\"/\" method=\"post\"><b>DISPLAY: </b><input type=\"hidden\" name=\"di\" value=\"doff\" Checked><input type=\"submit\" value=\"Disable\"></form>"
#define DISPLAYOFFSTR             "<form action=\"/\" method=\"post\"><b>DISPLAY: </b><input type=\"hidden\" name=\"di\" value=\"don\"><input type=\"submit\" value=\"Enable\"></form>"

#define STARTSCREENONSTR          "<form action=\"/\" method=\"post\"><b>Startscreen: </b><input type=\"hidden\" name=\"ss\" value=\"ssoff\" Checked><input type=\"submit\" value=\"Disable\"></form>"
#define STARTSCREENOFFSTR         "<form action=\"/\" method=\"post\"><b>Startscreen: </b><input type=\"hidden\" name=\"ss\" value=\"sson\"><input type=\"submit\" value=\"Enable\"></form>"

#define STARTHPSWMONSTR           "<form action=\"/\" method=\"post\"><b>HPSW Messages: </b><input type=\"hidden\" name=\"hp\" value=\"hpoff\" Checked><input type=\"submit\" value=\"Disable\"></form>"
#define STARTHPSWMOFFSTR          "<form action=\"/\" method=\"post\"><b>HPSW Messages: </b><input type=\"hidden\" name=\"hp\" value=\"hpon\"><input type=\"submit\" value=\"Enable\"></form>"

#define STARTFMDLONSTR            "<form action=\"/\" method=\"post\"><b>MS Forcedownload: </b><input type=\"hidden\" name=\"fd\" value=\"fdoff\" Checked><input type=\"submit\" value=\"Disable\"></form>"
#define STARTFMDLOFFSTR           "<form action=\"/\" method=\"post\"><b>MS Forcedownload: </b><input type=\"hidden\" name=\"fd\" value=\"fdon\"><input type=\"submit\" value=\"Enable\"></form>"

// ======================================================================
// 2. TRACING -- DO NOT CHANGE
// ======================================================================
// ArduinoTrace - github.com/bblanchon/ArduinoTrace
// Copyright Benoit Blanchon 2018-2019
// Provide a trace fucntion, printing file, line number, function and parameters
// DEBUG needs to be defined to get output to Serial Port
// If DEBUG is not defined nothing happens
#define TRACE() \
DebugPrint(__FILE__); \
DebugPrint(':'); \
DebugPrint(__LINE__); \
DebugPrint(": "); \
DebugPrintln(__PRETTY_FUNCTION__);

// ======================================================================
// 3. DEBUGGING -- DO NOT CHANGE
// ======================================================================
//#define DEBUG 1

#ifdef  DEBUG                                         // Macros are usually in all capital letters.
#define DebugPrint(...) Serial.print(__VA_ARGS__)     // DPRINT is a macro, debug print
#define DebugPrintln(...) Serial.println(__VA_ARGS__) // DPRINTLN is a macro, debug print with new line
#else
#define DebugPrint(...)                               // now defines a blank line
#define DebugPrintln(...)                             // now defines a blank line
#endif

// ======================================================================
// 4. HEAP DEBUGGING - DO NOT CHANGE / DO NOT ENABLE
// ======================================================================
#define HEAPDEBUG     1

#ifdef  HEAPDEBUG   
#define HDebugPrint(...) Serial.print(__VA_ARGS__)      // HDebugPrint is a macro, serial print
#define HDebugPrintln(...) Serial.println(__VA_ARGS__)  // HDebugPrintln is a macro, serial print with new line
#define HDebugPrintf(...) Serial.printf(__VA_ARGS__)    // HDebugPrintf is a macro, serial printf
#else
#define HDebugPrint(...)                                // now defines a blank line
#define HDebugPrintln(...)                              // now defines a blank line
#define HDebugPrintf(...)
#endif

// ======================================================================
// 5. TIMING TESTS - DO NOT CHANGE / DO NOT ENABLE
// ======================================================================
//#define TIMEDTESTS 1

#ifdef TIMEDTESTS

#define TIMESETUP                   1
//#define TIMELOOP                    1
#define TIMEWSROOTSEND              1
#define TIMEWSROOTHANDLE            1
#define TIMEWSROOTBUILD             1
#define TIMEWSMOVEHANDLE            1
#define TIMEWSBUILDMOVE             1
#define TIMEWSSENDMOVE              1
#define TIMEWSSENDPRESETS           1
#define TIMEWSHANDLEPRESETS         1
#define TIMEWSBUILDPRESETS          1

#define TIMEMSSENDPG1               1
#define TIMEMSSENDPG2               1
#define TIMEMSSENDPG3               1
#define TIMEMSSENDPG4               1
#define TIMEMSSENDPG5               1
#define TIMEMSBUILDPG1              1
#define TIMEMSBUILDPG2              1
#define TIMEMSBUILDPG3              1
#define TIMEMSBUILDPG4              1
#define TIMEMSBUILDPG5              1
#define TIMEMSHANDLEPG1             1
#define TIMEMSHANDLEPG2             1
#define TIMEMSHANDLEPG3             1
#define TIMEMSHANDLEPG4             1
#define TIMEMSHANDLEPG5             1

#define TIMEASCOMROOT               1
#define TIMEASCOMBUILDSETUP         1
#define TIMEASCOMHANDLESETUP        1
#define TIMEASCOMHANDLEFOCUSERSETUP 1
#define TIMEASCOMHANDLEAPIVER       1
#define TIMEASCOMHANDLEAPIDES       1
#define TIMEASCOMHANDLEAPICON       1
#endif

#endif // generalDefinitions.h
