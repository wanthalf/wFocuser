// ======================================================================
// ManagementServer.cpp : myFP2ESP MANAGEMENT SERVER ROUTINES
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

#include <Arduino.h>
#include "myBoards.h"
#include "focuserconfig.h"
#include "FocuserSetupData.h"
#include "images.h"
#include "generalDefinitions.h"

#if defined(ESP8266)                        // this "define(ESP8266)" comes from Arduino IDE
#include <FS.h>                             // include the SPIFFS library  
#else                                       // otherwise assume ESP32
#include "SPIFFS.h"
#endif

#ifndef STATICIPON
#define STATICIPON    1
#endif

#include "displays.h"

// ======================================================================
// Extern Data
// ======================================================================
extern SetupData   *mySetupData;
extern DriverBoard *driverboard;

extern OLED_NON *myoled;

#include "temp.h"
extern TempProbe *myTempProbe;

extern volatile bool halt_alert;
extern unsigned long ftargetPosition;
extern byte isMoving;
extern int  packetsreceived;
extern int  packetssent;
extern bool mdnsserverstate;                       // states for services, RUNNING | STOPPED
extern bool webserverstate;
extern bool ascomserverstate;
extern bool ascomdiscoverystate;
extern bool managementserverstate;
extern bool tcpipserverstate;
extern bool otaupdatestate;
extern bool duckdnsstate;
extern bool displaystate;
extern int  staticip;
extern int  tprobe1;

// ======================================================================
// Extern functions
// ======================================================================
extern void start_tcpipserver(void);
extern void stop_tcpipserver(void);
extern void start_webserver(void);
extern void stop_webserver(void);
extern void start_ascomremoteserver(void);
extern void stop_ascomremoteserver(void);
extern void software_Reboot(int);
extern long getrssi(void);
extern bool init_leds(void);
extern bool init_homepositionswitch(void);
extern bool init_pushbuttons(void);

#ifdef MDNSSERVER
extern void start_mdns_service(void);
extern void stop_mdns_service(void);
#endif

// ======================================================================
// Forward Declarations
// ======================================================================
void MANAGEMENT_sendadminpg5(void);
void MANAGEMENT_sendadminpg4(void);
void MANAGEMENT_sendadminpg3(void);
void MANAGEMENT_sendadminpg2(void);
void MANAGEMENT_sendadminpg1(void);

// ======================================================================
// MANAGEMENT INTERFACE - CHANGE AT YOUR OWN PERIL
// ======================================================================
#ifdef MANAGEMENT

#if defined(ESP8266)
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif // if defined(esp8266)

#if defined(ESP8266)
#undef DEBUG_ESP_HTTP_SERVER
ESP8266WebServer mserver(MSSERVERPORT);
#else
WebServer mserver(MSSERVERPORT);
#endif // if defined(esp8266)

String MSpg;
File   fsUploadFile;

boolean ishexdigit( char c )
{
  if ( (c >= '0') && (c <= '9') )                 // is a digit
  {
    return true;
  }
  if ( (c >= 'a') && (c <= 'f') )                 // is a-f
  {
    return true;
  }
  if ( (c >= 'A') && (c <= 'F') )                 // is A-F
  {
    return true;
  }
  return false;
}

// convert the file extension to the MIME type
String MANAGEMENT_getcontenttype(String filename)
{
  String retval = "text/plain";
  if (filename.endsWith(".html"))
  {
    retval = "text/html";
  }
  else if (filename.endsWith(".css"))
  {
    retval = "text/css";
  }
  else if (filename.endsWith(".js"))
  {
    retval = "application/javascript";
  }
  else if (filename.endsWith(".ico"))
  {
    retval = "image/x-icon";
  }
  //retval = "application/octet-stream";
  return retval;
}

void MANAGEMENT_sendmyheader(void)
{
  //mserver.sendHeader(F(CACHECONTROLSTR), F(NOCACHENOSTORESTR));
  //mserver.sendHeader(F(PRAGMASTR), F(NOCACHESTR));
  //mserver.sendHeader(F(EXPIRESSTR), "-1");
  //mserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
  //mserver.send(NORMALWEBPAGE, F(TEXTPAGETYPE), "");
  mserver.client().println("HTTP/1.1 200 OK");
  mserver.client().println("Content-type:text/html");
  //mserver.client().println("Connection: close");       // only valid on http/1.0
  mserver.client().println();
}

void MANAGEMENT_sendmycontent()
{
  mserver.client().print(MSpg);
}

// send the requested file to the client (if it exists)
bool MANAGEMENT_handlefileread(String path)
{
  DebugPrintln("handleFileRead: " + path);
  if (path.endsWith("/"))
  {
    path += "index.html";                               // if a folder is requested, send the index file
  }
  String contentType = MANAGEMENT_getcontenttype(path); // get the MIME type
  if ( SPIFFS.exists(path) )                            // if the file exists
  {
    File file = SPIFFS.open(path, "r");                 // open it
    if ( mySetupData->get_forcedownload() == 1)         // should the file be downloaded or displayed?
    {
      if ( path.indexOf(".html") == -1)
      {
        // if not an html file, force download : html files will be displayed in browser
        mserver.sendHeader("Content-Type", "application/octet-stream");
        mserver.sendHeader("Content-Disposition", "attachment");
      }
    }
    mserver.streamFile(file, contentType);              // and send it to the client
    file.close();                                       // then finish by closing the file
    return true;
  }
  else
  {
    TRACE();
    DebugPrintln(FILENOTFOUNDSTR);
    return false;                                       // if the file doesn't exist, return false
  }
}

void MANAGEMENT_checkreboot(void)
{
  String msg = mserver.arg("srestart");                 // if reboot controller
  if ( msg != "" )
  {
    String WaitPage = "<html><meta http-equiv=refresh content=\"" + String(MSREBOOTPAGEDELAY) + "\"><head><title>Management Server></title></head><body><p>Please wait, controller rebooting.</p></body></html>";
    mserver.send(NORMALWEBPAGE, TEXTPAGETYPE, WaitPage );
    WaitPage = "";
    delay(1000);                                        // wait for page to be sent
    software_Reboot(REBOOTDELAY);
  }
}

void MANAGEMENT_displaydeletepage()
{
  // spiffs was started earlier when server was started so assume it has started
  if ( SPIFFS.exists("/msdelete.html") )                // check for the webpage
  {
    File file = SPIFFS.open("/msdelete.html", "r");     // open it
    DebugPrintln(READPAGESTR);
    MSpg = file.readString();                           // read contents into string
    file.close();
    DebugPrintln(PROCESSPAGESTARTSTR);

    // Web page colors
    String bcol = mySetupData->get_wp_backcolor();
    MSpg.replace("%BKC%", bcol);
    String txtcol = mySetupData->get_wp_textcolor();
    MSpg.replace("%TXC%", txtcol);
    String ticol = mySetupData->get_wp_titlecolor();
    MSpg.replace("%TIC%", ticol);
    String hcol = mySetupData->get_wp_headercolor();
    MSpg.replace("%HEC%", hcol);
    DebugPrintln(PROCESSPAGEENDSTR);
  }
  else
  {
    TRACE();
    DebugPrintln(FSFILENOTFOUNDSTR);
    MSpg = FILENOTFOUNDSTR;
  }
  mserver.send(NORMALWEBPAGE, F(TEXTPAGETYPE), MSpg);
}

void MANAGEMENT_handledeletefile()
{
  String msg;
  String df = mserver.arg("fname");                     // check server arguments, df has filename myoled
  if ( df != "" )                                       // check for file in spiffs
  {
    // spiffs was started earlier when server was started so assume it has started
    //df = "/" + df;
    if ( df[0] != '/')
    {
      df = '/' + df;
    }
    if ( SPIFFS.exists(df))
    {
      if ( SPIFFS.remove(df))
        msg = "The file is deleted: " + df;
      mserver.send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, msg);
    }
    else
    {
      msg = String(FILENOTFOUNDSTR) + df;               // file does not exist
      mserver.send(NOTFOUNDWEBPAGE, PLAINTEXTPAGETYPE, msg);
    }
  }
  else
  {
    msg = "File field was empty";
    mserver.send(BADREQUESTWEBPAGE, PLAINTEXTPAGETYPE, msg);
  }
}

void MANAGEMENT_listFSfiles(void)
{
  // spiffs was started earlier when server was started so assume it has started
  // example code taken from FSBrowser
  String path = "/";
  DebugPrintln("MANAGEMENT_listFSfiles: " + path);
#if defined(ESP8266)
  String output = "{[";
  Dir dir = SPIFFS.openDir("/");
  while (dir.next())
  {
    output += "{" + dir.fileName() + "}, ";
  }
  output += "]}";
  mserver.send(NORMALWEBPAGE, String(JSONTEXTPAGETYPE), output);
#else // ESP32
  File root = SPIFFS.open(path);
  path = String();

  String output = "{[";
  if (root.isDirectory())
  {
    File file = root.openNextFile();
    while (file)
    {
      if (output != "[")
      {
        output += ',';
      }
      output += "{\"type\":\"";
      output += (file.isDirectory()) ? "dir" : "file";
      output += "\",\"name\":\"";
      output += String(file.name()).substring(1);
      output += "\"}";
      file = root.openNextFile();
    }
  }
  output += "]}";
  mserver.send(NORMALWEBPAGE, String(JSONTEXTPAGETYPE), output);
#endif
}

void MANAGEMENT_buildnotfound(void)
{
  // spiffs was started earlier when server was started so assume it has started
  if ( SPIFFS.exists("/msnotfound.html"))               // load page from fs - wsnotfound.html
  {
    // open file for read
    File file = SPIFFS.open("/msnotfound.html", "r");
    // read contents into string
    DebugPrintln(READPAGESTR);
    MSpg = file.readString();
    file.close();
    DebugPrintln(PROCESSPAGESTARTSTR);
    // process for dynamic data
    // Web page colors
    String bcol = mySetupData->get_wp_backcolor();
    MSpg.replace("%BKC%", bcol);
    String txtcol = mySetupData->get_wp_textcolor();
    MSpg.replace("%TXC%", txtcol);
    String ticol = mySetupData->get_wp_titlecolor();
    MSpg.replace("%TIC%", ticol);
    String hcol = mySetupData->get_wp_headercolor();
    MSpg.replace("%HEC%", hcol);
    MSpg.replace("%VER%", String(programVersion));
    MSpg.replace("%NAM%", mySetupData->get_brdname());
    // add code to handle reboot controller
    MSpg.replace("%BT%", String(CREBOOTSTR));
    MSpg.replace("%HEA%", String(ESP.getFreeHeap()));
    DebugPrintln(PROCESSPAGEENDSTR);
  }
  else
  {
    TRACE();
    DebugPrintln(FSFILENOTFOUNDSTR);
    MSpg = MANAGEMENTNOTFOUNDSTR;
  }
  delay(10);                                            // small pause so background tasks can run
}

void MANAGEMENT_handlenotfound(void)
{
  MANAGEMENT_checkreboot();                             // if reboot controller;
  MANAGEMENT_buildnotfound();
  mserver.send(NOTFOUNDWEBPAGE, TEXTPAGETYPE, MSpg);
  MSpg = "";
}

