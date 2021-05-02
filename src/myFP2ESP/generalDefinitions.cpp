// ======================================================================
// generalDefinitions.cpp : myFP2ESP GENERAL DEFINITIONS AND Text messages
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

const char* programVersion            = "222";
const char* ProgramAuthor             = "(c) R BROWN 2020";

const char* MANAGEMENTURLNOTFOUNDSTR  = "<html><head><title>Management server></title></head><body><p>URL was not found</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOME\"></form></p></body></html>";
const char* WEBSERVERURLNOTFOUNDSTR   = "<html><head><title>Web-server></title></head><body><p>URL was not found</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOME\"></form></p></body></html>";
const char* ASCOMSERVERURLNOTFOUNDSTR = "<html><head><title>ASCOM REMOTE SERVER</title></head><body><p>File not found</p><p><p><a href=\"/setup/v1/focuser/0/setup\">Setup page</a></p></body></html>";

// oled messages
const char* CURRENTPOSSTR             = "Current Pos = ";
const char* TARGETPOSSTR              = "Target Pos  = ";
const char* COILPWRSTR                = "Coil power  = ";
const char* REVDIRSTR                 = "Reverse Dir = ";
const char* STEPMODESTR               = "Step Mode   = ";
const char* TEMPSTR                   = "Temperature = ";
const char* MOTORSPEEDSTR             = "Motor Speed = ";
const char* MAXSTEPSSTR               = "MaxSteps    = ";
const char* TCOMPSTEPSSTR             = "TComp Steps = ";
const char* TCOMPSTATESTR             = "TComp State = ";
const char* TCOMPDIRSTR               = "TComp Dir   = ";
const char* BACKLASHINSTR             = "Backlash In = ";
const char* BACKLASHOUTSTR            = "Backlash Out =";
const char* BACKLASHINSTEPSSTR        = "Backlash In#= ";
const char* BACKLASHOUTSTEPSSTR       = "Backlash Ou#= ";
const char* BLUETOOTHSTR              = "Bluetooth Mode";
const char* LOCALSERIALSTR            = "Local Serial Mode";
const char* FSFILENOTFOUNDSTR         = "FS file not found";

const char* BACKCOLORINVALIDSTR       = "err: Back color";
const char* NEWTITLECOLORSTR          = "Title color ";
const char* TITLECOLORINVALIDSTR      = "err: Title color";
const char* NEWHEADERCOLORSTR         = "Header color ";
const char* HEADERCOLORINVALIDSTR     = "err: Header color";
const char* NEWTEXTCOLORSTR           = "Text color ";
const char* TEXTCOLORINVALIDSTR       = "err: Text color";

const char* SETPGOPTIONSTR            = "set OLED pg opt";
const char* SENDPAGESTR               = "Send page";
const char* SETPGTIMESTR              = "set OLED pg time";

const char* CREATEFILEFAILSTR         = "err: file";
const char* WRITEFILEFAILSTR          = "err: write to file";
const char* WRITEFILESUCCESSSTR       = "write file ok";
