// ---------------------------------------------------------------------------
// myFP2ESP GENERAL DEFINITIONS
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// COPYRIGHT
// ---------------------------------------------------------------------------
// (c) Copyright Robert Brown 2014-2020. All Rights Reserved.
// (c) Copyright Holger M, 2019-2020. All Rights Reserved.
// ---------------------------------------------------------------------------

#include <Arduino.h>

#ifndef generalDefinitions_h
#define generalDefinitions_h

// ---------------------------------------------------------------------------
// 1: GENERAL DEFINES -- DO NOT CHANGE
// ---------------------------------------------------------------------------

#define ALPACAPORT            4040          // ASCOM Remote server port
#define WEBSERVERPORT         80            // Web server port
#define MSSERVERPORT          6060          // Management interface - cannot be changed
#define MDNSSERVERPORT        7070          // mDNS service
#define WS_REFRESHRATE        60            // web server page refresh time 60s
#define MINREFRESHPAGERATE    10            // 10s - too low and the overhead becomes too much for the controller
#define MAXREFRESHPAGERATE    900           // 15m
#define DUCKDNS_REFREHRATE    60000         // duck dns, check ip address every 60s for an update
#define RUNNING               true          // service state running
#define STOPPED               false         // service state stopped
#define MSREBOOTPAGEDELAY     20000         // management service reboot page, time (s) between next page refresh
#define REBOOTDELAY           2000          // When rebooting controller, delay (2s) from msg to actual reboot
#define MotorReleaseDelay     120*1000      // motor release power after 120s

#define OLED_ADDR             0x3C          // some OLED displays maybe at 0x3F, use I2Cscanner to find correct address
#define SCREEN_WIDTH          128           // OLED display width, in pixels
#define SCREEN_HEIGHT         64            // OLED display height, in pixels

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
#define TEMPPRECISION         10            // Set the default DS18B20 precision to 0.25 of a degree 9=0.5, 10=0.25, 11=0.125, 12=0.0625
#define LCDUPDATEONMOVE       15            // defines how many steps before refreshing position when moving if lcdupdateonmove is 1
#define FOCUSERUPPERLIMIT     2000000000L   // arbitary focuser limit up to 2000000000
#define FOCUSERLOWERLIMIT     1024L         // lowest value that maxsteps can be
#define LCDPAGETIMEMIN        2             // 2s minimum lcd page display time
#define LCDPAGETIMEMAX        10            // 10s maximum lcd page display time
#define DEFAULTSAVETIME       30000         // default time to wait before saving data to SPIFFS
#define HOMESTEPS             200           // Prevent searching for home position switch never returning, this should be > than # of steps between closed and open
#define HPSWOPEN              0             // hpsw states refelect status of switch
#define HPSWCLOSED            1

#ifdef HOMEPOSITIONSWITCH
#define HPS_alert             !((bool)digitalRead(HPSWPIN))
#else
#define HPS_alert             false
#endif

#ifndef SLOW
#define SLOW                  0             // motorspeeds
#endif
#ifndef MED
#define MED                   1
#endif
#ifndef FAST
#define FAST                  2
#endif

// You can set the speed of the motor when performing backlash to SLOW, MED or FAST
#define BACKLASHSPEED         SLOW

// ---------------------------------------------------------------------------
// 2: DO NOT CHANGE
// ---------------------------------------------------------------------------

#define moving_in             false
#define moving_out            !moving_in

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

