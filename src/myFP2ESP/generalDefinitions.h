// ======================================================================
// generalDefinitions.h : myFP2ESP GENERAL DEFINITIONS
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

#ifndef generalDefinitions_h
#define generalDefinitions_h

#include <Arduino.h>

struct pbTimer {
  uint32_t current;
  uint32_t last;
  uint32_t initial;
};

// ======================================================================
// 1: GENERAL DEFINES -- DO NOT CHANGE
// ======================================================================

enum connection_status { disconnected, connected };
//  StateMachine definition
enum StateMachineStates { State_Idle, State_InitMove, State_Backlash, State_Moving, State_DelayAfterMove, State_FinishedMove, State_SetHomePosition };

// controller modes
#define BLUETOOTHMODE         1
#define ACCESSPOINT           2
#define STATIONMODE           3
#define LOCALSERIAL           4
//#define OLED_TEXT             1
//#define OLED_GRAPHIC          2
#define MYFP2ESP_PROTOCOL     1
#define MOONLITE_PROTOCOL     2

// INTERFACE SETTINGS
#define EOFSTR                '#'
#define STARTCMDSTR           ':'
#define ESPDATA               0             // command has come from tcp/ip
#define BTDATA                1             // command has come from bluetooth
#define SERIALDATA            2             // command has come from serial port
#define QUEUELENGTH           20            // number of commands that can be saved in the serial queue
#define RUNNING               true          // service state running
#define STOPPED               false         // service state stopped
#define REBOOTDELAY           2000          // When rebooting controller, delay (2s) from msg to actual reboot
#define moving_in             false
#define moving_out            !moving_in
#define moving_main           moving_in

// MOTOR SETTINGS
#define MOTORPULSETIME        2             // DO NOT CHANGE
#define DEFAULTSTEPSIZE       50.0          // This is the default setting for the step size in microns
#define MINIMUMSTEPSIZE       0.0
#define MAXIMUMSTEPSIZE       100.0
#define DEFAULTPOSITION       5000L
#define DEFAULTMAXSTEPS       80000L
#define MotorReleaseDelay     120*1000      // motor release power after 120s
#define FOCUSERUPPERLIMIT     2000000000L   // arbitary focuser limit up to 2000000000
#define FOCUSERLOWERLIMIT     1024L         // lowest value that maxsteps can be
#define HOMESTEPS             200           // Prevent searching for home position switch never returning, this should be > than # of steps between closed and open
#define HPSWOPEN              0             // hpsw states refelect status of switch
#define HPSWCLOSED            1

// ASCOM SERVICE
#define ALPACAPORT            4040          // ASCOM Remote port
#define MAXASCOMPAGESIZE      2200          // largest = /setuppage = 2042

// DISPLAY
#define OLEDPAGETIMEMIN       2             // 2s minimum oled page display time
#define OLEDPAGETIMEMAX       10            // 10s maximum oled page display time
#define OLEDPGOPTIONALL       7             // oled page enable, ALL pages "111"
#define OLEDUPDATEONMOVE      15            // defines how many steps before refreshing position when moving if oledupdateonmove is 1

// DUCKDNS SERVICE
#define DUCKDNS_REFRESHRATE   60000         // duck dns, check ip address every 60s for an update

// MANAGEMENT SERVICE
#define MSSERVERPORT          6060          // Management interface - cannot be changed
#define MSREBOOTPAGEDELAY     20000         // management service reboot page, time (s) between next page refresh
#define MAXMANAGEMENTPAGESIZE 3700          // largest = /msindex2 = 3568
#define MAXCUSTOMBRDJSONSIZE  300

// MDNS SERVICE
#define MDNSSERVERPORT        7070          // mDNS service

// SERIAL PORT
#define SERIALPORTSPEED       57600         // 9600, 14400, 19200, 28800, 38400, 57600, 115200
#define MOONLITESERIALPORTSPEED 9600

// TCP/IP SERVICE
#define SERVERPORT            2020          // TCPIP port for myFP2ESP

// TEMPERATURE PROBE
#define TEMPREFRESHRATE       3000L         // refresh rate between temperature conversions unless an update is requested via serial command
#define TEMPRESOLUTION        10            // Set the default DS18B20 precision to 0.25 of a degree 9=0.5, 10=0.25, 11=0.125, 12=0.0625

// TIMES AND DELAYS SETTINGS
#define DEFAULTSAVETIME       30000         // default time to wait before saving data to FS

// WEBSERVER SERVICE
#define WEBSERVERPORT         80            // Web server port
#define WS_REFRESHRATE        60            // web server page refresh time 60s
#define MINREFRESHPAGERATE    10            // 10s - too low and the overhead becomes too much for the controller
#define MAXREFRESHPAGERATE    900           // 15m
#define MAXWEBPAGESIZE        4100          // largest = / = 3943

