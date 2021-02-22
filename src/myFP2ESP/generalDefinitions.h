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

#define MAXWEBPAGESIZE        3400
#define MAXASCOMPAGESIZE      2200
#define MAXMANAGEMENTPAGESIZE 3600          // largest page is MANAGEMENT_buildpredefinedboard() = 3469

#ifndef SLOW
#define SLOW                  0             // motorspeeds
#endif
#ifndef MED
#define MED                   1
#endif
#ifndef FAST
#define FAST                  2
#endif

// ======================================================================
// 2: DO NOT CHANGE
// ======================================================================

#define moving_in             false
#define moving_out            !moving_in
#define moving_main           moving_in               

#ifndef STEP1
#define STEP1                 1             // stepmodes
#endif
#ifndef STEP2
#define STEP2                 2
#endif
#ifndef STEP4
#define STEP4                 4
#endif
#ifndef STEP8
#define STEP8                 8
#endif
#ifndef STEP16
#define STEP16                16
#endif
#ifndef STEP32
#define STEP32                32
#endif
#ifndef STEP64
#define STEP64                64
#endif
#ifndef STEP128
#define STEP128               128
#endif
#ifndef STEP256
#define STEP256               256
#endif

#define EOFSTR                '#'
#define STARTCMDSTR           ':'

// ======================================================================
// 3: DEFINES FOR BOARD TYPES
// ======================================================================
#ifndef CUSTOMBRD
#define CUSTOMBRD             99          // For a user custom board see 0.jsn in /data/boards folder
#endif
#ifndef WEMOSDRV8825
#define WEMOSDRV8825          35          // if using a drv8825 you also need to set DRV8825STEPMODE in myBoards.h
#endif
#ifndef PRO2EDRV8825                     
#define PRO2EDRV8825          36          // if using a drv8825 you also need to set DRV8825STEPMODE in myBoards.h
#endif
#ifndef PRO2EDRV8825BIG                   // PRO2EDRV8825BIG Board is now deprecated
#define PRO2EDRV8825BIG       37          // if using a drv8825 you also need to set DRV8825STEPMODE in myBoards.h
#endif
#ifndef PRO2EULN2003
#define PRO2EULN2003          38
#endif
#ifndef PRO2EL293DNEMA
#define PRO2EL293DNEMA        39          // Motor shield ESP8266 with NEMA motor
#endif
#ifndef PRO2EL293D28BYJ48
#define PRO2EL293D28BYJ48     40          // Motor shield ESP8266 with 28BYJ48 motor
#endif
#ifndef PRO2EL298N
#define PRO2EL298N            41          // uses PCB layout for ULN2003
#endif
#ifndef PRO2EL293DMINI
#define PRO2EL293DMINI        42          // uses PCB layout for ULN2003
#endif
#ifndef PRO2EL9110S
#define PRO2EL9110S           43          // uses PCB layout for ULN2003
#endif
#ifndef PRO2ESP32DRV8825
#define PRO2ESP32DRV8825      44
#endif
#ifndef PRO2ESP32ULN2003
#define PRO2ESP32ULN2003      45
#endif
#ifndef PRO2ESP32L298N
#define PRO2ESP32L298N        46
#endif
#ifndef PRO2ESP32L293DMINI
#define PRO2ESP32L293DMINI    47          // uses PCB layout for ULN2003
#endif
#ifndef PRO2ESP32L9110S
#define PRO2ESP32L9110S       48          // uses PCB layout for ULN2003
#endif
#ifndef PRO2ESP32R3WEMOS
#define PRO2ESP32R3WEMOS      49          // https://www.ebay.com/itm/R3-Wemos-UNO-D1-R32-ESP32-WIFI-Bluetooth-CH340-Devolopment-Board-For-Arduino/264166013552
#endif
#ifndef WEMOSDRV8825H
#define WEMOSDRV8825H         50          // this is for Holger
#endif

extern const char* programVersion;
extern const char* ProgramAuthor;