#define STATEMOVINGSTR        ">Moving"
#define STATEAPPLYBACKLASH    ">ApplyBacklash"
#define STATESETHOMEPOSITION  ">SetHomePosition"
#define STATEFINDHOMEPOSITION ">FindHomePosition#"
#define STATEDELAYAFTERMOVE   ">DelayAfterMove"
#define STATEFINISHEDMOVE     ">FinishedMove"
#define STATEIDLE             ">Idle"
#define STATEINITMOVE         ">InitMove"
#define EOFSTR                '#'
#define STARTCMDSTR           ':'
#define PORTSTR               "Port= "
#define SENDSTR               "Send= "
#define SERIALSTARTSTR        "Serial started"
#define DEBUGONSTR            "Debug on"
#define BLUETOOTHSTARTSTR     "Bluetooth started"
#define ATTEMPTCONNSTR        "Attempt connection to= "
#define SERVERREADYSTR        "Server Ready"
#define STARTSTR              "Start"
#define ENDSTR                "End"
#define PROGRESSSTR           "Progress: "
#define ERRORSTR              "Err= "
#define READYSTR              "Ready"
#define SETUPDRVBRDSTR        "Setup drvbrd= "
#define DRVBRDDONESTR         "Driver board done"
#define CHECKCPWRSTR          "Check coilpower"
#define CPWRRELEASEDSTR       "Coil power released"
#define STARTAPSTR            "Start Access Point"
#define STARTSMSTR            "Start Station mode"
#define SETSTATICIPSTR        "Setup Static IP"
#define ATTEMPTSSTR           "Attempt= "
#define STARTTCPSERVERSTR     "Start TCP Server"
#define TCPSERVERSTARTEDSTR   "TCP Server started"
#define GETLOCALIPSTR         "Get local IP address"
#define SETUPDUCKDNSSTR       "Setup DuckDNS"
#define SETUPENDSTR           "Setup end"
#define STARTOTASERVICESTR    "Start OTA service"
#define SSIDSTR               "SSID = "
#define IPADDRESSSTR          "IP   = "
#define WIFIRESTARTSTR        "Restarting"
#define WIFIBEGINSTATUSSTR    "Wifi.begin status code: "
#define CHECKFORTPROBESTR     "Check for Tprobe"
#define ACCESSPOINTSTR        "Access point: "
#define STATIONMODESTR        "Station mode: "
#define CONFIGSAVEDSTR        "new Config saved: "
#define CONFIGSAVEREQUESTSTR  "Config save requested "
#define RELEASEMOTORSTR       "Idle: release motor"
#define TCPCLIENTCONNECTSTR   "tcp client connected"
#define TCPCLIENTDISCONNECTSTR "tcp client disconnected"
#define APCONNECTFAILSTR      "!connect to AP "
#define CONNECTEDSTR          "Connected"
#define I2CSTARTSTR           "I2C start"
#define I2CDEVICENOTFOUNDSTR  "I2C device !found"
#define I2CDEVICEFOUNDSTR     "I2C device found "
#define MANAGEMENTNOTFOUNDSTR "<html><head><title>Management Server</title></head><body><p>URL was not found</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOMEPAGE\"></form></p></body></html>"
#define WEBSERVERNOTFOUNDSTR  "<html><head><title>myFP2ESP Web Server</title></head><body><p>URL was not found</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOMEPAGE\"></form></p></body></html>"
#define ASCOMSERVERNOTFOUNDSTR  "<html><head><title>ASCOM REMOTE SERVER: Not found</title></head><body><p>FS could not be started</p><p><p><a href=\"/setup/v1/focuser/0/setup\">Setup page</a></p></body></html>";
      
#define WRITEFILEFAILSTR          "file write fail"
#define WRITEFILESUCCESSSTR       "Write to file OK"
#define CREATEFILEFAILSTR         "!create file"
#define CHECKWIFICONFIGFILESTR    "check for Wifi config file"
#define DESERIALIZEERRORSTR       "Deserialization err"
#define SERVERNOTRUNNINGSTR       "Server !running"

// home position switch messages
#define HPCLOSEDFPNOT0STR         "HP closed, fcurrentPosition !=0"
#define HPCLOSEDFP0STR            "HP closed, fcurrentPosition=0"
#define HPMOVETILLOPENSTR         "HP Move out till OPEN"
#define HPMOVEOUTERRORSTR         "HP MoveOUT ERROR: HOMESTEPS exceeded#"
#define HPMOVEOUTSTEPSSTR         "HP MoveOUT stepstaken="
#define HPMOVEOUTFINISHEDSTR      "HP MoveOUT ended"

// temperature probe messages
#define TPROBESTR                 "Tsensors= "
#define TPROBENOTFOUNDSTR         "Tprobe !found"
#define GETTEMPPROBESSTR          "Get # of Tsensors"
#define SETTPROBERESSTR           "Set Tprecision to "

// web page color messages
#define BACKCOLORINVALIDSTR       "Bkcol invalid"
#define NEWTITLECOLORSTR          "Title color "
#define TITLECOLORINVALIDSTR      "Title col invalid"
#define NEWHEADERCOLORSTR         "Header color "
#define HEADERCOLORINVALIDSTR     "Header col invalid"
#define NEWTEXTCOLORSTR           "Text color "
#define TEXTCOLORINVALIDSTR       "Text col invalid"

// oled messages
#define CURRENTPOSSTR             "Current Pos = "
#define TARGETPOSSTR              "Target Pos  = "
#define COILPWRSTR                "Coil power  = "
#define REVDIRSTR                 "Reverse Dir = "
#define STEPMODESTR               "Step Mode   = "
#define TEMPSTR                   "Temperature = "
#define MOTORSPEEDSTR             "Motor Speed = "
#define MAXSTEPSSTR               "MaxSteps    = "
#define TCOMPSTEPSSTR             "TComp Steps = "
#define TCOMPSTATESTR             "TComp State = "
#define TCOMPDIRSTR               "TComp Dir   = "
#define BACKLASHINSTR             "Backlash In = "
#define BACKLASHOUTSTR            "Backlash Out ="
#define BACKLASHINSTEPSSTR        "Backlash In#= "
#define BACKLASHOUTSTEPSSTR       "Backlash Ou#= "
#define BLUETOOTHSTR              "Bluetooth Mode"
#define LOCALSERIALSTR            "Local Serial Mode"

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
#define FILENOTFOUNDSTR           "File !found "
#define FILEFOUNDSTR              "File found"
#define CANNOTCREATEFILESTR       "!create file"