// defines for ASCOMSERVER, MDNSSERVER, WEBSERVER
#define ASCOMREMOTESTR        "ASCOM Remote: "
#define WEBSERVERSTR          "Webserver: "
#define NORMALWEBPAGE         200
#define FILEUPLOADSUCCESS     300
#define BADREQUESTWEBPAGE     400
#define NOTFOUNDWEBPAGE       404
#define INTERNALSERVERERROR   500
#define TEXTPAGETYPE          "text/html"
#define PLAINTEXTPAGETYPE     "text/plain"
#define JSONTEXTPAGETYPE      "text/json"
#define JSONPAGETYPE          "application/json"
#define FILENOTFOUNDSTR       "Not found"
#define FILEFOUNDSTR          "Found"
#define NOTDEFINEDSTR         "Not defined"
#define MDNSSTARTFAILSTR      "Err starting MDNS responder"
#define MDNSSTARTEDSTR        "mDNS responder started"

// ======================================================================
// 2: DO NOT CHANGE
// ======================================================================

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

// ======================================================================
// 3: DO NOT CHANGE
// ======================================================================
// DO NOT CHANGE ANY OF THESE
// DO NOT CHANGE ANY OF THESE

#define MANAGEMENTISMOVINGSTR     "<html><head><title>Management Server</title></head><body><p>Focuser is Moving. Please try again once focuser has stopped</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOMEPAGE\"></form></p></body></html>"
#define MANAGEMENTNOTFOUNDSTR     "<html><head><title>Management Server</title></head><body><p>URL not found</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOMEPAGE\"></form></p></body></html>"
#define WEBSERVERNOTFOUNDSTR      "<html><head><title>Web Server</title></head><body><p>URL not found</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOMEPAGE\"></form></p></body></html>"
#define ASCOMSERVERNOTFOUNDSTR    "<html><head><title>ASCOM REMOTE Server</title></head><body><p>FS not started</p><p><p><a href=\"/setup/v1/focuser/0/setup\">Setup page</a></p></body></html>";

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
// 4. MACRO FOR MUTEX: DO NOT CHANGE / DO NOT ENABLE
// ======================================================================
// This is a bit of sneaky code to only use Mutex on ESP32

#if defined(ESP8266)
// in esp8266, volatile data_type varname is all that is needed
#define varENTER_CRITICAL(...)            // blank line
#define varEXIT_CRITICAL(...)             // blank line
#else
// in esp32, we should use a Mutex for access
#define varENTER_CRITICAL(...)  portENTER_CRITICAL(__VA_ARGS__);
#define varEXIT_CRITICAL(...)   portEXIT_CRITICAL(__VA_ARGS__); 
#endif


// ======================================================================
// 5. HEAP DEBUGGING - DO NOT CHANGE / DO NOT ENABLE
// ======================================================================
//#define HEAPDEBUG     1

#ifdef  HEAPDEBUG
#define HDebugPrint(...) Serial.print(__VA_ARGS__)              // HDebugPrint is a macro, serial print
#define HDebugPrintln(...) Serial.println(__VA_ARGS__)          // HDebugPrintln is a macro, serial print with new line
#define HDebugPrintf(...) Serial.printf(__VA_ARGS__)            // HDebugPrintf is a macro, serial printf
#else
#define HDebugPrint(...)                                        // now defines a blank line
#define HDebugPrintln(...)                                      // now defines a blank line
#define HDebugPrintf(...)
#endif


// ======================================================================
// 6. DEBUGGING -- DO NOT CHANGE
// ======================================================================
//#define DEBUG 1

//#define ASCOM_DEBUG       1                                   // for debugging ascomserver
//#define BOARD_DEBUG       1                                   // for debugging myboards
//#define DISPLAY_DEBUG     1                                   // for debugging displays
//#define COMMS_DEBUG       1                                   // for debugging comms
//#define MANAGEMENT_DEBUG  1                                   // for debugging management server
//#define SETUP_DEBUG       1                                   // for debugging setup()
//#define SETUPDATA_DEBUG   1                                   // for debugging FocuserSetupData
//#define TEMP_DEBUG        1                                   // for debugging temp probe
//#define WEBSERVER_DEBUG   1                                   // for debugging webserver
//#define HPSW_Debug        1                                   // for debugging hpsw

#ifdef  DEBUG                                                   // Macros are usually in all capital letters.
#define DebugPrint(...)   Serial.print(__VA_ARGS__)             // DPRINT is a macro, debug print
#define DebugPrintln(...) Serial.println(__VA_ARGS__)           // DPRINTLN is a macro, debug print with new line
#else
#define DebugPrint(...)                                         // now defines a blank line
#define DebugPrintln(...)                                       // now defines a blank line
#endif