void MANAGEMENT_buildupload(void)
{
  // spiffs was started earlier when server was started so assume it has started
  if ( SPIFFS.exists("/msupload.html"))                 // load page from fs - wsupload.html
  {
    File file = SPIFFS.open("/msupload.html", "r");     // open file for read
    DebugPrintln(READPAGESTR);
    MSpg = file.readString();                           // read contents into string
    file.close();
    DebugPrintln(PROCESSPAGESTARTSTR);
    // process for dynamic data
    String bcol = mySetupData->get_wp_backcolor();
    MSpg.replace("%BKC%", bcol);
    String txtcol = mySetupData->get_wp_textcolor();
    MSpg.replace("%TXC%", txtcol);
    String ticol = mySetupData->get_wp_titlecolor();
    MSpg.replace("%TIC%", ticol);
    String hcol = mySetupData->get_wp_headercolor();
    MSpg.replace("%HEC%", hcol);
    MSpg.replace("%VER%", String(programVersion));
    MSpg.replace("%NAM%", mySetupData->get_brdname());
    // add code to handle reboot controller
    MSpg.replace("%BT%", String(CREBOOTSTR));
    DebugPrintln(PROCESSPAGEENDSTR);
  }
  else
  {
    TRACE();
    DebugPrintln(FSFILENOTFOUNDSTR);
    MSpg = MANAGEMENTNOTFOUNDSTR;
  }
  delay(10);                                            // small pause so background tasks can run
}

void MANAGEMENT_displayfileupload(void)
{
  MANAGEMENT_buildupload();
  mserver.send(NORMALWEBPAGE, TEXTPAGETYPE, MSpg);
  MSpg = "";
}

