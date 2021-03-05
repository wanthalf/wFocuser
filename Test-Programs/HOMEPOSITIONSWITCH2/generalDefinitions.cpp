// ----------------------------------------------------------------------------------------------
// myFP2ESP GENERAL DEFINITIONS Textmessages
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// COPYRIGHT
// ----------------------------------------------------------------------------------------------
// (c) Copyright Robert Brown 2014-2020. All Rights Reserved.
// (c) Copyright Holger M, 2019-2020. All Rights Reserved.
// ----------------------------------------------------------------------------------------------

const char* programVersion        = "131";
const char* ProgramAuthor         = "(c) R BROWN 2020";

const char* STATEMOVINGSTR        = ">Moving";
const char* STATEAPPLYBACKLASH    = ">ApplyBacklash";
const char* STATESETHOMEPOSITION  = ">SetHomePosition";
const char* STATEFINDHOMEPOSITION = ">FindHomePosition#= ";
const char* STATEDELAYAFTERMOVE   = ">DelayAfterMove= ";
const char* STATEFINISHEDMOVE     = ">FinishedMove";
const char* STATEIDLE             = ">Idle";
const char* STATEINITMOVE         = ">InitMove";
const char* PORTSTR               = "Port= ";
const char* SENDSTR               = "Send= ";
const char* SERIALSTARTSTR        = "Serial started";
const char* DEBUGONSTR            = "Debug on";
const char* BLUETOOTHSTARTSTR     = "Bluetooth started";
const char* ATTEMPTCONNSTR        = "Attempt connection to= ";
const char* APSTARTFAILSTR        = "Fail starting AP ";
const char* SERVERREADYSTR        = "Server Ready= ";
const char* STARTSTR              = "Start";
const char* ENDSTR                = "End";
const char* PROGRESSSTR           = "Progress: ";
const char* ERRORSTR              = "Err= ";
const char* READYSTR              = "Ready";
const char* SETUPDRVBRDSTR        = "Setup drvbrd= ";
const char* DRVBRDDONESTR         = "Driver board done";
const char* CHECKCPWRSTR          = "Check coilpower";
const char* CPWRRELEASEDSTR       = "Coil power released";
const char* STARTAPSTR            = "Start Access Point";
const char* STARTSMSTR            = "Start Station mode";
const char* SETSTATICIPSTR        = "Setup Static IP";
const char* ATTEMPTSSTR           = "Attempt= ";
const char* STARTTCPSERVERSTR     = "Start TCP Server";
const char* TCPSERVERSTARTEDSTR   = "TCP Server started";
const char* GETLOCALIPSTR         = "Get local IP address";
const char* SETUPDUCKDNSSTR       = "Setup DuckDNS";
const char* SETUPENDSTR           = "Setup end";
const char* STARTOTASERVICESTR    = "Start OTA service";
const char* SSIDSTR               = "SSID = ";
const char* IPADDRESSSTR          = "IP   = ";
const char* WIFIRESTARTSTR        = "Restarting";
const char* WIFIBEGINSTATUSSTR    = "Wifi.begin status code: ";
const char* CHECKFORTPROBESTR     = "Check for Tprobe";
const char* ACCESSPOINTSTR        = "Access point: ";
const char* STATIONMODESTR        = "Station mode: ";
const char* CONFIGSAVEDSTR        = "new Config saved: ";
const char* RELEASEMOTORSTR       = "Idle: release motor";
const char* LOOPSTARTSTR          = "Loop Start =";
const char* LOOPENDSTR            = "Loop End =";
const char* TCPCLIENTCONNECTSTR   = "tcp client connected";
const char* TCPCLIENTDISCONNECTSTR = "tcp client disconnected";
const char* APCONNECTFAILSTR      = "Did not connect to AP ";
const char* CONNECTEDSTR          = "Connected";
const char* I2CDEVICENOTFOUNDSTR  = "I2C device !found";
const char* ASCOMREMOTESTR        = "ASCOM Remote";
const char* REBOOTWSSTR           = "Rebooting web-server";
const char* REBOOTTCPSTR          = "Rebooting tcp/ip-server";
const char* REBOOTCNTLRSTR        = "Rebooting controller";
const char* REBOOTASCOMSTR        = "Rebooting ASCOM remote server";
const char* ASCOMSERVERNOTDEFINEDSTR = "ASCOM remote server not defined";
const char* WEBSERVERNOTDEFINEDSTR = "Web server not defined";
const char* SETPGOPTIONSTR        = "set OLED pg opt";
const char* SETPGTIMESTR          = "set OLED pg time";