extern const char* STATEMOVINGSTR;
extern const char* STATEAPPLYBACKLASH;
extern const char* STATESETHOMEPOSITION;
extern const char* STATEFINDHOMEPOSITION;
extern const char* STATEDELAYAFTERMOVE;
extern const char* STATEFINISHEDMOVE;
extern const char* STATEIDLE;
extern const char* STATEINITMOVE;
extern const char* PORTSTR;
extern const char* SENDSTR;
extern const char* SERIALSTARTSTR;
extern const char* DEBUGONSTR;
extern const char* BLUETOOTHSTARTSTR;
extern const char* ATTEMPTCONNSTR;
extern const char* APSTARTFAILSTR;
extern const char* SERVERREADYSTR;
extern const char* STARTSTR;
extern const char* ENDSTR;
extern const char* PROGRESSSTR;
extern const char* ERRORSTR;
extern const char* READYSTR;
extern const char* SETUPDRVBRDSTR;
extern const char* DRVBRDDONESTR;
extern const char* CHECKCPWRSTR;
extern const char* CPWRRELEASEDSTR;
extern const char* STARTAPSTR;
extern const char* STARTSMSTR;
extern const char* SETSTATICIPSTR;
extern const char* ATTEMPTSSTR;
extern const char* STARTTCPSERVERSTR;
extern const char* TCPSERVERSTARTEDSTR;
extern const char* GETLOCALIPSTR;
extern const char* SETUPDUCKDNSSTR;
extern const char* SETUPENDSTR;
extern const char* STARTOTASERVICESTR;
extern const char* SSIDSTR;
extern const char* IPADDRESSSTR;
extern const char* WIFIRESTARTSTR;
extern const char* WIFIBEGINSTATUSSTR;
extern const char* CHECKFORTPROBESTR;
extern const char* ACCESSPOINTSTR;
extern const char* STATIONMODESTR;
extern const char* CONFIGSAVEDSTR;
extern const char* RELEASEMOTORSTR;
extern const char* LOOPSTARTSTR;
extern const char* LOOPENDSTR;
extern const char* TCPCLIENTCONNECTSTR;
extern const char* TCPCLIENTDISCONNECTSTR;
extern const char* APCONNECTFAILSTR;
extern const char* CONNECTEDSTR;
extern const char* I2CDEVICENOTFOUNDSTR;
extern const char* ASCOMREMOTESTR;
extern const char* REBOOTWSSTR;
extern const char* REBOOTTCPSTR;
extern const char* REBOOTCNTLRSTR;
extern const char* REBOOTASCOMSTR;
extern const char* ASCOMSERVERNOTDEFINEDSTR;
extern const char* WEBSERVERNOTDEFINEDSTR;
extern const char* SETPGOPTIONSTR;
extern const char* SETPGTIMESTR;

extern const char* MANAGEMENTNOTFOUNDSTR;
extern const char* WEBSERVERNOTFOUNDSTR;
extern const char* ASCOMSERVERNOTFOUNDSTR;

extern const char* WRITEFILEFAILSTR;
extern const char* WRITEFILESUCCESSSTR;
extern const char* CREATEFILEFAILSTR;
extern const char* CHECKWIFICONFIGFILESTR;
extern const char* DESERIALIZEERRORSTR;
extern const char* SERVERNOTRUNNINGSTR;

extern const char* HPCLOSEDFPNOT0STR;
extern const char* HPCLOSEDFP0STR;
extern const char* HPMOVETILLOPENSTR;
extern const char* HPMOVEOUTERRORSTR;
extern const char* HPMOVEOUTSTEPSSTR;
extern const char* HPMOVEOUTFINISHEDSTR;

// temperature probe messages
extern const char* TPROBESTR;
extern const char* TPROBENOTFOUNDSTR;
extern const char* GETTEMPPROBESSTR;
extern const char* SETTPROBERESSTR;

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

// joystick messages
#define UPDATEJOYSTICKSTR         "joystick: update joystick"
#define JOYSTICKVALSTR            "Raw joyval:"
#define JOYSTICKXINVALSTR         "X IN joyval:"
#define JOYSTICKSPEEDSTR          ", Speed:"
#define JOYSTICKXOUTVALSTR        "X OUT joyval:"

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
#define FILENOTFOUNDSTR           "File not found"
#define FILEFOUNDSTR              "File found"
#define CANNOTCREATEFILESTR       "err: create file"

#define FSNOTSTARTEDSTR           "err: start FS"
#define BUILDDEFAULTPAGESTR       "build default page"
#define SPIFFSFILENOTFOUNDSTR     "file not found"
#define READPAGESTR               "read page into string"
#define PROCESSPAGESTARTSTR       "process page start"
#define PROCESSPAGEENDSTR         "process page done"
#define STARTASCOMSERVERSTR       "start ascom server"
#define STOPASCOMSERVERSTR        "stop ascom server"
#define STARTWEBSERVERSTR         "start web server"
#define STOPWEBSERVERSTR          "stop web server"
#define STOPMDNSSERVERSTR         "stop mdns server"

#define SERVERSTATESTOPSTR        "STOPPED"
#define SERVERSTATESTARTSTR       "STARTED"
#define SERVERSTATERUNSTR         "RUNNING"
#define SENDPAGESTR               "Send page"
#define ENABLEDSTR                "Enabled"
#define NOTENABLEDSTR             "Disabled"