void MANAGEMENT_handlefileupload(void)
{
  HTTPUpload& upload = mserver.upload();
  if (upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    if (!filename.startsWith("/"))
    {
      filename = "/" + filename;
    }
    DebugPrint("handleFileUpload Name: ");
    DebugPrintln(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    if (fsUploadFile)
    {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (fsUploadFile)
    {
      // If the file was successfully created
      fsUploadFile.close();
      DebugPrint("handleFileUpload Size: ");
      DebugPrintln(upload.totalSize);
      mserver.sendHeader("Location", "/mssuccess.html");
      mserver.send(301);
    }
    else
    {
      mserver.send(INTERNALSERVERERROR, String(PLAINTEXTPAGETYPE), String(CANNOTCREATEFILESTR));
    }
  }
}

void MANAGEMENT_buildadminpg5(void)
{
#ifdef TIMEMSBUILDPG5
  Serial.print("ms_buildpg5: ");
  Serial.println(millis());
#endif
  if ( SPIFFS.exists("/msindex5.html"))                 // constructs admin page 5 of management server
  {
    DebugPrintln(FILEFOUNDSTR);
    File file = SPIFFS.open("/msindex5.html", "r");     // open file for read
    DebugPrintln(READPAGESTR);
    MSpg = file.readString();                           // read contents into string
    file.close();
    DebugPrintln(PROCESSPAGESTARTSTR);
    // process for dynamic data
    String bcol = mySetupData->get_wp_backcolor();
    MSpg.replace("%BKC%", bcol);
    String txtcol = mySetupData->get_wp_textcolor();
    MSpg.replace("%TXC%", txtcol);
    String ticol = mySetupData->get_wp_titlecolor();
    MSpg.replace("%TIC%", ticol);
    String hcol = mySetupData->get_wp_headercolor();
    MSpg.replace("%HEC%", hcol);
    MSpg.replace("%VER%", String(programVersion));
    MSpg.replace("%NAM%", mySetupData->get_brdname());

    // Display image for color picker

    MSpg.replace("%BT%", String(CREBOOTSTR));           // add code to handle reboot controller

    // display heap memory for tracking memory loss?
    // only esp32?
    MSpg.replace("%HEA%", String(ESP.getFreeHeap()));
    DebugPrintln(PROCESSPAGEENDSTR);
  }
  else
  {
    // could not read file
    TRACE();
    DebugPrintln(BUILDDEFAULTPAGESTR);
    MSpg = MANAGEMENTNOTFOUNDSTR;
  }
#ifdef TIMEMSBUILDPG5
  Serial.print("ms_buildpg5: ");
  Serial.println(millis());
#endif
  delay(10);                                            // small pause so background tasks can run
}

void MANAGEMENT_handleadminpg5(void)
{
#ifdef TIMEMSHANDLEPG5
  Serial.print("ms_handlepg5: ");
  Serial.println(millis());
#endif
  MANAGEMENT_checkreboot();                             // if reboot controller;

  // there are no parameters to check

  MANAGEMENT_sendadminpg5();
#ifdef TIMEMSHANDLEPG5
  Serial.print("ms_handlepg5: ");
  Serial.println(millis());
#endif
}

void MANAGEMENT_buildadminpg4(void)
{
#ifdef TIMEMSBUILDPG4
  Serial.print("ms_buildpg4: ");
  Serial.println(millis());
#endif
  if ( SPIFFS.exists("/msindex4.html"))                 // constructs admin page 4 of management server
  {
    DebugPrintln(FILEFOUNDSTR);
    File file = SPIFFS.open("/msindex4.html", "r");     // open file for read
    DebugPrintln(READPAGESTR);
    MSpg = file.readString();                           // read contents into string
    file.close();
    DebugPrintln(PROCESSPAGESTARTSTR);
    // process for dynamic data
    String bcol = mySetupData->get_wp_backcolor();
    MSpg.replace("%BKC%", bcol);
    String txtcol = mySetupData->get_wp_textcolor();
    MSpg.replace("%TXC%", txtcol);
    String ticol = mySetupData->get_wp_titlecolor();
    MSpg.replace("%TIC%", ticol);
    String hcol = mySetupData->get_wp_headercolor();
    MSpg.replace("%HEC%", hcol);
    MSpg.replace("%VER%", String(programVersion));
    MSpg.replace("%NAM%", mySetupData->get_brdname());

    // Background color %BC% default #333333
    // Main Page Title Color %MP% default #8e44ad
    // Header Group Color %HC% default #3399ff
    // Text Color %TC% default #5d6d7e
    String bcolor = "<form action=\"/msindex4\" method=\"post\"><input type=\"text\" name=\"bc\" size=\"6\" value=\"" + String(mySetupData->get_wp_backcolor()) + "\"><input type=\"submit\" name=\"bc\" value=\"SET\"></form>";
    MSpg.replace("%BC%", String(bcolor));
    String ticolor = "<form action=\"/msindex4\" method=\"post\"><input type=\"text\" name=\"ti\" size=\"6\" value=\"" + String(mySetupData->get_wp_titlecolor()) + "\"><input type=\"submit\" name=\"ti\" value=\"SET\"></form>";
    MSpg.replace("%TI%", String(ticolor));
    String hcolor = "<form action=\"/msindex4\" method=\"post\"><input type=\"text\" name=\"hc\" size=\"6\" value=\"" + String(mySetupData->get_wp_headercolor()) + "\"><input type=\"submit\" name=\"hc\" value=\"SET\"></form>";
    MSpg.replace("%HC%", String(hcolor));
    String tcolor = "<form action=\"/msindex4\" method=\"post\"><input type=\"text\" name=\"tc\" size=\"6\" value=\"" + String(mySetupData->get_wp_textcolor()) + "\"><input type=\"submit\" name=\"tc\" value=\"SET\"></form>";
    MSpg.replace("%TC%", String(tcolor));

    MSpg.replace("%BT%", String(CREBOOTSTR));           // add code to handle reboot controller

    TRACE();
    DebugPrintln(PROCESSPAGEENDSTR);

    // display heap memory for tracking memory loss?
    // only esp32?
    MSpg.replace("%HEA%", String(ESP.getFreeHeap()));
    DebugPrintln(PROCESSPAGEENDSTR);
  }
  else
  {
    // could not read index file from SPIFFS
    TRACE();
    DebugPrintln(BUILDDEFAULTPAGESTR);
    MSpg = MANAGEMENTNOTFOUNDSTR;
  }
#ifdef TIMEMSBUILDPG4
  Serial.print("ms_buildpg4: ");
  Serial.println(millis());
#endif
  delay(10);                                            // small pause so background tasks can run
}

void MANAGEMENT_handleadminpg4(void)
{
#ifdef TIMEMSHANDLEPG4
  Serial.print("ms_handlepg4: ");
  Serial.println(millis());
#endif
  // code here to handle a put request
  String msg;

  MANAGEMENT_checkreboot();                             // if reboot controller;

  // Handle update of webpage colors
  // Main Page Title Color <span id="TI">%MP%</span></p> #8e44ad
  // <p>Background Color <span id="BC">%BC%</span></p> #333333
  // <p>Header Group Color ><span id="HC">%HC%</span></p> #3399ff
  // <p>Text Color ><span id="TC">%TC%</span></p> #5d6d7e
  // get background color
  msg = mserver.arg("bc");
  if ( msg != "" )
  {
    boolean flag = true;
    String str = msg;
    int len = str.length();
    for ( int i = 0; i < len; i++ )
    {
      char ch = str[i];
      if ( ishexdigit(ch) == false )
      {
        flag = false;
      }
    }
    if ( flag == false )
    {
      DebugPrintln(BACKCOLORINVALIDSTR);
    }
    else
    {
      mySetupData->set_wp_backcolor(str);               // set the new background color
    }
  }

  // get text color
  msg = mserver.arg("tc");
  if ( msg != "" )
  {
    boolean flag = true;
    String str = msg;
    int len = str.length();
    for ( int i = 0; i < len; i++ )
    {
      char ch = str[i];
      if ( ishexdigit(ch) == false )
      {
        flag = false;
      }
    }
    if ( flag == false )
    {
      DebugPrintln(TEXTCOLORINVALIDSTR);
    }
    else
    {
      DebugPrint(NEWTEXTCOLORSTR);
      DebugPrintln(str);
      mySetupData->set_wp_textcolor(str);               // set the new text color
    }
  }

  // get header color
  msg = mserver.arg("hc");
  if ( msg != "" )
  {
    boolean flag = true;
    String str = msg;
    int len = str.length();
    for ( int i = 0; i < len; i++ )
    {
      char ch = str[i];
      if ( ishexdigit(ch) == false )
      {
        flag = false;
      }
    }
    if ( flag == false )
    {
      DebugPrintln(HEADERCOLORINVALIDSTR);
    }
    else
    {
      DebugPrint(NEWHEADERCOLORSTR);
      DebugPrintln(str);
      mySetupData->set_wp_headercolor(str);             // set the new header color
    }
  }

  // get page title color
  msg = mserver.arg("ti");
  if ( msg != "" )
  {
    boolean flag = true;
    String str = msg;
    int len = str.length();
    for ( int i = 0; i < len; i++ )
    {
      char ch = str[i];
      if ( ishexdigit(ch) == false )
      {
        flag = false;
      }
    }
    if ( flag == false )
    {
      DebugPrintln(TITLECOLORINVALIDSTR);
    }
    else
    {
      DebugPrint(NEWTITLECOLORSTR);
      DebugPrintln(str);
      mySetupData->set_wp_titlecolor(str);              // set the new header color
    }
  }
  MANAGEMENT_sendadminpg4();
#ifdef TIMEMSHANDLEPG4
  Serial.print("ms_handlepg4: ");
  Serial.println(millis());
#endif
}

void MANAGEMENT_buildadminpg3(void)
{
#ifdef TIMEMSBUILDPG3
  Serial.print("ms_buildpg3: ");
  Serial.println(millis());
#endif
  // spiffs was started earlier when server was started so assume it has started
  if ( SPIFFS.exists("/msindex3.html"))                 // constructs admin page 3 of management server
  {
    DebugPrintln(FILEFOUNDSTR);
    File file = SPIFFS.open("/msindex3.html", "r");     // open file for read
    DebugPrintln(READPAGESTR);
    MSpg = file.readString();                           // read contents into string
    file.close();
    DebugPrintln(PROCESSPAGESTARTSTR);
    // process for dynamic data
    String bcol = mySetupData->get_wp_backcolor();
    MSpg.replace("%BKC%", bcol);
    String txtcol = mySetupData->get_wp_textcolor();
    MSpg.replace("%TXC%", txtcol);
    String ticol = mySetupData->get_wp_titlecolor();
    MSpg.replace("%TIC%", ticol);
    String hcol = mySetupData->get_wp_headercolor();
    MSpg.replace("%HEC%", hcol);
    MSpg.replace("%VER%", String(programVersion));
    MSpg.replace("%NAM%", mySetupData->get_brdname());

    if ( mySetupData->get_backlash_in_enabled() )
    {
      MSpg.replace("%BIE%", String(DISABLEBKINSTR));
      MSpg.replace("%STI%", String(ENABLEDSTR));
    }
    else
    {
      MSpg.replace("%BIE%", String(ENABLEBKINSTR));
      MSpg.replace("%STI%", String(NOTENABLEDSTR));
    }
    if ( mySetupData->get_backlash_out_enabled() )
    {
      MSpg.replace("%BOE%", String(DISABLEBKOUTSTR));
      MSpg.replace("%STO%", String(ENABLEDSTR));
    }
    else
    {
      MSpg.replace("%BOE%", String(ENABLEBKOUTSTR));
      MSpg.replace("%STO%", String(NOTENABLEDSTR));
    }

    MSpg.replace("%BIS%", String(BLINSTEPSTR));
    MSpg.replace("%BOS%", String(BLOUTSTEPSTR));
    MSpg.replace("%bins%", String(mySetupData->get_backlashsteps_in()));
    MSpg.replace("%bous%", String(mySetupData->get_backlashsteps_out()));

    // motor speed delay
    MSpg.replace("%MS%", "<form action=\"/msindex3\" method=\"post\">Delay: <input type=\"text\" name=\"msd\" size=\"6\" value=" + String(mySetupData->get_brdmsdelay()) + "> <input type=\"submit\" name=\"setmsd\" value=\"Set\"></form>");

    MSpg.replace("%BT%", String(CREBOOTSTR));           // add code to handle reboot controller

    // PUSHBUTTONS
    if ( mySetupData->get_pbenable() == 1)
    {
      MSpg.replace("%PBN%", String(DISABLEPBSTR));      // button
      MSpg.replace("%PBL%", String(ENABLEDSTR));        // state
    }
    else
    {
      MSpg.replace("%PBN%", String(ENABLEPBSTR));
      MSpg.replace("%PBL%", String(NOTENABLEDSTR));
    }

    DebugPrintln(PROCESSPAGEENDSTR);

    // display heap memory for tracking memory loss?
    // only esp32?
    MSpg.replace("%HEA%", String(ESP.getFreeHeap()));
    DebugPrintln(PROCESSPAGEENDSTR);
  }
  else
  {
    // could not read file
    TRACE();
    DebugPrintln(BUILDDEFAULTPAGESTR);
    MSpg = MANAGEMENTNOTFOUNDSTR;
  }
#ifdef TIMEMSBUILDPG3
  Serial.print("ms_buildpg3: ");
  Serial.println(millis());
#endif
  delay(10);                                            // small pause so background tasks can run
}

void MANAGEMENT_handleadminpg3(void)
{
#ifdef TIMEMSHANDLEPG3
  Serial.print("ms_handlepg3: ");
  Serial.println(millis());
#endif
  // code here to handle a put request
  String msg;

  MANAGEMENT_checkreboot();                             // if reboot controller;

  // handle backlash
  // backlash in enable/disable, enin, diin
  msg = mserver.arg("enin");
  if ( msg != "" )
  {
    DebugPrintln("adminpg3: enin: ");
    mySetupData->set_backlash_in_enabled(1);
  }
  msg = mserver.arg("diin");
  if ( msg != "" )
  {
    DebugPrintln("adminpg3: diin: ");
    mySetupData->set_backlash_in_enabled(0);
  }

  // backlash out enable/disable, enou, diou
  msg = mserver.arg("enou");
  if ( msg != "" )
  {
    DebugPrintln("adminpg3: enou: ");
    mySetupData->set_backlash_out_enabled(1);
  }
  msg = mserver.arg("diou");
  if ( msg != "" )
  {
    DebugPrintln("adminpg3: diou: ");
    mySetupData->set_backlash_out_enabled(0);
  }

  // backlash in steps, setbis, bis,
  msg = mserver.arg("setbis");
  if ( msg != "" )
  {
    DebugPrintln("adminpg3: setbis: ");
    String st = mserver.arg("bis");
    DebugPrint("adminpg3: bis: ");
    DebugPrintln(st);
    byte steps = st.toInt();                            // no need to test for <0 and > 255 as it is a byte value
    mySetupData->set_backlashsteps_in(steps);
  }

  // backlash out steps, setbos, bos
  msg = mserver.arg("setbos");
  if ( msg != "" )
  {
    DebugPrintln("adminpg3: setbos: ");
    String st = mserver.arg("bos");
    DebugPrint("adminpg3: bos: ");
    DebugPrintln(st);
    byte steps = st.toInt();
    mySetupData->set_backlashsteps_out(steps);
  }

  // motor speed delay
  msg = mserver.arg("setmsd");
  if ( msg != "" )
  {
    DebugPrint("set motor speed delay: ");
    DebugPrintln(msg);
    String ms = mserver.arg("msd");                      // process new motor speed delay
    if ( ms != "" )
    {
      unsigned long newdelay = 4000;
      newdelay = ms.toInt();
      newdelay = (newdelay < 1000) ? 1000 : newdelay;   // ensure it is not too low
      mySetupData->set_brdmsdelay(newdelay);
    }
  }

  // push buttons enable/disable
  msg = mserver.arg("pbon");
  if ( msg != "" )
  {
    if ( (mySetupData->get_brdpb1pin() == -1) && (mySetupData->get_brdpb1pin() == -1) )
    {
      DebugPrintln("push button pins are -1. Cannot enable pins");
      mySetupData->set_pbenable(0);
    }
    else
    {
      // if current state is disabled then enable
      if ( mySetupData->get_pbenable() == 0)
      {
        mySetupData->set_pbenable(1);
        init_pushbuttons();
      }
    }
  }
  msg = mserver.arg("pboff");
  if ( msg != "" )
  {
    mySetupData->set_pbenable(0);
  }

  MANAGEMENT_sendadminpg3();
#ifdef TIMEMSHANDLEPG3
  Serial.print("ms_handlepg3: ");
  Serial.println(millis());
#endif
}

void MANAGEMENT_buildadminpg2(void)
{
#ifdef TIMEMSBUILDPG2
  Serial.print("ms_buildpg2: ");
  Serial.println(millis());
#endif
  // spiffs was started earlier when server was started so assume it has started
  DebugPrintln("management: FS mounted");               // constructs admin page 2 of management server
  if ( SPIFFS.exists("/msindex2.html"))
  {
    DebugPrintln(FILEFOUNDSTR);
    File file = SPIFFS.open("/msindex2.html", "r");     // open file for read
    DebugPrintln(READPAGESTR);
    MSpg = file.readString();                           // read contents into string
    file.close();

    DebugPrintln(PROCESSPAGESTARTSTR);
    // process for dynamic data
    String bcol = mySetupData->get_wp_backcolor();
    MSpg.replace("%BKC%", bcol);
    String txtcol = mySetupData->get_wp_textcolor();
    MSpg.replace("%TXC%", txtcol);
    String ticol = mySetupData->get_wp_titlecolor();
    MSpg.replace("%TIC%", ticol);
    String hcol = mySetupData->get_wp_headercolor();
    MSpg.replace("%HEC%", hcol);
    MSpg.replace("%VER%", String(programVersion));
    MSpg.replace("%NAM%", mySetupData->get_brdname());

    // tcp/ip server
#if defined(ACCESSPOINT) || defined(STATIONMODE)
    if ( tcpipserverstate == RUNNING )
    {
      MSpg.replace("%TBT%", String(STOPTSSTR));
      MSpg.replace("%TST%", SERVERSTATERUNSTR);
    }
    else
    {
      MSpg.replace("%TBT%", String(STARTTSSTR));
      MSpg.replace("%TST%", SERVERSTATESTOPSTR);
    }
#if defined(ESP8266)
    // esp8266 cannot change port of server
    String portstr = "Port: " + String(mySetupData->get_tcpipport());
    MSpg.replace("%TPO%", portstr );
#else
    // esp32
    MSpg.replace("%TPO%", "<form action=\"/msindex2\" method =\"post\">Port: <input type=\"text\" name=\"tp\" size=\"6\" value=" + String(mySetupData->get_tcpipport()) + "> <input type=\"submit\" name=\"settsport\" value=\"Set\"></form>");
#endif // #if defined(ESP8266)
#else
    MSpg.replace("%TPO%", "Port: " + String(mySetupData->get_tcpipport()));
    MSpg.replace("%TBT%", "Not defined");
#endif // #if defined(ACCESSPOINT) || defined(STATIONMODE)

    // Webserver status %WST%
    if ( webserverstate == RUNNING )
    {
      MSpg.replace("%WBT%", String(STOPWSSTR));
      MSpg.replace("%WST%", String(SERVERSTATERUNSTR));
    }
    else
    {
      MSpg.replace("%WBT%", String(STARTWSSTR));
      MSpg.replace("%WST%", String(SERVERSTATESTOPSTR));
    }
    // Webserver Port number %WPO%, %WBT%refresh Rate %WRA%
    MSpg.replace("%WPO%", "<form action=\"/msindex2\" method =\"post\">Port: <input type=\"text\" name=\"wp\" size=\"6\" value=" + String(mySetupData->get_webserverport()) + "> <input type=\"submit\" name=\"setwsport\" value=\"Set\"></form>");
    MSpg.replace("%WRA%", "<form action=\"/msindex2\" method =\"post\">Refresh Rate: <input type=\"text\" name=\"wr\" size=\"6\" value=" + String(mySetupData->get_webpagerefreshrate()) + "> <input type=\"submit\" name=\"setwsrate\" value=\"Set\"></form>");

    // ascom server start/stop service, Status %AST%, Port %APO%, Button %ABT%
    if ( ascomserverstate == RUNNING )
    {
      MSpg.replace("%AST%", String(STOPASSTR));
      MSpg.replace("%ABT%", String(SERVERSTATERUNSTR));
    }
    else
    {
      MSpg.replace("%AST%", String(STARTASSTR));
      MSpg.replace("%ABT%", String(SERVERSTATESTOPSTR));
    }
    MSpg.replace("%APO%", "<form action=\"/msindex2\" method =\"post\">Port: <input type=\"text\" name=\"ap\" size=\"8\" value=" + String(mySetupData->get_ascomalpacaport()) + "> <input type=\"submit\" name=\"setasport\" value=\"Set\"></form>");

    // TEMPERATURE PROBE ENABLE/DISABLE, State %TPE%, Button %TPO%
    if ( mySetupData->get_temperatureprobestate() == 1 )
    {
      MSpg.replace("%TPP%", String(DISABLETEMPSTR));    // button
      MSpg.replace("%TPE%", String(ENABLEDSTR));        // state
    }
    else
    {
      MSpg.replace("%TPP%", String(ENABLETEMPSTR));
      MSpg.replace("%TPE%", String(NOTENABLEDSTR));
    }
    // Temperature Mode %TEM%
    // Celcius=1, Fahrenheit=0
    if ( mySetupData->get_tempmode() == 1 )
    {
      // celsius - Change to Fahrenheit
      MSpg.replace("%TEM%", String(DISPLAYFSTR));
    }
    else
    {
      // Fahrenheit - change to celsius
      MSpg.replace("%TEM%", String(DISPLAYCSTR));
    }

    // INOUT LEDS ENABLE/DISABLE, State %INL%, Button %INO%
    if ( mySetupData->get_inoutledstate() == 1)
    {
      MSpg.replace("%INO%", String(DISABLELEDSTR));     // button
      MSpg.replace("%INL%", String(ENABLEDSTR));        // state
    }
    else
    {
      MSpg.replace("%INO%", String(ENABLELEDSTR));
      MSpg.replace("%INL%", String(NOTENABLEDSTR));
    }

    if ( mySetupData->get_hpswitchenable() == 1)
    {
      MSpg.replace("%HPO%", String(DISABLEHPSWSTR));   // button
      MSpg.replace("%HPL%", String(ENABLEDSTR));       // state
    }
    else
    {
      MSpg.replace("%HPO%", String(ENABLEHPSWSTR));
      MSpg.replace("%HPL%", String(NOTENABLEDSTR));
    }

    // add code to handle reboot controller %BT%
    MSpg.replace("%BT%", String(CREBOOTSTR));

    // display heap memory for tracking memory loss, %HEA%
    // only esp32?
    MSpg.replace("%HEA%", String(ESP.getFreeHeap()));
    DebugPrintln(PROCESSPAGEENDSTR);
  }
  else
  {
    // could not read file
    TRACE();
    DebugPrintln(BUILDDEFAULTPAGESTR);
    MSpg = MANAGEMENTNOTFOUNDSTR;
  }
#ifdef TIMEMSBUILDPG2
  Serial.print("ms_buildpg2: ");
  Serial.println(millis());
#endif
}

void MANAGEMENT_handleadminpg2(void)
{
#ifdef TIMEMSHANDLEPG2
  Serial.print("ms_handlepg2: ");
  Serial.println(millis());
#endif
  // code here to handle a put request
  String msg;

  MANAGEMENT_checkreboot();                             // if reboot controller;

  // TCP/IP server START STOP
  msg = mserver.arg("startts");
  if ( msg != "" )
  {
    DebugPrintln("adminpg2: startts: ");
#if defined(ACCESSPOINT) || defined(STATIONMODE)
    start_tcpipserver();
#endif
  }
  msg = mserver.arg("stopts");
  if ( msg != "" )
  {
    DebugPrintln("adminpg2: stopts: ");
#if defined(ACCESSPOINT) || defined(STATIONMODE)
    stop_tcpipserver();
#endif
  }
  // tcpip server change port
  msg = mserver.arg("settsport");
  if ( msg != "" )
  {
    DebugPrint("set tcpip server port: ");
    DebugPrintln(msg);
    String tp = mserver.arg("tp");                      // process new webserver port number
    if ( tp != "" )
    {
      unsigned long newport = 0;
      DebugPrint("tp:");
      DebugPrintln(tp);
      newport = tp.toInt();
      if ( tcpipserverstate == STOPPED )
      {
        unsigned long currentport = mySetupData->get_tcpipport();
        if ( newport == currentport)
        {
          // port is the same so do not bother to change it
          DebugPrintln("tp err: new Port = current port");
        }
        else
        {
          if ( newport == MSSERVERPORT )                              // if same as management server
          {
            DebugPrintln("wp err: new Port = MSSERVERPORT");
          }
          else if ( newport == mySetupData->get_ascomalpacaport() )   // if same as ASCOM REMOTE server
          {
            DebugPrintln("wp err: new Port = ALPACAPORT");
          }
          else if ( newport == mySetupData->get_mdnsport() )          // if same as mDNS server
          {
            DebugPrintln("wp err: new Port = MDNSSERVERPORT");
          }
          else if ( newport == mySetupData->get_webserverport() )     // if same as mDNS server
          {
            DebugPrintln("wp err: new Port = WEBSERVERPORT");
          }
          else
          {
            DebugPrintln("New tcpipserver port = " + String(newport));
            mySetupData->set_tcpipport(newport);                      // assign new port and save it
          }
        }
      }
      else
      {
        DebugPrintln("Attempt to change tcpipserver port when tcpipserver running");
      }
    }
  }

  // webserver START STOP service
  msg = mserver.arg("startws");
  if ( msg != "" )
  {
    DebugPrintln("adminpg2: startws: ");
    if ( webserverstate == STOPPED)
    {
      start_webserver();
      mySetupData->set_webserverstate(1);
    }
  }
  msg = mserver.arg("stopws");
  if ( msg != "" )
  {
    DebugPrintln("adminpg2: stopws: ");
    if ( webserverstate == RUNNING )
    {
      stop_webserver();
      mySetupData->set_webserverstate(0);
    }
  }
  // webserver change port - we should be able to change port if not running or enabled or not enabled
  msg = mserver.arg("setwsport");
  if ( msg != "" )
  {
    DebugPrint("set web server port: ");
    DebugPrintln(msg);
    String wp = mserver.arg("wp");                                    // process new webserver port number
    if ( wp != "" )
    {
      unsigned long newport = 0;
      DebugPrint("wp:");
      DebugPrintln(wp);
      newport = wp.toInt();
      if ( webserverstate == STOPPED )
      {
        unsigned long currentport = mySetupData->get_webserverport();
        if ( newport == currentport)
        {
          DebugPrintln("wp err: new Port = current port");            // port is the same so do not bother to change it
        }
        else
        {
          if ( newport == MSSERVERPORT )                              // if same as management server
          {
            DebugPrintln("wp err: new Port = MSSERVERPORT");
          }
          else if ( newport == mySetupData->get_ascomalpacaport() )   // if same as ASCOM REMOTE server
          {
            DebugPrintln("wp err: new Port = ALPACAPORT");
          }
          else if ( newport == mySetupData->get_mdnsport() )          // if same as mDNS server
          {
            DebugPrintln("wp err: new Port = MDNSSERVERPORT");
          }
          else if ( newport == mySetupData->get_tcpipport() )         // if same as tcpip server
          {
            DebugPrintln("wp err: new Port = SERVERPORT");
          }
          else
          {
            DebugPrintln("New webserver port = " + String(newport));
            mySetupData->set_webserverport(newport);                  // assign new port and save it
          }
        }
      }
      else
      {
        DebugPrintln("Attempt to change webserver port when webserver running");
      }
    }
  }
  // web page refresh rate - should be able to change at any time
  msg = mserver.arg("setwsrate");
  if ( msg != "" )
  {
    DebugPrint("set web server page refresh rate: ");
    DebugPrintln(msg);
    String wr = mserver.arg("wr");                      // process new webserver page refresh rate
    if ( wr != "" )
    {
      int newrate = 0;
      DebugPrint("wr:");
      DebugPrintln(wr);
      newrate = wr.toInt();
      int currentrate = mySetupData->get_webpagerefreshrate();
      if ( newrate == currentrate)
      {
        // port is the same so do not bother to change it
        DebugPrintln("wr err: new page refresh rate = current page refresh rate");
      }
      else
      {
        if ( newrate < MINREFRESHPAGERATE )
        {
          DebugPrintln("wr err: Page refresh rate too low");
          newrate = MINREFRESHPAGERATE;
        }
        else if ( newrate > MAXREFRESHPAGERATE )
        {
          DebugPrintln("wr err: Page refresh rate too high");
          newrate = MAXREFRESHPAGERATE;
        }
        DebugPrintln("New page refresh rate = " + String(newrate));
        mySetupData->set_webpagerefreshrate(newrate);                // assign new refresh rate and save it
      }
    }
  }

  // ascomserver start/stop service
  msg = mserver.arg("startas");
  if ( msg != "" )
  {
    DebugPrintln("adminpg2: startas: ");
    if ( ascomserverstate == STOPPED )
    {
      start_ascomremoteserver();
      mySetupData->set_ascomserverstate(1);
    }
  }
  msg = mserver.arg("stopas");
  if ( msg != "" )
  {
    DebugPrintln("adminpg2: stopas: ");
    if ( ascomserverstate == RUNNING)
    {
      stop_ascomremoteserver();
      mySetupData->set_ascomserverstate(0);
    }
  }
  // ascom server port
  msg = mserver.arg("setasport");
  if ( msg != "" )
  {
    DebugPrint("set ascom server port: ");
    DebugPrintln(msg);
    String ap = mserver.arg("ap");                                    // process new ascomalpaca port number
    if ( ap != "" )
    {
      unsigned long newport = 0;
      DebugPrint("ap:");
      DebugPrintln(ap);
      newport = ap.toInt();
      if ( ascomserverstate == STOPPED )
      {
        unsigned long currentport = mySetupData->get_ascomalpacaport();
        if ( newport == currentport)
        {
          // port is the same so do not bother to change it
          DebugPrintln("ap error: new Port = current port");
        }
        else
        {
          if ( newport == MSSERVERPORT )                              // if same as management server
          {
            DebugPrintln("wp err: new Port = MSSERVERPORT");
          }
          else if ( newport == mySetupData->get_webserverport() )     // if same as webserver
          {
            DebugPrintln("wp err: new Port = ALPACAPORT");
          }
          else if ( newport == mySetupData->get_mdnsport() )          // if same as mDNS server
          {
            DebugPrintln("wp err: new Port = MDNSSERVERPORT");
          }
          else if ( newport == mySetupData->get_tcpipport() )          // if same as tcpip server
          {
            DebugPrintln("wp err: new Port = SERVERPORT");
          }
          else
          {
            DebugPrintln("New ascomalpaca port = " + String(newport));
            mySetupData->set_ascomalpacaport(newport);                  // assign new port and save it
          }
        }
      }
      else
      {
        DebugPrintln("Attempt to change ascomalpaca port when ascomserver running");
      }
    }
  }

  // Temperature Probe ENABLE/DISABLE, starttp, stoptp
  msg = mserver.arg("starttp");
  if ( msg != "" )
  {
    if ( mySetupData->get_brdtemppin() == -1)
    {
      DebugPrintln("temp pin is -1. Cannot enable pin");
      mySetupData->set_temperatureprobestate(0);
      tprobe1 = 0;
    }
    else
    {
      // check if already enabled
      if (mySetupData->get_temperatureprobestate() == 1)                // if probe is enabled
      {
        if ( tprobe1 == 0 )                                             // if probe not found
        {
          myTempProbe = new TempProbe;                                  // attempt to start probe and search for probe
        }
      }
      else
      {
        mySetupData->set_temperatureprobestate(1);                      // enable probe
        if ( tprobe1 == 0 )                                             // if probe not found
        {
          myTempProbe = new TempProbe;                                  // attempt to start probe and search for probe
        }
      }
    }
  }
  msg = mserver.arg("stoptp");
  if ( msg != "" )
  {
    if ( mySetupData->get_brdtemppin() == -1)
    {
      DebugPrintln("temp pin is -1. Cannot enable pin");
      mySetupData->set_temperatureprobestate(0);
      tprobe1 = 0;
    }
    else
    {
      if ( mySetupData->get_temperatureprobestate() == 1 )
      {
        // there is no destructor call
        tprobe1 = 0;
        mySetupData->set_temperatureprobestate(0);
      }
      else
      {
        // do nothing, already disabled
        tprobe1 = 0;
      }
    }
  }

  // Temperature probe celsius/farentheit
  msg = mserver.arg("tm");
  if ( msg != "" )
  {
    DebugPrint("Set temp mode: ");
    DebugPrintln(msg);
    if ( msg == "cel" )
    {
      mySetupData->set_tempmode(1);
    }
    else if ( msg == "fah" )
    {
      mySetupData->set_tempmode(0);
    }
  }

  // LEDS ENABLE/DISABLE startle, stople
  msg = mserver.arg("startle");
  if ( msg != "" )
  {
    if ( (mySetupData->get_brdinledpin() == -1) || (mySetupData->get_brdinledpin() == -1) )
    {
      DebugPrintln("led pins are -1. Cannot enable pins");
      mySetupData->set_inoutledstate(0);
    }
    else
    {
      // if disabled then enable
      if ( mySetupData->get_inoutledstate() == 0)
      {
        mySetupData->set_inoutledstate(1);
        // reinitialise pins
        String drvbrd = mySetupData->get_brdname();
        if (drvbrd.equals("PRO2ESP32ULN2003") || drvbrd.equals("PRO2ESP32L298N") || drvbrd.equals("PRO2ESP32L293DMINI") || drvbrd.equals("PRO2ESP32L9110S") || drvbrd.equals("PRO2ESP32DRV8825") )
        {
          init_leds();
        }
      }
    }
  }
  msg = mserver.arg("stople");
  if ( msg != "" )
  {
    // if enabled then disable
    if ( mySetupData->get_inoutledstate() == 1)
    {
      mySetupData->set_inoutledstate(0);
    }
  }

  // HOME POSITION SWITCH, enable, disable
  msg = mserver.arg("hpswon");
  if ( msg != "" )
  {
    if ( mySetupData->get_brdhpswpin() == -1 )
    {
      DebugPrintln("hpsw pin is -1. Cannot enable pin");
      mySetupData->set_hpswitchenable(0);
    }
    else
    {
      // if current state is disabled then enable
      if ( mySetupData->get_hpswitchenable() == 0)
      {
        mySetupData->set_hpswitchenable(1);
        init_homepositionswitch();
      }
    }
  }
  msg = mserver.arg("hpswoff");
  if ( msg != "" )
  {
    mySetupData->set_hpswitchenable(0);
  }

  MANAGEMENT_sendadminpg2();
#ifdef TIMEMSHANDLEPG2
  Serial.print("ms_handlepg2: ");
  Serial.println(millis());
#endif
}

void MANAGEMENT_buildadminpg1(void)
{
#ifdef TIMEMSBUILDPG1
  Serial.print("ms_buildpg1: ");
  Serial.println(millis());
#endif
  // spiffs was started earlier when server was started so assume it has started
  if ( SPIFFS.exists("/msindex1.html"))                 // constructs home page of management server
  {
    DebugPrintln(FILEFOUNDSTR);
    File file = SPIFFS.open("/msindex1.html", "r");     // open file for read
    DebugPrintln(READPAGESTR);
    MSpg = file.readString();                           // read contents into string
    file.close();

    DebugPrintln(PROCESSPAGESTARTSTR);
    // process for dynamic data
    String bcol = mySetupData->get_wp_backcolor();
    MSpg.replace("%BKC%", bcol);
    String txtcol = mySetupData->get_wp_textcolor();
    MSpg.replace("%TXC%", txtcol);
    String ticol = mySetupData->get_wp_titlecolor();
    MSpg.replace("%TIC%", ticol);
    String hcol = mySetupData->get_wp_headercolor();
    MSpg.replace("%HEC%", hcol);
    MSpg.replace("%VER%", String(programVersion));
    MSpg.replace("%NAM%", mySetupData->get_brdname());
#ifdef BLUETOOTHMODE
    MSpg.replace("%MOD%", "BLUETOOTH : " + String(BLUETOOTHNAME));
#endif
#ifdef ACCESSPOINT
    MSpg.replace("%MOD%", "ACCESSPOINT");
#endif
#ifdef STATIONMODE
    MSpg.replace("%MOD%", "STATIONMODE");
#endif
#ifdef LOCALSERIAL
    MSpg.replace("%MOD%", "LOCALSERIAL");
#endif

#ifdef MDNSSERVER
    if ( mdnsserverstate == RUNNING)
    {
      MSpg.replace("%MST%", "RUNNING");
    }
    else
    {
      MSpg.replace("%MST%", "STOPPED");
    }
    MSpg.replace("%MPO%", "<form action=\"/\" method =\"post\">Port: <input type=\"text\" name=\"mdnsp\" size=\"8\" value=" + String(mySetupData->get_mdnsport()) + "> <input type=\"submit\" name=\"setmdnsport\" value=\"Set\"></form>");
    if ( mdnsserverstate == RUNNING)
    {
      MSpg.replace("%MBT%", String(MDNSTOPSTR));
    }
    else
    {
      MSpg.replace("%MBT%", String(MDNSSTARTSTR));
    }
#else
    MSpg.replace("%MST%", "Not defined");
    MSpg.replace("%MPO%", "Port: " + String(mySetupData->get_mdnsport()));
    MSpg.replace("%MBT%", " ");
#endif

#ifdef OTAUPDATES
    if ( otaupdatestate == RUNNING )
    {
      MSpg.replace("%OST%", "RUNNING");
    }
    else
    {
      MSpg.replace("%OST%", "STOPPED");
    }
#else
    MSpg.replace("%OST%", "Not defined");
#endif

#ifdef USEDUCKDNS
    if ( duckdnsstate == RUNNING )
    {
      MSpg.replace("%DST%", "RUNNING");
    }
    else
    {
      MSpg.replace("%DST%", "STOPPED");
    }
#else
    MSpg.replace("%DST%", "Not defined");
#endif

    // staticip %IPS%
    if ( staticip == STATICIPON )
    {
      MSpg.replace("%IPS%", "ON");
    }
    else
    {
      MSpg.replace("%IPS%", "OFF");
    }

    // display %OLE%
    DebugPrint(" MS: Display state: ");
    DebugPrintln(displaystate);
    if ( displaystate == true)
    {
      if ( mySetupData->get_displayenabled() == 1 )
      {
        MSpg.replace("%OLE%", String(DISPLAYONSTR));                // checked already
      }
      else
      {
        MSpg.replace("%OLE%", String(DISPLAYOFFSTR));               // not checked
      }
    }
    else
    {
      MSpg.replace("%OLE%", " not defined in firmware");            // not checked
    }

    // if oled display page group option update
    // %PG% is current page option, %PGO% is option binary string
    MSpg.replace("%PG%", mySetupData->get_oledpageoption() );
    String oled;
    oled = "<form action=\"/\" method=\"post\"><input type=\"text\" name=\"pg\" size=\"12\" value=" + String(mySetupData->get_oledpageoption()) + "> <input type=\"submit\" name=\"setpg\" value=\"Set\"></form>";
    MSpg.replace("%PGO%", oled );

    // page display time
    MSpg.replace("%PT%", String(mySetupData->get_lcdpagetime()) );
    oled = "<form action=\"/\" method=\"post\"><input type=\"text\" name=\"pt\" size=\"12\" value=" + String(mySetupData->get_lcdpagetime()) + "> <input type=\"submit\" name=\"setpt\" value=\"Set\"></form>";
    MSpg.replace("%PGT%", oled );

    // startscreen %SS%
    if ( mySetupData->get_showstartscreen() == 1 )
    {
      // checked already
      MSpg.replace("%SS%", String(STARTSCREENONSTR));
    }
    else
    {
      // not checked
      MSpg.replace("%SS%", String(STARTSCREENOFFSTR));
    }

    // FORCEMANAGEMENTDOWNLOAD %MDL%
    if ( mySetupData->get_forcedownload() == 1 )
    {
      // checked already
      MSpg.replace("%MDL%", String(STARTFMDLONSTR));
    }
    else
    {
      // not checked
      MSpg.replace("%MDL%", String(STARTFMDLOFFSTR));
    }

    // SHOWHPSWMESSAGES %HPM%
    if ( mySetupData->get_showhpswmsg() == 1 )
    {
      // checked already
      MSpg.replace("%HPM%", String(STARTHPSWMONSTR));
    }
    else
    {
      // not checked
      MSpg.replace("%HPM%", String(STARTHPSWMOFFSTR));
    }

    MSpg.replace("%BT%", String(CREBOOTSTR));           // add code to handle reboot controller

    // display heap memory for tracking memory loss?
    // only esp32?
    MSpg.replace("%HEA%", String(ESP.getFreeHeap()));
    DebugPrintln(PROCESSPAGEENDSTR);
  }
  else
  {
    // could not read file
    TRACE();
    DebugPrintln(BUILDDEFAULTPAGESTR);
    MSpg = MANAGEMENTNOTFOUNDSTR;
  }
#ifdef TIMEMSBUILDPG1
  Serial.print("ms_buildpg1: ");
  Serial.println(millis());
#endif
  delay(10);                                            // small pause so background tasks can run
}

void MANAGEMENT_handleadminpg1(void)
{
#ifdef TIMEMSHANDLEPG1
  Serial.print("ms_handlepg1: ");
  Serial.println(millis());
#endif
  // code here to handle a put request
  String msg;

  MANAGEMENT_checkreboot();                              // if reboot controller;

  // mdns server
  msg = mserver.arg("startmdns");
  if ( msg != "" )
  {
    DebugPrintln("MANAGEMENT_handleadminpg1: startmdns: ");
#ifdef MDNSSERVER
    start_mdns_service();
#endif
  }
  msg = mserver.arg("stopmdns");
  if ( msg != "" )
  {
    DebugPrintln("MANAGEMENT_handleadminpg1: stopmdns: ");
#ifdef MDNSSERVER
    stop_mdns_service();
#endif
  }
  // mdns port
  msg = mserver.arg("setmdnsport");
  if ( msg != "" )
  {
    DebugPrint("set web server port: ");
    DebugPrintln(msg);
    String mp = mserver.arg("mdnsp");                                 // process new webserver port number
    if ( mp != "" )
    {
      unsigned long newport = 0;
      DebugPrint("mp:");
      DebugPrintln(mp);
      newport = mp.toInt();
      if ( mdnsserverstate == STOPPED )
      {
        unsigned long currentport = mySetupData->get_mdnsport();
        if ( newport == currentport)
        {
          // port is the same so do not bother to change it
          DebugPrintln("mp err: new Port = current port");
        }
        else
        {
          if ( newport == MSSERVERPORT )                              // if same as management server
          {
            DebugPrintln("mp err: new Port = MSSERVERPORT");
          }
          else if ( newport == mySetupData->get_ascomalpacaport() )   // if same as ASCOM REMOTE server
          {
            DebugPrintln("mp err: new Port = ALPACAPORT");
          }
          else if ( newport == mySetupData->get_webserverport() )     // if same as web server
          {
            DebugPrintln("mp err: new Port = WEBSERVERPORT");
          }
          else if ( newport == mySetupData->get_tcpipport() )        // if same as tcpip server
          {
            DebugPrintln("wp err: new Port = SERVERPORT");
          }
          else
          {
            DebugPrintln("New webserver port = " + String(newport));
            mySetupData->set_mdnsport(newport);                       // assign new port and save it
          }
        }
      }
      else
      {
        DebugPrintln("Attempt to change mdnsserver port when mdnsserver running");
      }
    }
  }

  if ( displaystate == true)
  {
    // if update display state
    msg = mserver.arg("di");
    if ( msg != "" )
    {
      DebugPrint("Set display state: ");
      DebugPrintln(msg);
      if ( msg == "don" )
      {
        mySetupData->set_displayenabled(1);
        myoled->display_on();
      }
      else
      {
        mySetupData->set_displayenabled(0);
        myoled->display_off();
      }
    }
  }
  else
  {
    MSpg.replace("%OLE%", "Display not defined in firmware");     // not checked
  }

  // if oled display page group option update
  // OLED Page Group Option [State] [options][SET]
  msg = mserver.arg("setpg");
  if ( msg != "" )
  {
    String tp = mserver.arg("pg");
    if ( tp == "" )                                     // check for null
    {
      tp = OLEDPGOPTIONALL;
    }
    if ( tp.length() != 3  )                            // check for 3 digits
    {
      tp = OLEDPGOPTIONALL;
    }
    for ( unsigned int i = 0; i < tp.length(); i++)     // check for 0 or 1
    {
      if ( (tp[i] != '0') && (tp[i] != '1') )
      {
        tp = OLEDPGOPTIONALL;
        break;
      }
    }
    DebugPrint(SETPGOPTIONSTR);
    DebugPrintln(msg);
    mySetupData->set_oledpageoption(tp);
  }

  // if oled page time update
  msg = mserver.arg("settm");
  if ( msg != "" )
  {
    String tp = mserver.arg("pt");
    if ( tp != "" )
    {
      unsigned long pgtime = tp.toInt();
      if ( pgtime < MINOLEDPAGETIME )
      {
        pgtime = MINOLEDPAGETIME;                       // at least 2s
      }
      else if ( pgtime > MAXOLEDPAGETIME )
      {
        pgtime = MAXOLEDPAGETIME;
      }
      DebugPrint(SETPGTIMESTR);
      DebugPrintln(msg);
      mySetupData->set_lcdpagetime(pgtime);
    }
    else
    {
      // ignore
    }
  }

  // if update start screen
  msg = mserver.arg("ss");
  if ( msg != "" )
  {
    DebugPrint("Set start screen state: ");
    DebugPrintln(msg);
    if ( msg == "sson" )
    {
      mySetupData->set_showstartscreen(1);
    }
    else
    {
      mySetupData->set_showstartscreen(0);
    }
  }

  // if update home position switch messages
  msg = mserver.arg("hp");
  if ( msg != "" )
  {
    DebugPrint("Set hpswmsg state: ");
    DebugPrintln(msg);
    if ( msg == "hpon" )
    {
      mySetupData->set_showhpswmsg(1);
    }
    else
    {
      mySetupData->set_showhpswmsg(0);
    }
  }

  // if update force management server download
  msg = mserver.arg("fd");
  if ( msg != "" )
  {
    DebugPrint("Set ms_force download state: ");
    DebugPrintln(msg);
    if ( msg == "fdon" )
    {
      mySetupData->set_forcedownload(1);
    }
    else
    {
      mySetupData->set_forcedownload(0);
    }
  }

  // OTA, DuckDNS, StaticIP are status only so no need to check or update here

  MANAGEMENT_sendadminpg1();
#ifdef TIMEMSHANDLEPG1
  Serial.print("ms_handlepg1: ");
  Serial.println(millis());
#endif
}

void MANAGEMENT_sendadminpg5(void)
{
#ifdef TIMEMSSENDPG5
  Serial.print("ms_sendpg5: ");
  Serial.println(millis());
#endif
  MANAGEMENT_buildadminpg5();
  DebugPrintln("root() - send admin pg5");
  MANAGEMENT_sendmyheader();
  MANAGEMENT_sendmycontent();
  MSpg = "";
#ifdef TIMEMSSENDPG5
  Serial.print("ms_sendpg5: ");
  Serial.println(millis());
#endif
  delay(10);
}

void MANAGEMENT_sendadminpg4(void)
{
#ifdef TIMEMSSENDPG4
  Serial.print("ms_sendpg4: ");
  Serial.println(millis());
#endif
  MANAGEMENT_buildadminpg4();
  DebugPrintln("root() - send admin pg4");
  MANAGEMENT_sendmyheader();
  MANAGEMENT_sendmycontent();
  MSpg = "";
#ifdef TIMEMSSENDPG4
  Serial.print("ms_sendpg4: ");
  Serial.println(millis());
#endif
  delay(10);
}

void MANAGEMENT_sendadminpg3(void)
{
#ifdef TIMEMSSENDPG3
  Serial.print("ms_sendpg3: ");
  Serial.println(millis());
#endif
  MANAGEMENT_buildadminpg3();
  DebugPrintln("root() - send admin pg3");
  MANAGEMENT_sendmyheader();
  MANAGEMENT_sendmycontent();
  MSpg = "";
#ifdef TIMEMSSENDPG3
  Serial.print("ms_sendpg3: ");
  Serial.println(millis());
#endif
  delay(10);
}

void MANAGEMENT_sendadminpg2(void)
{
#ifdef TIMEMSSENDPG2
  Serial.print("ms_sendpg2: ");
  Serial.println(millis());
#endif
  MANAGEMENT_buildadminpg2();
  DebugPrintln("root() - send admin pg2");
  MANAGEMENT_sendmyheader();
  MANAGEMENT_sendmycontent();
  MSpg = "";
#ifdef TIMEMSSENDPG2
  Serial.print("ms_sendpg2: ");
  Serial.println(millis());
#endif
  delay(10);
}

void MANAGEMENT_sendadminpg1(void)
{
#ifdef TIMEMSSENDPG1
  Serial.print("ms_sendpg1: ");
  Serial.println(millis());
#endif
  MANAGEMENT_buildadminpg1();
  DebugPrintln("root() - send admin pg1");
  MANAGEMENT_sendmyheader();
  MANAGEMENT_sendmycontent();
  MSpg = "";
#ifdef TIMEMSSENDPG1
  Serial.print("ms_sendpg1: ");
  Serial.println(millis());
#endif
  delay(10);
}

void MANAGEMENT_sendACAOheader(void)
{
  mserver.sendHeader("Access-Control-Allow-Origin", "*");
}

void MANAGEMENT_sendjson(String str)
{
  //if ( mySetupData->get_crossdomain() == 1 )
  {
    MANAGEMENT_sendACAOheader();                               // add a cross origin header
  }
  mserver.send(NORMALWEBPAGE, JSONPAGETYPE, str );
}

// generic get
void MANAGEMENT_handleget(void)
{
  // return json string of state, on or off or value
  // ascom, leds, temp, webserver, position, ismoving, display, motorspeed, coilpower, reverse
  String jsonstr;

  if ( mserver.argName(0) == "ascom" )
  {
    jsonstr = "{\"ascomserver\":" + String(mySetupData->get_ascomserverstate()) + " }";
    MANAGEMENT_sendjson(jsonstr);
  }
  else if ( mserver.argName(0) == "leds" )
  {
    jsonstr = "{\"ledstate\":" + String(mySetupData->get_inoutledstate()) + " }";
    MANAGEMENT_sendjson(jsonstr);
  }
  else if ( mserver.argName(0) == "tempprobe" )
  {
    jsonstr = "{\"tempprobe\":" + String(mySetupData->get_temperatureprobestate()) + " }";
    MANAGEMENT_sendjson(jsonstr);
  }
  else if ( mserver.argName(0) == "webserver" )
  {
    jsonstr = "{\"webserver\":" + String(mySetupData->get_webserverstate()) + " }";
    MANAGEMENT_sendjson(jsonstr);
  }
  else if ( mserver.argName(0) == "position" )
  {
    jsonstr = "{ \"position\":" + String(mySetupData->get_fposition()) + " }";
    MANAGEMENT_sendjson(jsonstr);
  }
  else if ( mserver.argName(0) == "ismoving" )
  {
    jsonstr = "{ \"ismoving\":" + String(isMoving) + " }";
    MANAGEMENT_sendjson(jsonstr);
  }
  else if ( mserver.argName(0) == "display" )
  {
    jsonstr = "{ \"display\":" + String(mySetupData->get_displayenabled()) + " }";
    MANAGEMENT_sendjson(jsonstr);
  }
  else if ( mserver.argName(0) == "motorspeed" )
  {
    jsonstr = "{ \"motorspeed\":" + String(mySetupData->get_motorspeed()) + " }";
    MANAGEMENT_sendjson(jsonstr);
  }
  else if ( mserver.argName(0) == "coilpower" )
  {
    jsonstr = "{ \"coilpower\":" + String(mySetupData->get_coilpower()) + " }";
    MANAGEMENT_sendjson(jsonstr);
  }
  else if ( mserver.argName(0) == "reverse" )
  {
    jsonstr = "{ \"reverse\":" + String(mySetupData->get_reversedirection()) + " }";
    MANAGEMENT_sendjson(jsonstr);
  }
  else if ( mserver.argName(0) == "rssi" )
  {
    long rssi = getrssi();
    jsonstr = "{ \"rssi\":" + String(rssi) + " }";
    MANAGEMENT_sendjson(jsonstr);
  }
  else if ( mserver.argName(0) == "hpsw" )
  {
    jsonstr = "{ \"hpsw\":" + String(mySetupData->get_hpswitchenable()) + " }";
    MANAGEMENT_sendjson(jsonstr);
  }
  else
  {
    jsonstr = "{ \"error\":\"unknown-command\" }";
    MANAGEMENT_sendjson(jsonstr);
  }
}

// generic set
void MANAGEMENT_handleset(void)
{
  // get parameter after ?
  String value;
  bool rflag = false;
  String drvbrd = mySetupData->get_brdname();
  // ascom, leds, tempprobe, webserver, position, move, display, motorspeed, coilpower, reverse

  // ascom remote server
  value = mserver.arg("ascom");
  if ( value != "" )
  {
    if ( value == "on" )
    {
      DebugPrintln("ASCOM server: ON");
      if ( mySetupData->get_ascomserverstate() == 0)
      {
        start_ascomremoteserver();
        rflag = true;
      }
    }
    else if ( value == "off" )
    {
      DebugPrintln("ASCOM server: OFF");
      if ( mySetupData->get_ascomserverstate() == 1)
      {
        stop_ascomremoteserver();
        rflag = true;
      }
    }
  }

  // in out leds
  value = mserver.arg("leds");
  if ( value != "" )
  {
    if ( value == "on" )
    {
      if ( (mySetupData->get_brdinledpin() == -1) || (mySetupData->get_brdoutledpin() == -1) )
      {
        DebugPrintln("led pins are -1. Cannot enable leds");
        mySetupData->set_inoutledstate(0);
      }
      else
      {
        DebugPrintln("LED's: ON");
        if ( mySetupData->get_inoutledstate() == 0)
        {
          mySetupData->set_inoutledstate(1);
          // reinitialise pins
          if (drvbrd.equals("PRO2ESP32ULN2003") || drvbrd.equals("PRO2ESP32L298N") || drvbrd.equals("PRO2ESP32L293DMINI") || drvbrd.equals("PRO2ESP32L9110S") || drvbrd.equals("PRO2ESP32DRV8825") )
          {
            init_leds();
          }
          rflag = true;
        }
      }
    }
    else if ( value == "off" )
    {
      DebugPrintln("LED's: OFF");
      if ( mySetupData->get_inoutledstate() == 1)
      {
        mySetupData->set_inoutledstate(0);
        rflag = true;
      }
    }
  }

  // temperature probe
  value = mserver.arg("tempprobe");
  if ( value != "" )
  {
    if ( value == "on" )
    {
      if ( mySetupData->get_brdtemppin() == -1)
      {
        DebugPrintln("temp pin is -1. Cannot enable temp probe");
        mySetupData->set_temperatureprobestate(0);
      }
      else
      {
        DebugPrintln("Tempprobe: ON");
        if ( mySetupData->get_temperatureprobestate() == 0)
        {
          mySetupData->set_temperatureprobestate(1);
          myTempProbe = new TempProbe;
          rflag = true;
        }
      }
    }
    else if ( value == "off" )
    {
      DebugPrintln("Tempprobe: OFF");
      if ( mySetupData->get_temperatureprobestate() == 1)
      {
        // there is no destructor call
        mySetupData->set_temperatureprobestate(0);
        rflag = true;
      }
    }
  }

  // web server
  value = mserver.arg("webserver");
  if ( value != "" )
  {
    if ( value == "on" )
    {
      DebugPrintln("weberver: ON");
      if ( mySetupData->get_webserverstate() == 0)
      {
        start_webserver();
        rflag = true;
      }
    }
    else if ( value == "off" )
    {
      DebugPrintln("webserver: OFF");
      if ( mySetupData->get_webserverstate() == 1)
      {
        stop_webserver();
        rflag = true;
      }
    }
  }

  // position - does not move focuser
  value = mserver.arg("position");
  if ( value != "" )
  {
    unsigned long temp = value.toInt();
    DebugPrint("Set position: ");
    DebugPrintln(temp);
    ftargetPosition = ( temp > mySetupData->get_maxstep()) ? mySetupData->get_maxstep() : temp;
    mySetupData->set_fposition(ftargetPosition);      // current position in SPIFFS
    driverboard->setposition(ftargetPosition);        // current position in driver board
    rflag = true;
  }

  // move - moves focuser position
  value = mserver.arg("move");
  if ( value != "" )
  {
    unsigned long temp = value.toInt();
    DebugPrint("Move to position: ");
    DebugPrintln(temp);
    ftargetPosition = ( temp > mySetupData->get_maxstep()) ? mySetupData->get_maxstep() : temp;
    rflag = true;
  }

  if ( displaystate == true )
  {
    // display
    value = mserver.arg("display");
    if ( value != "" )
    {
      if ( value == "on" )
      {
        DebugPrintln("display: ON");
        mySetupData->set_displayenabled(1);
        myoled->display_on();
        rflag = true;
      }
      else if ( value == "off" )
      {
        DebugPrintln("display: OFF");
        if ( mySetupData->get_displayenabled() == 1)
        {
          mySetupData->set_displayenabled(0);
          myoled->display_off();
          rflag = true;
        }
      }
    }
  }
  else
  {
    MSpg.replace("%OLE%", "Display not defined in firmware");     // not checked
  }

  // motorspeed
  value = mserver.arg("motorspeed");
  if ( value != "" )
  {
    int tmp = value.toInt();
    DebugPrint("Motorspeed: ");
    DebugPrintln(tmp);
    if ( tmp < SLOW )
    {
      tmp = SLOW;
    }
    if ( tmp > FAST )
    {
      tmp = FAST;
    }
    mySetupData->set_motorspeed(tmp);
    rflag = true;
  }

  // coilpower
  value = mserver.arg("coilpower");
  if ( value != "" )
  {
    DebugPrint("coilpower:");
    DebugPrintln(value);
    if ( value == "on" )
    {
      mySetupData->set_coilpower(1);
      driverboard->enablemotor();
      rflag = true;
    }
    else if ( value == "off" )
    {
      mySetupData->set_coilpower(0);
      driverboard->releasemotor();
      rflag = true;
    }
  }

  // if reversedirection
  value = mserver.arg("reverse");
  if ( value != "" )
  {
    DebugPrint("reverse:");
    DebugPrintln(value);
    if ( value == "on" )
    {
      mySetupData->set_reversedirection(1);
      rflag = true;
    }
    else if ( value == "off" )
    {
      mySetupData->set_reversedirection(0);
      rflag = true;
    }
  }

  // home position switch
  value = mserver.arg("hpsw");
  if ( value != "" )
  {
    DebugPrint("hpsw:");
    DebugPrintln(value);
    if ( mySetupData->get_brdhpswpin() == -1 )
    {
      DebugPrintln("hpsw pin is -1. Cannot enable hpsw");
      mySetupData->set_hpswitchenable(0);
    }
    else
    {
      if ( value == "on" )
      {
        mySetupData->set_hpswitchenable(1);
        init_homepositionswitch();
        rflag = true;
      }
      else if ( value == "off" )
      {
        mySetupData->set_hpswitchenable(0);
        rflag = true;
      }
    }
  }

  // send generic OK
  if ( rflag == true )
  {
    MANAGEMENT_sendjson("{ \"cmd:\":\"ok\" }");
  }
  else
  {
    MANAGEMENT_sendjson("{ \"cmd:\":\"nok\" }");
  }
}

void MANAGEMENT_ascomoff(void)
{
  // ascom server stop
  if ( mySetupData->get_ascomserverstate() == 1)
  {
    DebugPrintln("stop ascomserver");
    stop_ascomremoteserver();
  }
  mserver.send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "ASCOM Alpaca Server Off");
}

void MANAGEMENT_ascomon(void)
{
  // ascom server start
  if ( mySetupData->get_ascomserverstate() == 0)
  {
    DebugPrintln("start ascomserver");
    start_ascomremoteserver();
  }
  mserver.send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "ASCOM Alpaca Server On");
}