const char* MANAGEMENTURLNOTFOUNDSTR  = "<html><head><title>Management server></title></head><body><p>URL was not found</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOME\"></form></p></body></html>";
const char* WEBSERVERURLNOTFOUNDSTR   = "<html><head><title>Web-server></title></head><body><p>URL was not found</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOME\"></form></p></body></html>";
const char* ASCOMSERVERURLNOTFOUNDSTR = "<html><head><title>ASCOM REMOTE SERVER</title></head><body><p>File not found</p><p><p><a href=\"/setup/v1/focuser/0/setup\">Setup page</a></p></body></html>";

const char* WRITEFILEFAILSTR      = "!write to file";
const char* WRITEFILESUCCESSSTR   = "Write to file OK";
const char* CREATEFILEFAILSTR     = "!create file";
const char* CHECKWIFICONFIGFILESTR = "check for Wifi config file";
const char* DESERIALIZEERRORSTR   = "Deserialization err";
const char* SERVERNOTRUNNINGSTR   = "Server !running";

// temperature probe messages
const char* TPROBESTR             = "Tsensors= ";
const char* TPROBENOTFOUNDSTR     = "Tprobe !found";
const char* GETTEMPPROBESSTR      = "Get #Tsensors";
const char* SETTPROBERESSTR       = "Set Tprecision to ";

// home position switch
const char* HPCLOSEDFPNOT0STR     = "HP Sw=1, Pos !0";
const char* HPCLOSEDFP0STR        = "HP Sw=1, Pos=0";
const char* HPMOVETILLOPENSTR     = "HP Sw=0, Mov out";
const char* HPMOVEOUTERRORSTR     = "HP Sw=0, Mov out err";
const char* HPMOVEOUTFINISHEDSTR  = "HP Sw=0, Mov out ok";
const char* HPMOVEOUTSTEPSSTR     = "HP Steps, Mov out: ";

// oled messages
const char* CURRENTPOSSTR       =  "Current Pos = ";
const char* TARGETPOSSTR        =  "Target Pos  = ";
const char* COILPWRSTR          =  "Coil power  = ";
const char* REVDIRSTR           =  "Reverse Dir = ";
const char* STEPMODESTR         =  "Step Mode   = ";
const char* TEMPSTR             =  "Temperature = ";
const char* MOTORSPEEDSTR       =  "Motor Speed = ";
const char* MAXSTEPSSTR         =  "MaxSteps    = ";
const char* TCOMPSTEPSSTR       =  "TComp Steps = ";
const char* TCOMPSTATESTR       =  "TComp State = ";
const char* TCOMPDIRSTR         =  "TComp Dir   = ";
const char* BACKLASHINSTR       =  "Backlash In = ";
const char* BACKLASHOUTSTR      =  "Backlash Out =";
const char* BACKLASHINSTEPSSTR  =  "Backlash In#= ";
const char* BACKLASHOUTSTEPSSTR =  "Backlash Ou#= ";
const char* BLUETOOTHSTR        =  "Bluetooth Mode";
const char* LOCALSERIALSTR      =  "Local Serial Mode";
const char* FSFILENOTFOUNDSTR   =  "FS file !found";

const char* BACKCOLORINVALIDSTR = "Back color !valid";
const char* NEWTITLECOLORSTR    = "Title color ";
const char* TITLECOLORINVALIDSTR= "Title color !valid";
const char* NEWHEADERCOLORSTR   = "Header color ";
const char* HEADERCOLORINVALIDSTR = "Header color !valid";
const char* NEWTEXTCOLORSTR     = "Text color ";
const char* TEXTCOLORINVALIDSTR = "Text color !valid";