#ifdef  ASCOM_DEBUG                                             // for debugging ascom server
#define Ascom_DebugPrint(...) Serial.print(__VA_ARGS__)         // DPRINT is a macro, debug print
#define Ascom_DebugPrintln(...) Serial.println(__VA_ARGS__)     // DPRINTLN is a macro, debug print with new line
#else
#define Ascom_DebugPrint(...)                                   // now defines a blank line
#define Ascom_DebugPrintln(...)                                 // now defines a blank line
#endif

#ifdef  BOARD_DEBUG                                             // for debugging myboards.cpp
#define Board_DebugPrint(...) Serial.print(__VA_ARGS__)         // DPRINT is a macro, debug print
#define Board_DebugPrintln(...) Serial.println(__VA_ARGS__)     // DPRINTLN is a macro, debug print with new line
#else
#define Board_DebugPrint(...)                                   // now defines a blank line
#define Board_DebugPrintln(...)                                 // now defines a blank line
#endif

#ifdef  DISPLAY_DEBUG                                           // for debugging displays.cpp
#define Display_DebugPrint(...) Serial.print(__VA_ARGS__)       // DPRINT is a macro, debug print
#define Display_DebugPrintln(...) Serial.println(__VA_ARGS__)   // DPRINTLN is a macro, debug print with new line
#else
#define Display_DebugPrint(...)                                 // now defines a blank line
#define Display_DebugPrintln(...)                              // now defines a blank line
#endif

#ifdef  COMMS_DEBUG                                             // for debugging comms
#define Comms_DebugPrint(...) Serial.print(__VA_ARGS__)         // DPRINT is a macro, debug print
#define Comms_DebugPrintln(...) Serial.println(__VA_ARGS__)     // DPRINTLN is a macro, debug print with new line
#else
#define Comms_DebugPrint(...)                                   // now defines a blank line
#define Comms_DebugPrintln(...)                                 // now defines a blank line
#endif

#ifdef  MANAGEMENT_DEBUG                                        // for debugging management server
#define MSrvr_DebugPrint(...) Serial.print(__VA_ARGS__)         // DPRINT is a macro, debug print
#define MSrvr_DebugPrintln(...) Serial.println(__VA_ARGS__)     // DPRINTLN is a macro, debug print with new line
#else
#define MSrvr_DebugPrint(...)                                   // now defines a blank line
#define MSrvr_DebugPrintln(...)                                 // now defines a blank line
#endif

#ifdef  SETUP_DEBUG                                             // for debugging setup()
#define Setup_DebugPrint(...) Serial.print(__VA_ARGS__)         // DPRINT is a macro, debug print
#define Setup_DebugPrintln(...) Serial.println(__VA_ARGS__)     // DPRINTLN is a macro, debug print with new line
#else
#define Setup_DebugPrint(...)                                   // now defines a blank line
#define Setup_DebugPrintln(...)                                 // now defines a blank line
#endif

#ifdef  SETUPDATA_DEBUG                                         // for debugging FocuserSetupData
#define SetupData_DebugPrint(...) Serial.print(__VA_ARGS__)     // DPRINT is a macro, debug print
#define SetupData_DebugPrintln(...) Serial.println(__VA_ARGS__) // DPRINTLN is a macro, debug print with new line
#else
#define SetupData_DebugPrint(...)                               // now defines a blank line
#define SetupData_DebugPrintln(...)                             // now defines a blank line
#endif

#ifdef  TEMP_DEBUG                                              // for debugging temp.cpp
#define Temp_DebugPrint(...) Serial.print(__VA_ARGS__)          // DPRINT is a macro, debug print
#define Temp_DebugPrintln(...) Serial.println(__VA_ARGS__)      // DPRINTLN is a macro, debug print with new line
#else
#define Temp_DebugPrint(...)                                    // now defines a blank line
#define Temp_DebugPrintln(...)                                  // now defines a blank line
#endif

#ifdef  WEBSERVER_DEBUG                                         // for debugging webserver
#define WebS_DebugPrint(...) Serial.print(__VA_ARGS__)          // DPRINT is a macro, debug print
#define WebS_DebugPrintln(...) Serial.println(__VA_ARGS__)      // DPRINTLN is a macro, debug print with new line
#else
#define WebS_DebugPrint(...)                                    // now defines a blank line
#define WebS_DebugPrintln(...)                                  // now defines a blank line
#endif
#ifdef  HPSW_Debug
#define HPSW_DebugPrint(...)   Serial.print(__VA_ARGS__)  
#define HPSW_DebugPrintln(...) Serial.println(__VA_ARGS__) 
#else
#define HPSW_DebugPrint(...)  
#define HPSW_DebugPrintln(...) 
#endif


// ======================================================================
// 7. TRACING -- DO NOT CHANGE
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
// 8. TIMING TESTS - DO NOT CHANGE / DO NOT ENABLE
// ======================================================================
//#define TIMEDTESTS 1

#ifdef TIMEDTESTS

//#define TIMESETUP                   1
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