void MANAGEMENT_ledsoff(void)
{
  // in out leds stop
  // if disabled then enable
  if ( mySetupData->get_inoutledstate() == 1)
  {
    DebugPrintln("leds off");
    mySetupData->set_inoutledstate(0);
  }
  mserver.send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "IN-OUT LED's Off");
}

void MANAGEMENT_ledson(void)
{
  String drvbrd = mySetupData->get_brdname();
  // in out leds start
  if ( (mySetupData->get_brdinledpin() == -1) || (mySetupData->get_brdoutledpin() == -1))
  {
    mySetupData->set_inoutledstate(0);
    mserver.send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "IN-OUT LED's pins not set");
  }
  else
  {
    if ( mySetupData->get_inoutledstate() == 0)
    {
      DebugPrintln("leds on");
      mySetupData->set_inoutledstate(1);
      // reinitialise pins
      if (drvbrd.equals("PRO2ESP32ULN2003") || drvbrd.equals("PRO2ESP32L298N") || drvbrd.equals("PRO2ESP32L293DMINI") || drvbrd.equals("PRO2ESP32L9110S") || drvbrd.equals("PRO2ESP32DRV8825") )
      {
        init_leds();
        mserver.send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "IN-OUT LED's enabled");
      }
      else
      {
        DebugPrintln("Not supported on board type");
        mserver.send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "Not supported on board type");
      }
    }
  }
}