#define MANAGEMENTNOTFOUNDSTR   "<html><head><title>Management Server</title></head><body><p>URL not found</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOMEPAGE\"></form></p></body></html>"
#define WEBSERVERNOTFOUNDSTR    "<html><head><title>Web Server</title></head><body><p>URL not found</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOMEPAGE\"></form></p></body></html>"
#define ASCOMSERVERNOTFOUNDSTR  "<html><head><title>ASCOM REMOTE Server</title></head><body><p>FS not started</p><p><p><a href=\"/setup/v1/focuser/0/setup\">Setup page</a></p></body></html>";

#define MDNSSTARTFAILSTR          "Err setting up MDNS responder"
#define MDNSSTARTEDSTR            "mDNS responder started"

#define CREBOOTSTR                "<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"srestart\" value=\"true\"><input type=\"submit\" value=\"REBOOT CONTROLLER\"></form>"

#define ENABLEBKINSTR             "<form action=\"/msindex3\" method=\"post\"><b>BL-IN State</b> [%STI%]: <input type=\"hidden\" name=\"enin\" value=\"true\"><input type=\"submit\" value=\"ENABLE\"></form>"
#define DISABLEBKINSTR            "<form action=\"/msindex3\" method=\"post\"><b>BL-IN State</b> [%STI%]: <input type=\"hidden\" name=\"diin\" value=\"true\"><input type=\"submit\" value=\"DISABLE\"></form>"
#define ENABLEBKOUTSTR            "<form action=\"/msindex3\" method=\"post\"><b>BL-OUT State</b> [%STO%]: <input type=\"hidden\" name=\"enou\" value=\"true\"><input type=\"submit\" value=\"ENABLE\"></form>"
#define DISABLEBKOUTSTR           "<form action=\"/msindex3\" method=\"post\"><b>BL-OUT State</b> [%STO%]: <input type=\"hidden\" name=\"diou\" value=\"true\"><input type=\"submit\" value=\"DISABLE\"></form>"
#define BLINSTEPSTR               "<form action=\"/msindex3\" method =\"post\"><b>BL-In &nbsp;Steps:</b> <input type=\"text\" name=\"bis\" size=\"6\" value=\"%bins%\"> <input type=\"submit\" name=\"setbis\" value=\"Set\"></form>"
#define BLOUTSTEPSTR              "<form action=\"/msindex3\" method =\"post\"><b>BL-Out Steps:</b> <input type=\"text\" name=\"bos\" size=\"6\" value=\"%bous%\"> <input type=\"submit\" name=\"setbos\" value=\"Set\"></form>"

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
#define DISPLAYONSTR              "<form action=\"/\" method=\"post\"><b>Display: </b><input type=\"hidden\" name=\"di\" value=\"doff\" Checked><input type=\"submit\" value=\"Turn Off\"></form>"
#define DISPLAYOFFSTR             "<form action=\"/\" method=\"post\"><b>Display: </b><input type=\"hidden\" name=\"di\" value=\"don\"><input type=\"submit\" value=\"Turn On\"></form>"
#define STARTSCREENONSTR          "<form action=\"/\" method=\"post\"><b>Startscreen: </b><input type=\"hidden\" name=\"ss\" value=\"ssoff\" Checked><input type=\"submit\" value=\"Turn Off\"></form>"
#define STARTSCREENOFFSTR         "<form action=\"/\" method=\"post\"><b>Startscreen: </b><input type=\"hidden\" name=\"ss\" value=\"sson\"><input type=\"submit\" value=\"Turn On\"></form>"
#define STARTHPSWMONSTR           "<form action=\"/\" method=\"post\"><b>HPSW Messages: </b><input type=\"hidden\" name=\"hp\" value=\"hpoff\" Checked><input type=\"submit\" value=\"Turn Off\"></form>"
#define STARTHPSWMOFFSTR          "<form action=\"/\" method=\"post\"><b>HPSW Messages: </b><input type=\"hidden\" name=\"hp\" value=\"hpon\"><input type=\"submit\" value=\"Turn On\"></form>"
#define STARTFMDLONSTR            "<form action=\"/\" method=\"post\"><b>MS Forcedownload: </b><input type=\"hidden\" name=\"fd\" value=\"fdoff\" Checked><input type=\"submit\" value=\"Turn Off\"></form>"
#define STARTFMDLOFFSTR           "<form action=\"/\" method=\"post\"><b>MS Forcedownload: </b><input type=\"hidden\" name=\"fd\" value=\"fdon\"><input type=\"submit\" value=\"Turn On\"></form>"

#define NOTDEFINEDSTR             "not defined in firmware"

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
//#define HEAPDEBUG     1

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