#define FSNOTSTARTEDSTR           "!start FS"
#define BUILDDEFAULTPAGESTR       "build default page"
#define FSFILENOTFOUNDSTR         "file !found"
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

#define MDNSSTARTFAILSTR          "Error starting MDNS responder"
#define MDNSSTARTEDSTR            "mDNS responder started"

#define CREBOOTSTR                "<form action=\"/\" method =\"post\"><input type=\"hidden\" name=\"srestart\" value=\"true\"><input type=\"submit\" value=\"REBOOT CONTROLLER\"></form>"

#define ENABLEBKINSTR             "<form action=\"/msindex3\" method=\"post\"><b>BL-IN State</b> [%STI%]: <input type=\"hidden\" name=\"enin\" value=\"true\"><input type=\"submit\" value=\"ENABLE\"></form>"
#define DISABLEBKINSTR            "<form action=\"/msindex3\" method=\"post\"><b>BL-IN State</b> [%STI%]: <input type=\"hidden\" name=\"diin\" value=\"true\"><input type=\"submit\" value=\"DISABLE\"></form>"
#define ENABLEBKOUTSTR            "<form action=\"/msindex3\" method=\"post\"><b>BL-OUT State</b> [%STO%]: <input type=\"hidden\" name=\"enou\" value=\"true\"><input type=\"submit\" value=\"ENABLE\"></form>"
#define DISABLEBKOUTSTR           "<form action=\"/msindex3\" method=\"post\"><b>BL-OUT State</b> [%STO%]: <input type=\"hidden\" name=\"diou\" value=\"true\"><input type=\"submit\" value=\"DISABLE\"></form>"

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

#define NOTDEFINEDSTR             "!defined in firmware"

// ---------------------------------------------------------------------------
// 2. TRACING - DO NOT CHANGE / DO NOT ENABLE
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// 3. DEBUGGING - DO NOT CHANGE / DO NOT ENABLE
// ---------------------------------------------------------------------------
//#define DEBUG     1

#ifdef  DEBUG                                         //Macros are usually in all capital letters.
#define DebugPrint(...) Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
#define DebugPrintln(...) Serial.println(__VA_ARGS__) //DPRINTLN is a macro, debug print with new line
#else
#define DebugPrint(...)                               //now defines a blank line
#define DebugPrintln(...)                             //now defines a blank line
#endif

// ---------------------------------------------------------------------------
// 4. TIMING TESTS - DO NOT CHANGE / DO NOT ENABLE
// ---------------------------------------------------------------------------
//#define TIMEDTESTS 1
#ifdef TIMEDTESTS

#define TIMESETUP           1
//#define TIMELOOP            1
#define TIMEWSROOTSEND      1
#define TIMEWSROOTHANDLE    1
#define TIMEWSROOTBUILD     1
#define TIMEWSMOVEHANDLE    1
#define TIMEWSBUILDMOVE     1
#define TIMEWSSENDMOVE      1
#define TIMEWSSENDPRESETS   1
#define TIMEWSHANDLEPRESETS 1
#define TIMEWSBUILDPRESETS  1

#define TIMEMSSENDPG1       1
#define TIMEMSSENDPG2       1
#define TIMEMSSENDPG3       1
#define TIMEMSSENDPG4       1
#define TIMEMSSENDPG5       1
#define TIMEMSBUILDPG1      1
#define TIMEMSBUILDPG2      1
#define TIMEMSBUILDPG3      1
#define TIMEMSBUILDPG4      1
#define TIMEMSBUILDPG5      1
#define TIMEMSHANDLEPG1     1
#define TIMEMSHANDLEPG2     1
#define TIMEMSHANDLEPG3     1
#define TIMEMSHANDLEPG4     1
#define TIMEMSHANDLEPG5     1

#define TIMEASCOMBUILDSETUP         1
#define TIMEASCOMHANDLESETUP        1
#define TIMEASCOMHANDLEFOCUSERSETUP 1
#define TIMEASCOMHANDLEAPIVER       1
#define TIMEASCOMHANDLEAPIDES       1
#define TIMEASCOMHANDLEAPICON       1
#endif

#endif // generalDefinitions.h