void MANAGEMENT_tempoff(void)
{
  // temp probe stop
  if (mySetupData->get_temperatureprobestate() == 1)              // if probe enabled
  {
    DebugPrintln("temp off");
    // there is no destructor call
    mySetupData->set_temperatureprobestate(0);                    // disable probe
    myTempProbe->stop_temp_probe();                               // stop probe
    tprobe1 = 0;                                                  // indicate no probe found
  }
  mserver.send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "Temperature probe Off");
}

void MANAGEMENT_tempon(void)
{
  if ( mySetupData->get_brdtemppin() == -1 )
  {
    mySetupData->set_temperatureprobestate(0);
    mserver.send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "Temp probe pin not set");
  }
  else
  {
    // temp probe start
    if ( mySetupData->get_temperatureprobestate() == 0)               // if probe is disabled
    {
      mySetupData->set_temperatureprobestate(1);                      // enable it
      if ( tprobe1 == 0 )                                             // if there was no probe found
      {
        DebugPrintln("Create new tempprobe");
        myTempProbe = new TempProbe;                                  // create new instance and look for probe
        mserver.send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "Temperature probe On");
      }
      else
      {
        DebugPrintln("myTempProbe already created");
        mserver.send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "Temperature probe already on");
      }
    }
    else
    {
      mserver.send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "Temperature probe already On");
    }
  }
}

void MANAGEMENT_webserveroff(void)
{
  if ( mySetupData->get_webserverstate() == 1)
  {
    DebugPrintln("webserver off");
    stop_webserver();
  }
  mserver.send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "Web-Server Off");
}

void MANAGEMENT_webserveron(void)
{
  // set web server option
  if ( mySetupData->get_webserverstate() == 0)
  {
    DebugPrintln("webserver on");
    start_webserver();
  }
  mserver.send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "Web-Server On");
}

// halt
void MANAGEMENT_halt(void)
{
  halt_alert = true;
}

void MANAGEMENT_rssi(void)
{
  long rssi = getrssi();
  mserver.send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, String(rssi) );
}

// reboot controller
void MANAGEMENT_reboot(void)
{
  String WaitPage = "<html><meta http-equiv=refresh content=\"" + String(MSREBOOTPAGEDELAY) + "\"><head><title>Management Server></title></head><body><p>Please wait, controller rebooting.</p></body></html>";
  mserver.send(NORMALWEBPAGE, TEXTPAGETYPE, WaitPage );
  WaitPage = "";
  delay(1000);                                        // wait for page to be sent
  software_Reboot(REBOOTDELAY);
}

void MANAGEMENT_customconfig()
{
  // beta - user enters/pastes json string
  // user enters details on page then clicks generate board
#if defined(ESP8266)
  // show only E boards
#else
  // show only esp32 boards
#endif
}

void MANAGEMENT_customconfig_handler()
{

}

void MANAGEMENT_predefinedboard1()
{
  int brdnumber = -1;
  String boardname;
  String jsonstr;
  // build the board based on type
  // This will load all the board config from file and then save to board_config.jsn
  // there is a post so get the datastring that specifies board name
  // use datastring to load /boards/boardname.jsn
  // read the file into jsonstr
  // send it to createboardconfigfromjson

  // to handle reboot option
  MANAGEMENT_checkreboot();                              // if reboot controller;

  // process post data to extract driver board
  String msg = mserver.arg("brd");
  if ( msg != "" )
  {
    brdnumber = msg.toInt();
    DebugPrint("predefined board number: ");
    DebugPrintln(brdnumber);
  }

  // load driver config from /boards based on driverbrd number
  boardname = "/boards/" + String(brdnumber) + ".jsn";
  DebugPrint("Board name of file = ");
  DebugPrintln(boardname);

  // try to load board definition from file

  MSpg = "<html><head><title>Management Server</title></head><body>";
  MSpg = MSpg + "Predefined board numer: " + String(brdnumber) + "\n";
  if ( SPIFFS.exists(boardname))
  {
    DebugPrintln(FILEFOUNDSTR);
    File file = SPIFFS.open(boardname, "r");            // open file for read
    DebugPrintln(READPAGESTR);
    jsonstr = file.readString();                        // read contents into string
    file.close();
  }
  else
  {
    DebugPrintln("Error: Could not load board config file");
    jsonstr = "";
  }
  MSpg = MSpg + jsonstr + "\n";

  if ( jsonstr != "" )
  {
    if ( mySetupData->CreateBoardConfigfromjson(jsonstr) == true )
    {
      MSpg = MSpg + "Changed board type: Success\n";
    }
    else
    {
      MSpg = MSpg + "Changed board type: Fail\n";
      MSpg = MSpg + "Error in createBoardConfigfromjson(). Board type not set.\n";
    }
    // add a home button
    MSpg = MSpg + "<form action=\"/\" method=\"post\"><input type=\"submit\" value=\"Management Server HOME\"></form>";
  }
  MSpg = MSpg + "</body></html>\n\n";
  MANAGEMENT_sendmyheader();
  MANAGEMENT_sendmycontent();
  MSpg = "";
}

void MANAGEMENT_buildpredefinedboard()
{
  // MsPg size is around 3469 bytes.

  // spiffs was started earlier when server was started so assume it has started
  DebugPrintln("buildpredefinedconfig: Start");
  if ( SPIFFS.exists("/predefbrd.html"))
  {
    DebugPrintln(FILEFOUNDSTR);
    File file = SPIFFS.open("/predefbrd.html", "r");    // open file for read
    DebugPrintln(READPAGESTR);
    MSpg = file.readString();                           // read contents into string
    file.close();

    DebugPrintln(PROCESSPAGESTARTSTR);
    // process for dynamic data
    String bcol = mySetupData->get_wp_backcolor();
    MSpg.replace("%BKC%", bcol);
    String txtcol = mySetupData->get_wp_textcolor();
    MSpg.replace("%TXC%", txtcol);
    String ticol = mySetupData->get_wp_titlecolor();
    MSpg.replace("%TIC%", ticol);
    String hcol = mySetupData->get_wp_headercolor();
    MSpg.replace("%HEC%", hcol);
    MSpg.replace("%VER%", String(programVersion));
    MSpg.replace("%NAM%", mySetupData->get_brdname());

    // use #ifdef ESP8266 and elif to only display those boards for the target cpu type
    String espbrds;
    espbrds.reserve(2100);
#if defined(ESP8266)
    // List all the ESP8266 board types [1996]
    espbrds = "<table><tr><td>WEMOSDRV8825H</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"50\"><input type=\"submit\" value=\"Select\"></form></td></tr><tr><td>WEMOSDRV8825</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"35\"><input type=\"submit\" value=\"Select\"></form></td></tr><tr><td>PRO2EDRV8825</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"36\"><input type=\"submit\" value=\"Select\"></form></td></tr><tr><td>PRO2EDRV8825BIG</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"37\"><input type=\"submit\" value=\"Select\"></form></td></tr><tr><td>PRO2EULN2003</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"38\"><input type=\"submit\" value=\"Select\"></form></td></tr><tr><td>PRO2EL293DNEMA</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"39\"><input type=\"submit\" value=\"Select\"></form></td></tr><tr><td>PRO2EL293D28BYJ48</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"40\"><input type=\"submit\" value=\"Select\"></form></td></tr><tr><td>PRO2EL298N</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"41\"><input type=\"submit\" value=\"Select\"></form></td></tr><tr><td>PRO2EL293DMINI</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"42\"><input type=\"submit\" value=\"Select\"></form></td></tr><tr><td>PRO2EL9110S</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"43\"><input type=\"submit\" value=\"Select\"></form></td></tr><tr><td>CUSTOMBRD</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"99\"><input type=\"submit\" value=\"Select\"></form></td></tr></table>";
    MSpg.replace("%ESP8266%", espbrds);
    // hide esp32 boards
    MSpg.replace("%ESP32%", "None: Target is set for ESP8266" );
#else
    // List all the ESP32 board types [1298]
    espbrds = "<table><tr><td>PRO2ESP32DRV8825</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"44\"><input type=\"submit\" value=\"Select\"></form></td></tr><tr><td>PRO2ESP32ULN2003</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"45\"><input type=\"submit\" value=\"Select\"></form></td></tr><tr><td>PRO2ESP32L298N</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"46\"><input type=\"submit\" value=\"Select\"></form></td></tr><tr><td>PRO2ESP32L293DMINI</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"47\"><input type=\"submit\" value=\"Select\"></form></td></tr><tr><td>PRO2ESP32L9110S</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"48\"><input type=\"submit\" value=\"Select\"></form></td></tr><tr><td>PRO2ESP32R3WEMOS</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"49\"><input type=\"submit\" value=\"Select\"></form></td></tr><tr><td>CUSTOMBRD</td><td><form action=\"/predefbrd1\" method=\"post\"><input type=\"hidden\" name=\"brd\" value=\"99\"><input type=\"submit\" value=\"Select\"></form></td></tr></table>";
    MSpg.replace("%ESP32%", espbrds);

    // hide esp8266 boards
    MSpg.replace("%ESP8266%", "None: Target is set for ESP32");
#endif
    // display heap memory for tracking memory loss, %HEA%
    // only esp32?
    MSpg.replace("%HEA%", String(ESP.getFreeHeap()));

    // add code to handle reboot controller %BT%
    MSpg.replace("%BT%", String(CREBOOTSTR));

    DebugPrintln(PROCESSPAGEENDSTR);
  }
  else
  {
    // could not read file
    TRACE();
    DebugPrintln(BUILDDEFAULTPAGESTR);
    MSpg = MANAGEMENTNOTFOUNDSTR;
  }
}

void MANAGEMENT_predefinedboard()
{
  // to handle reboot option if this was HTTP_POST
  MANAGEMENT_checkreboot();                              // if reboot controller;

  MANAGEMENT_buildpredefinedboard();
  MANAGEMENT_sendmyheader();
  MANAGEMENT_sendmycontent();
  MSpg = "";
}

void MANAGEMENT_buildconfigpg()
{
  // spiffs was started earlier when server was started so assume it has started
  DebugPrintln("buildconfigpg: Start");
  if ( SPIFFS.exists("/config.html"))
  {
    DebugPrintln(FILEFOUNDSTR);
    File file = SPIFFS.open("/config.html", "r");       // open file for read
    DebugPrintln(READPAGESTR);
    MSpg = file.readString();                           // read contents into string
    file.close();

    DebugPrintln(PROCESSPAGESTARTSTR);
    // process for dynamic data
    String bcol = mySetupData->get_wp_backcolor();
    MSpg.replace("%BKC%", bcol);
    String txtcol = mySetupData->get_wp_textcolor();
    MSpg.replace("%TXC%", txtcol);
    String ticol = mySetupData->get_wp_titlecolor();
    MSpg.replace("%TIC%", ticol);
    String hcol = mySetupData->get_wp_headercolor();
    MSpg.replace("%HEC%", hcol);
    MSpg.replace("%VER%", String(programVersion));
    MSpg.replace("%NAM%", mySetupData->get_brdname());

    // add code to handle reboot controller %BT%
    MSpg.replace("%BT%", String(CREBOOTSTR));

    // display heap memory for tracking memory loss, %HEA%
    // only esp32?
    MSpg.replace("%HEA%", String(ESP.getFreeHeap()));
    DebugPrintln(PROCESSPAGEENDSTR);
  }
  else
  {
    // could not read file
    TRACE();
    DebugPrintln(BUILDDEFAULTPAGESTR);
    MSpg = MANAGEMENTNOTFOUNDSTR;
  }
}

void MANAGEMENT_confighandler()
{
  DebugPrintln("confighandler");

  // to handle reboot option
  MANAGEMENT_checkreboot();                              // if reboot controller;

  MANAGEMENT_buildconfigpg();
  MANAGEMENT_sendmyheader();
  MANAGEMENT_sendmycontent();
  MSpg = "";
}

void MANAGEMENT_config()
{
  // This will be start of config - select pre-defined or custon
  // After post this will goto either customconfig() or predefinedconfig()
  DebugPrintln("config");
  MANAGEMENT_buildconfigpg();
  MANAGEMENT_sendmyheader();
  MANAGEMENT_sendmycontent();
  MSpg = "";
}

void start_management(void)
{
  if ( !SPIFFS.begin() )
  {
    TRACE();
    DebugPrintln(FSNOTSTARTEDSTR);
    DebugPrintln(SERVERSTATESTOPSTR);
    managementserverstate = STOPPED;
    return;
  }
  MSpg.reserve(MAXMANAGEMENTPAGESIZE);        // largest page is MANAGEMENT_buildpredefinedboard() = 3469
  mserver.on("/",         HTTP_GET,  MANAGEMENT_sendadminpg1);
  mserver.on("/",         HTTP_POST, MANAGEMENT_handleadminpg1);
  mserver.on("/msindex1", HTTP_GET,  MANAGEMENT_sendadminpg1);
  mserver.on("/msindex1", HTTP_POST, MANAGEMENT_handleadminpg1);
  mserver.on("/msindex2", HTTP_GET,  MANAGEMENT_sendadminpg2);
  mserver.on("/msindex2", HTTP_POST, MANAGEMENT_handleadminpg2);
  mserver.on("/msindex3", HTTP_GET,  MANAGEMENT_sendadminpg3);
  mserver.on("/msindex3", HTTP_POST, MANAGEMENT_handleadminpg3);
  mserver.on("/msindex4", HTTP_GET,  MANAGEMENT_sendadminpg4);            // web colors
  mserver.on("/msindex4", HTTP_POST, MANAGEMENT_handleadminpg4);
  mserver.on("/color",    HTTP_GET,  MANAGEMENT_sendadminpg5);
  mserver.on("/color",    HTTP_POST, MANAGEMENT_handleadminpg5);          // color picker
  mserver.on("/delete",   HTTP_GET,  MANAGEMENT_displaydeletepage);
  mserver.on("/delete",   HTTP_POST, MANAGEMENT_handledeletefile);
  mserver.on("/list",     HTTP_GET,  MANAGEMENT_listFSfiles);
  mserver.on("/upload",   HTTP_GET,  MANAGEMENT_displayfileupload);

  mserver.on("/ascomoff",     HTTP_GET,  MANAGEMENT_ascomoff);
  mserver.on("/ascomon",      HTTP_GET,  MANAGEMENT_ascomon);
  mserver.on("/ledsoff",      HTTP_GET,  MANAGEMENT_ledsoff);
  mserver.on("/ledson",       HTTP_GET,  MANAGEMENT_ledson);
  mserver.on("/tempon",       HTTP_GET,  MANAGEMENT_tempon);
  mserver.on("/tempoff",      HTTP_GET,  MANAGEMENT_tempoff);
  mserver.on("/webserveroff", HTTP_GET,  MANAGEMENT_webserveroff);
  mserver.on("/webserveron",  HTTP_GET,  MANAGEMENT_webserveron);
  mserver.on("/rssi",         HTTP_GET,  MANAGEMENT_rssi);
  mserver.on("/set",                 MANAGEMENT_handleset);               // generic set function
  mserver.on("/get",                 MANAGEMENT_handleget);               // generic get function

  mserver.on("/config",  	    HTTP_GET,  MANAGEMENT_config);
  mserver.on("/config",  	    HTTP_POST, MANAGEMENT_confighandler);
  mserver.on("/predefbrd",               MANAGEMENT_predefinedboard);

  mserver.on("/predefbrd1",              MANAGEMENT_predefinedboard1);

  mserver.on("/custombrd",    HTTP_GET,  MANAGEMENT_customconfig);
  mserver.on("/custombrd",    HTTP_POST, MANAGEMENT_customconfig_handler);

  mserver.on("/upload",   HTTP_POST, []() {
    mserver.send(NORMALWEBPAGE);
  }, MANAGEMENT_handlefileupload );
  mserver.onNotFound([]() {                             // if the client requests any URI
    if (!MANAGEMENT_handlefileread(mserver.uri()))      // send file if it exists
    {
      MANAGEMENT_handlenotfound();                      // otherwise, respond with a 404 (Not Found) error
    }
  });
  mserver.begin();
  managementserverstate = RUNNING;
  TRACE();
  DebugPrintln(SERVERSTATESTARTSTR);
  delay(10);                                            // small pause so background tasks can run
}

void stop_management(void)
{
  if ( managementserverstate == RUNNING )
  {
    mserver.stop();
    managementserverstate = STOPPED;
    TRACE();
    DebugPrintln(SERVERSTATESTOPSTR);
  }
  else
  {
    DebugPrintln(SERVERNOTRUNNINGSTR);
  }
}

#endif // #ifdef MANAGEMENT
