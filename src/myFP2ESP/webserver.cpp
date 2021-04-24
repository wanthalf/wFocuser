// ======================================================================
// webserver.cpp : myFP2ESP WEBSERVER ROUTINES
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

// ======================================================================
// Includes
// ======================================================================
#include "focuserconfig.h"                      // boarddefs.h included as part of focuserconfig.h"
#include "generalDefinitions.h"
#include "FocuserSetupData.h"
#include "myBoards.h"
#include "temp.h"
#include "displays.h"

#if defined(ESP8266)                            // this "define(ESP8266)" comes from Arduino IDE
#undef DEBUG_ESP_HTTP_SERVER                    // prevent messages from WiFiServer 
#include <ESP8266WiFi.h>
#include <FS.h>                                 // include the SPIFFS library  
#include <ESP8266WebServer.h>
#else                                           // otherwise assume ESP32
#include <WiFi.h>
#include "SPIFFS.h"
#include <WebServer.h>
#endif
#include <SPI.h>

// ======================================================================
// EXTERNS
// ======================================================================
extern volatile bool halt_alert;
#if defined(ESP8266)
// in esp8266, volatile data_type varname is all that is needed
#else
// in esp32, we should use a Mutex for access
extern portMUX_TYPE  halt_alertMux;
#endif

extern unsigned long ftargetPosition;           // target position
extern char          ipStr[16];                 // shared between BT mode and other modes
extern byte          isMoving;                  // is the motor currently moving
extern bool          webserverstate;
extern bool          reboot;
extern int           tprobe1;
extern float         lasttemp;
extern bool          displaystate;

extern TempProbe     *myTempProbe;
extern SetupData     *mySetupData;
extern DriverBoard*  driverboard;
extern OLED_NON      *myoled;
extern void          heapmsg(void);

// forward declarations
void WEBSERVER_sendpresets(void);
void WEBSERVER_sendroot(void);

// ======================================================================
// WEBSERVER Data
// ======================================================================
#include "webserver.h"
#if defined(ESP8266)
#undef DEBUG_ESP_HTTP_SERVER
ESP8266WebServer *webserver;
#else
WebServer *webserver;
#endif // if defined(esp8266)
String WSpg;

// ======================================================================
// WEBSERVER Code
// ======================================================================
void WEBSERVER_sendACAOheader(void)
{
  webserver->sendHeader("Access-Control-Allow-Origin", "*");
}

void WEBSERVER_sendmyheader(void)
{
  //webserver->sendHeader(F(CACHECONTROLSTR), F(NOCACHENOSTORESTR));
  //webserver->sendHeader(F(PRAGMASTR), F(NOCACHESTR));
  //webserver->sendHeader(F(EXPIRESSTR), "-1");
  //webserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
  //webserver->send(NORMALWEBPAGE, F(TEXTPAGETYPE), "");
  webserver->client().println("HTTP/1.1 200 OK");
  webserver->client().println("Content-type:text/html");
  //webserver->client().println("Connection: close");       // only valid on http/1.0
  webserver->client().println();
}

void WEBSERVER_sendmycontent()
{
  webserver->client().print(WSpg);
}

void WEBSERVER_buildnotfound(void)
{
  // spiffs was started earlier when server was started so assume it has started
  if ( SPIFFS.exists("/wsnotfound.html"))               // load page from fs - wsnotfound.html
  {
    File file = SPIFFS.open("/wsnotfound.html", "r");   // open file for read
    WSpg = file.readString();                           // read contents into string
    file.close();

    // process for dynamic data
    WSpg.replace("%IP%", ipStr);
    WSpg.replace("%POR%", String(mySetupData->get_webserverport()));
    WSpg.replace("%VER%", String(programVersion));
    WSpg.replace("%NAM%", mySetupData->get_brdname());
    String bcol = mySetupData->get_wp_backcolor();
    WSpg.replace("%BKC%", bcol);
    String txtcol = mySetupData->get_wp_textcolor();
    WSpg.replace("%TXC%", txtcol);
    String ticol = mySetupData->get_wp_titlecolor();
    WSpg.replace("%TIC%", ticol);
    String hcol = mySetupData->get_wp_headercolor();
    WSpg.replace("%HEC%", hcol);
  }
  else
  {
    TRACE();
    WebS_DebugPrintln("not found");
    WSpg = WEBSERVERNOTFOUNDSTR;
  }
  delay(10);                                            // small pause so background tasks can run
}

void WEBSERVER_handlenotfound(void)
{
  WEBSERVER_buildnotfound();
  webserver->send(NOTFOUNDWEBPAGE, TEXTPAGETYPE, WSpg);
  WSpg = "";
}

void WEBSERVER_buildpresets(void)
{
#ifdef TIMEWSBUILDPRESETS
  Serial.print("ws_buildpresets: ");
  Serial.println(millis());
#endif
  // spiffs was started earlier when server was started so assume it has started
  if ( SPIFFS.exists("/wspresets.html"))                // load page from fs - wspresets.html
  {
    File file = SPIFFS.open("/wspresets.html", "r");    // open file for read
    WSpg = file.readString();                           // read contents into string
    file.close();

    // process for dynamic data
    String bcol = mySetupData->get_wp_backcolor();
    WSpg.replace("%BKC%", bcol);
    String txtcol = mySetupData->get_wp_textcolor();
    WSpg.replace("%TXC%", txtcol);
    String ticol = mySetupData->get_wp_titlecolor();
    WSpg.replace("%TIC%", ticol);
    String hcol = mySetupData->get_wp_headercolor();
    WSpg.replace("%HEC%", hcol);
    WSpg.replace("%RAT%", String(mySetupData->get_webpagerefreshrate()));
    WSpg.replace("%IP%", ipStr);
    WSpg.replace("%POR%", String(mySetupData->get_webserverport()));
    WSpg.replace("%VER%", String(programVersion));
    WSpg.replace("%NAM%", mySetupData->get_brdname());
    WSpg.replace("%CPO%", String(driverboard->getposition()));
    WSpg.replace("%TPO%", String(ftargetPosition));
    WSpg.replace("%MOV%", String(isMoving));

    WSpg.replace("%WSP0%", String(mySetupData->get_focuserpreset(0)));
    WSpg.replace("%WSP1%", String(mySetupData->get_focuserpreset(1)));
    WSpg.replace("%WSP2%", String(mySetupData->get_focuserpreset(2)));
    WSpg.replace("%WSP3%", String(mySetupData->get_focuserpreset(3)));
    WSpg.replace("%WSP4%", String(mySetupData->get_focuserpreset(4)));
    WSpg.replace("%WSP5%", String(mySetupData->get_focuserpreset(5)));
    WSpg.replace("%WSP6%", String(mySetupData->get_focuserpreset(6)));
    WSpg.replace("%WSP7%", String(mySetupData->get_focuserpreset(7)));
    WSpg.replace("%WSP8%", String(mySetupData->get_focuserpreset(8)));
    WSpg.replace("%WSP9%", String(mySetupData->get_focuserpreset(9)));
  }
  else
  {
    // could not read preset file from SPIFFS
    TRACE();
    WebS_DebugPrintln("not found");
    WSpg = WEBSERVERNOTFOUNDSTR;
  }
#ifdef TIMEWSBUILDPRESETS
  Serial.print("ws_buildpresets: ");
  Serial.println(millis());
#endif
  delay(10);                                            // small pause so background tasks can run
}

void WEBSERVER_handlepresets(void)
{
#ifdef TIMEWSHANDLEPRESETS
  Serial.print("ws_handlepresets: ");
  Serial.println(millis());
#endif
  // if the root page was a HALT request via Submit button
  String halt_str = webserver->arg("ha");
  if ( halt_str != "" )
  {
    TRACE();
    WebS_DebugPrintln(halt_str);
    varENTER_CRITICAL(&halt_alertMux);
    halt_alert = true;
    varEXIT_CRITICAL(&halt_alertMux);
    // ftargetPosition = fcurrentPosition;
  }

  // if set focuser preset 0
  String fp_str = webserver->arg("setp0");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("setp0:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p0");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(0, (unsigned long)temp);
    }
  }

  // if goto focuser preset 0
  fp_str = webserver->arg("gop0");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("gop0:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p0");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(0, (unsigned long) temp);
      ftargetPosition = temp;
    }
  }

  // if set focuser preset 1
  fp_str = webserver->arg("setp1");

  if ( fp_str != "" )
  {
    WebS_DebugPrint("setp1:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p1");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(1, (unsigned long) temp);
    }
  }

  // if goto focuser preset 1
  fp_str = webserver->arg("gop1");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("gop1:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p1");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(1, (unsigned long) temp);
      ftargetPosition = temp;
    }
  }

  // if set focuser preset 2
  fp_str = webserver->arg("setp2");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("setp2:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p2");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(2, (unsigned long) temp);
    }
  }

  // if goto focuser preset 2
  fp_str = webserver->arg("gop2");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("gop2:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p2");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(2, (unsigned long) temp);
      ftargetPosition = temp;
    }
  }

  // if set focuser preset 3
  fp_str = webserver->arg("setp3");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("setp3:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p3");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(3, (unsigned long) temp);
    }
  }

  // if goto focuser preset 3
  fp_str = webserver->arg("gop3");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("gop3:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p3");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(3, (unsigned long) temp);
      ftargetPosition = temp;
    }
  }

  // if set focuser preset 4
  fp_str = webserver->arg("setp4");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("setp4:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p4");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(4, (unsigned long) temp);
    }
  }

  // if goto focuser preset 4
  fp_str = webserver->arg("gop4");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("gop4:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p4");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(4, (unsigned long) temp);
      ftargetPosition = temp;
    }
  }

  // if set focuser preset 5
  fp_str = webserver->arg("setp5");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("setp5:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p5");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(5, (unsigned long) temp);
    }
  }

  // if goto focuser preset 5
  fp_str = webserver->arg("gop5");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("gop5:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p5");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(5, (unsigned long) temp);
      ftargetPosition = temp;
    }
  }

  // if set focuser preset 6
  fp_str = webserver->arg("setp6");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("setp6:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p6");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(6, (unsigned long) temp);
    }
  }

  // if goto focuser preset 6
  fp_str = webserver->arg("gop6");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("gop6:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p6");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(6, (unsigned long) temp);
      ftargetPosition = temp;
    }
  }

  // if set focuser preset 7
  fp_str = webserver->arg("setp7");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("setp0:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p7");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(7, (unsigned long) temp);
    }
  }

  // if goto focuser preset 7
  fp_str = webserver->arg("gop7");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("gop0:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p7");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(7, (unsigned long) temp);
      ftargetPosition = temp;
    }
  }

  // if set focuser preset 8
  fp_str = webserver->arg("setp8");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("setp8:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p8");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(8, (unsigned long) temp);
    }
  }

  // if goto focuser preset 8
  fp_str = webserver->arg("gop8");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("gop8:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p8");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(8, (unsigned long) temp);
      ftargetPosition = temp;
    }
  }

  // if set focuser preset 9
  fp_str = webserver->arg("setp9");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("setp9:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p9");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(9, (unsigned long) temp);
    }
  }

  // if goto focuser preset 9
  fp_str = webserver->arg("gop9");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("gop9:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("p9");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = ( temp < 0 ) ? 0 : temp;
      temp = ( temp > (long)mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : temp;
      mySetupData->set_focuserpreset(9, (unsigned long) temp);
      ftargetPosition = temp;
    }
  }

  WEBSERVER_sendpresets();
#ifdef TIMEWSHANDLEPRESETS
  Serial.print("ws_handlepresets: ");
  Serial.println(millis());
#endif
  delay(10);                                            // small pause so background ESP8266 tasks can run
}

void WEBSERVER_sendpresets(void)
{
#ifdef TIMEWSSENDPRESETS
  Serial.print("ws:sendpresets: ");
  Serial.println(millis());
#endif
  WEBSERVER_buildpresets();
  // send the presetspage to a connected client
  WebS_DebugPrintln(SENDPAGESTR);
  WEBSERVER_sendmyheader();
  WEBSERVER_sendmycontent();
  WSpg = "";
#ifdef TIMEWSSENDPRESETS
  Serial.print("ws:sendpresets: ");
  Serial.println(millis());
#endif
  delay(10);                                            // small pause so background tasks can run
}

void WEBSERVER_buildmove(void)
{
  // construct the movepage now
#ifdef TIMEWSBUILDMOVE
  Serial.print("ws_buildmove: ");
  Serial.println(millis());
#endif
  // spiffs was started earlier when server was started so assume it has started
  if ( SPIFFS.exists("/wsmove.html"))                   // load page from fs - wsmove.html
  {
    File file = SPIFFS.open("/wsmove.html", "r");       // open file for read
    WSpg = file.readString();                           // read contents into string
    file.close();

    // process for dynamic data
    String bcol = mySetupData->get_wp_backcolor();
    WSpg.replace("%BKC%", bcol);
    String txtcol = mySetupData->get_wp_textcolor();
    WSpg.replace("%TXC%", txtcol);
    String ticol = mySetupData->get_wp_titlecolor();
    WSpg.replace("%TIC%", ticol);
    String hcol = mySetupData->get_wp_headercolor();
    WSpg.replace("%HEC%", hcol);
    WSpg.replace("%RAT%", String(mySetupData->get_webpagerefreshrate()));
    WSpg.replace("%IP%", ipStr);
    WSpg.replace("%POR%", String(mySetupData->get_webserverport()));
    WSpg.replace("%VER%", String(programVersion));
    WSpg.replace("%NAM%", mySetupData->get_brdname());
    WSpg.replace("%CPO%", String(driverboard->getposition()));
    WSpg.replace("%TPO%", String(ftargetPosition));
    WSpg.replace("%MOV%", String(isMoving));
  }
  else
  {
    // could not read move file from SPIFFS
    TRACE();
    WebS_DebugPrintln("not found");
    WSpg = WEBSERVERNOTFOUNDSTR;
  }
#ifdef TIMEWSBUILDMOVE
  Serial.print("ws_buildmove: ");
  Serial.println(millis());
#endif
  delay(10);                                            // small pause so background tasks can run
}

void WEBSERVER_sendmove(void)
{
#ifdef TIMEWSSENDMOVE
  Serial.print("ws:sendmove: ");
  Serial.println(millis());
#endif
  WEBSERVER_buildmove();
  WEBSERVER_sendmyheader();
  WEBSERVER_sendmycontent();
  WSpg = "";
#ifdef TIMEWSSENDMOVE
  Serial.print("ws:sendmove: ");
  Serial.println(millis());
#endif
  delay(10);                                            // small pause so background ESP8266 tasks can run
}

// handles move page of webserver
// this is called whenever a client requests move
void WEBSERVER_handlemove()
{
#ifdef TIMEWSMOVEHANDLE
  Serial.print("ws_handlemove: ");
  Serial.println(millis());
#endif
  // if the root page was a HALT request via Submit button
  String halt_str = webserver->arg("ha");
  if ( halt_str != "" )
  {
    TRACE();
    WebS_DebugPrintln(halt_str);
    varENTER_CRITICAL(&halt_alertMux);
    halt_alert = true;
    varEXIT_CRITICAL(&halt_alertMux);
    //ftargetPosition = fcurrentPosition;
  }

  // if move
  String fmv_str = webserver->arg("mv");
  if ( fmv_str != "" )
  {
    long temp = 0;
    TRACE();
    WebS_DebugPrintln(fmv_str);
    temp = fmv_str.toInt();
    long newtemp = (long) driverboard->getposition() + temp;
    newtemp = ( newtemp < 0 ) ? 0 : newtemp;
    newtemp = ( newtemp > (long)mySetupData->get_maxstep()) ? mySetupData->get_maxstep() : newtemp;
    ftargetPosition = (unsigned long) newtemp;
    WebS_DebugPrint("Move = "); WebS_DebugPrintln(fmv_str);
    WebS_DebugPrint("Position:");
    WebS_DebugPrintln(driverboard->getposition());
    WebS_DebugPrint("Target");
    WebS_DebugPrintln(ftargetPosition);
  }

  WEBSERVER_sendmove();
#ifdef TIMEWSMOVEHANDLE
  Serial.print("ws_handlemove: ");
  Serial.println(millis());
#endif
  delay(10);                                            // small pause so background ESP8266 tasks can run
}

void WEBSERVER_buildhome(void)
{
  // construct the homepage now
#ifdef TIMEWSROOTBUILD
  Serial.print("ws_buildroot: ");
  Serial.println(millis());
#endif
  // spiffs was started earlier when server was started so assume it has started
  if ( SPIFFS.exists("/wsindex.html"))                  // load page from fs - wsindex.html
  {
    File file = SPIFFS.open("/wsindex.html", "r");      // open file for read
    WSpg = file.readString();                           // read contents into string
    file.close();

    // process for dynamic data
    String bcol = mySetupData->get_wp_backcolor();
    WSpg.replace("%BKC%", bcol);
    String txtcol = mySetupData->get_wp_textcolor();
    WSpg.replace("%TXC%", txtcol);
    String ticol = mySetupData->get_wp_titlecolor();
    WSpg.replace("%TIC%", ticol);
    String hcol = mySetupData->get_wp_headercolor();
    WSpg.replace("%HEC%", hcol);

    WSpg.replace("%RAT%", String(mySetupData->get_webpagerefreshrate()));
    WSpg.replace("%IP%", ipStr);
    WSpg.replace("%POR%", String(mySetupData->get_webserverport()));
    WSpg.replace("%VER%", String(programVersion));
    WSpg.replace("%NAM%", mySetupData->get_brdname());
    // if this is a GOTO command then make this target else make current
    String fp_str = webserver->arg("gotopos");
    if ( fp_str != "" )
    {
      WSpg.replace("%CPO%", String(ftargetPosition));
    }
    else
    {
      WSpg.replace("%CPO%", String(driverboard->getposition()));
    }
    WSpg.replace("%TPO%", String(ftargetPosition));
    WSpg.replace("%MAX%", String(mySetupData->get_maxstep()));
    WSpg.replace("%MOV%", String(isMoving));

    if ( mySetupData->get_tempmode() == 1)
    {
      String tpstr = String(lasttemp, 2);
      WSpg.replace("%TEM%", tpstr);
      WSpg.replace("%TUN%", " c");
    }
    else
    {
      float ft = lasttemp;
      ft = (ft * 1.8) + 32;
      String tpstr = String(ft, 2);
      WSpg.replace("%TEM%", tpstr);
      WSpg.replace("%TUN%", " f");
    }

    WSpg.replace("%TPR%", String(mySetupData->get_tempresolution()));
    String smbuffer = String(mySetupData->get_brdstepmode());
    switch ( mySetupData->get_brdstepmode() )
    {
      case 1:
        smbuffer = WS_SM1CHECKED;
        smbuffer = smbuffer + WS_SM2UNCHECKED;
        smbuffer = smbuffer + WS_SM4UNCHECKED;
        smbuffer = smbuffer + WS_SM8UNCHECKED;
        smbuffer = smbuffer + WS_SM16UNCHECKED;
        smbuffer = smbuffer + WS_SM32UNCHECKED;
        break;
      case 2 :
        smbuffer = WS_SM1UNCHECKED;
        smbuffer = smbuffer + WS_SM2CHECKED;
        smbuffer = smbuffer + WS_SM4UNCHECKED;
        smbuffer = smbuffer + WS_SM8UNCHECKED;
        smbuffer = smbuffer + WS_SM16UNCHECKED;
        smbuffer = smbuffer + WS_SM32UNCHECKED;
        break;
      case 4 :
        smbuffer = WS_SM1UNCHECKED;
        smbuffer = smbuffer + WS_SM2UNCHECKED;
        smbuffer = smbuffer + WS_SM4CHECKED;
        smbuffer = smbuffer + WS_SM8UNCHECKED;
        smbuffer = smbuffer + WS_SM16UNCHECKED;
        smbuffer = smbuffer + WS_SM32UNCHECKED;
        break;
      case 8 :
        smbuffer = WS_SM1UNCHECKED;
        smbuffer = smbuffer + WS_SM2UNCHECKED;
        smbuffer = smbuffer + WS_SM4UNCHECKED;
        smbuffer = smbuffer + WS_SM8CHECKED;
        smbuffer = smbuffer + WS_SM16UNCHECKED;
        smbuffer = smbuffer + WS_SM32UNCHECKED;
        break;
      case 16 :
        smbuffer = WS_SM1UNCHECKED;
        smbuffer = smbuffer + WS_SM2UNCHECKED;
        smbuffer = smbuffer + WS_SM4UNCHECKED;
        smbuffer = smbuffer + WS_SM8UNCHECKED;
        smbuffer = smbuffer + WS_SM16CHECKED;
        smbuffer = smbuffer + WS_SM32UNCHECKED;
        break;
      case 32 :
        smbuffer = WS_SM1UNCHECKED;
        smbuffer = smbuffer + WS_SM2UNCHECKED;
        smbuffer = smbuffer + WS_SM4UNCHECKED;
        smbuffer = smbuffer + WS_SM8UNCHECKED;
        smbuffer = smbuffer + WS_SM16UNCHECKED;
        smbuffer = smbuffer + WS_SM32CHECKED;
        break;
      default :
        smbuffer = WS_SM1CHECKED;
        smbuffer = smbuffer + WS_SM2UNCHECKED;
        smbuffer = smbuffer + WS_SM4UNCHECKED;
        smbuffer = smbuffer + WS_SM8UNCHECKED;
        smbuffer = smbuffer + WS_SM16UNCHECKED;
        smbuffer = smbuffer + WS_SM32UNCHECKED;
        break;
    }
    WSpg.replace("%SMB%", smbuffer);
    String msbuffer = String(mySetupData->get_motorspeed());
    switch ( mySetupData->get_motorspeed() )
    {
      case 0:
        msbuffer = WS_MSSLOWCHECKED;
        msbuffer = msbuffer + WS_MSMEDUNCHECKED;
        msbuffer = msbuffer + WS_MSFASTUNCHECKED;
        break;
      case 1:
        msbuffer = WS_MSSLOWUNCHECKED;
        msbuffer = msbuffer + WS_MSMEDCHECKED;
        msbuffer = msbuffer + WS_MSFASTUNCHECKED;
        break;
      case 2:
        msbuffer = WS_MSSLOWUNCHECKED;
        msbuffer = msbuffer + WS_MSMEDUNCHECKED;
        msbuffer = msbuffer + WS_MSFASTCHECKED;
        break;
      default:
        msbuffer = WS_MSSLOWUNCHECKED;
        msbuffer = msbuffer + WS_MSMEDUNCHECKED;
        msbuffer = msbuffer + WS_MSFASTCHECKED;
        break;
    }
    WSpg.replace("%MSB%", msbuffer);
    String cpbuffer;
    if ( !mySetupData->get_coilpower() )
    {
      cpbuffer = "<input type=\"checkbox\" name=\"cp\" value=\"cp\" > ";
    }
    else
    {
      cpbuffer = "<input type=\"checkbox\" name=\"cp\" value=\"cp\" Checked> ";
    }
    WSpg.replace("%CPB%", cpbuffer);
    String rdbuffer;
    if ( !mySetupData->get_reversedirection() )
    {
      rdbuffer = "<input type=\"checkbox\" name=\"rd\" value=\"rd\" > ";
    }
    else
    {
      rdbuffer = "<input type=\"checkbox\" name=\"rd\" value=\"rd\" Checked> ";
    }
    WSpg.replace("%RDB%", rdbuffer);
    // display
    if ( displaystate == true )
    {
      if ( mySetupData->get_displayenabled() == 1 )
      {
        WSpg.replace("%OLE%", String(DISPLAYONSTR));      // checked already
      }
      else
      {
        WSpg.replace("%OLE%", String(DISPLAYOFFSTR));
      }
    }
    else
    {
      WSpg.replace("%OLE%", "<b>OLED:</b> Display not defined");
    }
  }
  else
  {
    // could not read index file from SPIFFS
    TRACE();
    WebS_DebugPrintln("not found");
    WSpg = WEBSERVERNOTFOUNDSTR;
  }
#ifdef TIMEWSROOTBUILD
  Serial.print("ws_buildroot: ");
  Serial.println(millis());
#endif
  delay(10);                                            // small pause so background tasks can run
}

// handles root page of webserver
// this is called whenever a client requests home page of sebserver
void WEBSERVER_handleroot()
{
#ifdef TIMEWSROOTHANDLE
  Serial.print("ws_handleroot: ");
  Serial.println(millis());
#endif
  // if the root page was a HALT request via Submit button
  String halt_str = webserver->arg("ha");
  if ( halt_str != "" )
  {
    WebS_DebugPrint("root() -halt:");
    WebS_DebugPrintln(halt_str);
    varENTER_CRITICAL(&halt_alertMux);
    halt_alert = true;
    varEXIT_CRITICAL(&halt_alertMux);
    //ftargetPosition = fcurrentPosition;
  }

  // if set focuser position
  String fp_str;
  fp_str = webserver->arg("setpos");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("setpos:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("fp");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      ftargetPosition = ( temp > (long)mySetupData->get_maxstep()) ? mySetupData->get_maxstep() : (unsigned long)temp;
      driverboard->setposition(ftargetPosition);
      mySetupData->set_fposition(ftargetPosition);
    }
  }

  // if goto focuser position
  fp_str = webserver->arg("gotopos");
  if ( fp_str != "" )
  {
    WebS_DebugPrint("gotopos:");
    WebS_DebugPrintln(fp_str);
    String fp = webserver->arg("fp");
    if ( fp != "" )
    {
      long temp = 0;
      WebS_DebugPrint("fp:");
      WebS_DebugPrintln(fp);
      temp = fp.toInt();
      temp = (temp < 0) ? 0 : temp;
      ftargetPosition = ( temp > (long)mySetupData->get_maxstep()) ? mySetupData->get_maxstep() : (unsigned long)temp;
    }
  }

  // if update of maxsteps
  String fmax_str = webserver->arg("fm");
  if ( fmax_str != "" )
  {
    long temp = 0;
    WebS_DebugPrint("root() -maxsteps:");
    WebS_DebugPrintln(fmax_str);
    temp = fmax_str.toInt();
    if ( temp < (long) driverboard->getposition() )               // if maxstep is less than focuser position
    {
      temp = (long) driverboard->getposition() + 1;
    }
    if ( temp < FOCUSERLOWERLIMIT )                     // do not set it less then 1024
    {
      temp = FOCUSERLOWERLIMIT;
    }
    if ( temp > (long) mySetupData->get_maxstep() )     // if higher than max value
    {
      temp = (long) mySetupData->get_maxstep();
    }
    mySetupData->set_maxstep((unsigned long)temp);
  }

  // if update motorspeed
  String fms_str = webserver->arg("ms");
  if ( fms_str != "" )
  {
    int temp1 = 0;
    WebS_DebugPrint("root() -motorspeed:");
    WebS_DebugPrintln(fms_str);
    temp1 = fms_str.toInt();
    if ( temp1 < SLOW )
    {
      temp1 = SLOW;
    }
    if ( temp1 > FAST )
    {
      temp1 = FAST;
    }
    mySetupData->set_motorspeed(temp1);
  }

  // if update coilpower
  String fcp_str = webserver->arg("cp");
  if ( fcp_str != "" )
  {
    WebS_DebugPrint("root() -coil power:");
    WebS_DebugPrintln(fcp_str);
    if ( fcp_str == "cp" )
    {
      mySetupData->set_coilpower(1);
    }
    else
    {
      mySetupData->set_coilpower(0);
    }
  }

  // if update reversedirection
  String frd_str = webserver->arg("rd");
  if ( frd_str != "" )
  {
    WebS_DebugPrint("root() -reverse direction:");
    WebS_DebugPrintln(frd_str);
    if ( frd_str == "rd" )
    {
      mySetupData->set_reversedirection(1);
    }
    else
    {
      mySetupData->set_reversedirection(0);
    }
  }

  // ======================================================================
  // Basic rule for setting stepmode
  // Set driverboard->setstepmode(xx);                 // this sets the physical pins and saves new stepmode
  // ======================================================================
  // if update stepmode
  // (1=Full, 2=Half, 4=1/4, 8=1/8, 16=1/16, 32=1/32, 64=1/64, 128=1/128, 256=1/256)
  String fsm_str = webserver->arg("sm");
  if ( fsm_str != "" )
  {
    int temp1 = 0;
    WebS_DebugPrint("root() -stepmode:");
    WebS_DebugPrintln(fsm_str);
    temp1 = fsm_str.toInt();
    if ( temp1 < STEP1 )
    {
      temp1 = STEP1;
    }
    if ( temp1 > mySetupData->get_brdmaxstepmode() )
    {
      temp1 = mySetupData->get_brdmaxstepmode();
    }
    driverboard->setstepmode(temp1);
  }

  // if update temperature resolution
  String tres_str = webserver->arg("tr");
  if ( tres_str != "" )
  {
    int temp = 0;
    WebS_DebugPrint("root() -temperature resolution:");
    WebS_DebugPrintln(tres_str);
    temp = tres_str.toInt();
    if ( temp < 9 )
    {
      // error, do not change
      temp = 9;
    }
    if ( temp > 12 )
    {
      temp = 12;
    }
    mySetupData->set_tempresolution(temp);
    if ( mySetupData->get_temperatureprobestate() == 1 )      // if probe enabled
    {
      if ( tprobe1 != 0 )                                     // if there was a probe found
      {
        myTempProbe->temp_setresolution((byte)temp);          // set resolution
      }
    }
  }

  if ( displaystate == true )
  {
    // if update display state
    String d_str = webserver->arg("di");
    if ( d_str != "" )
    {
      WebS_DebugPrint("root() -set display state: ");
      WebS_DebugPrintln(d_str);
      if ( d_str == "don" )
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
    WebS_DebugPrint("OLED not defined in firmware");
  }

  WEBSERVER_sendroot();
#ifdef TIMEWSROOTHANDLE
  Serial.print("ws_handleroot: ");
  Serial.println(millis());
#endif
}

void WEBSERVER_sendroot(void)
{
#ifdef TIMEWSROOTSEND
  Serial.print("ws_sendroot: ");
  Serial.println(millis());
#endif
  WEBSERVER_buildhome();
  WebS_DebugPrintln(SENDPAGESTR);
  WEBSERVER_sendmyheader();
  WEBSERVER_sendmycontent();
  WSpg = "";
#ifdef TIMEWSROOTSEND
  Serial.print("ws_sendroot: ");
  Serial.println(millis());
#endif
  delay(10);                                            // small pause so background ESP8266 tasks can run
}

void WEBSERVER_handleposition()
{
  webserver->send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, String(driverboard->getposition())); // Send position value only to client ajax request
}

void WEBSERVER_handleismoving()
{
  webserver->send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, String(isMoving));          // Send isMoving value only to client ajax request
}

void WEBSERVER_handletargetposition()
{
  webserver->send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, String(ftargetPosition));   //Send targetPosition value only to client ajax request
}

void WEBSERVER_handletemperature()
{
  webserver->send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, String(myTempProbe->read_temp(0), 2));   //Send temperature value only to client ajax request
}

void setup_webserver(void)
{
#if defined(ESP8266)
  webserver = new ESP8266WebServer(mySetupData->get_webserverport());
#else
  webserver = new WebServer(mySetupData->get_webserverport());
#endif // if defined(esp8266) 
  webserver->on("/",        HTTP_GET,  WEBSERVER_sendroot);
  webserver->on("/",        HTTP_POST, WEBSERVER_handleroot);
  webserver->on("/move",    HTTP_GET,  WEBSERVER_sendmove);
  webserver->on("/move",    HTTP_POST, WEBSERVER_handlemove);
  webserver->on("/presets", HTTP_GET,  WEBSERVER_sendpresets);
  webserver->on("/presets", HTTP_POST, WEBSERVER_handlepresets);

  webserver->on("/position", HTTP_GET, WEBSERVER_handleposition);
  webserver->on("/ismoving", HTTP_GET, WEBSERVER_handleismoving);
  webserver->on("/target",  HTTP_GET,  WEBSERVER_handletargetposition);
  webserver->on("/temp",    HTTP_GET,  WEBSERVER_handletemperature);

  webserver->onNotFound(WEBSERVER_handlenotfound);
  webserver->begin();
  WebS_DebugPrintln("start web server");
  mySetupData->set_webserverstate(1);
  HDebugPrint("Heap after  start_webserver = ");
  heapmsg();
  delay(10);                                        // small pause so background tasks can run
}

void start_webserver(void)
{
  if ( !SPIFFS.begin() )
  {
    TRACE();
    WebS_DebugPrintln("spiffs not started");
    WebS_DebugPrintln("stop webserver");
    mySetupData->set_webserverstate(0);     // disable web server
    return;
  }
  if ( WSpg == "" )
  {
    WSpg.reserve(MAXWEBPAGESIZE);
  }
  WebS_DebugPrintln("start web server");
  HDebugPrint("Heap before start_webserver = ");
  heapmsg();
  // on a reboot this test will be a 1
  if ( reboot == true )
  {
    setup_webserver();
  }
  if ( (reboot == false) && (mySetupData->get_webserverstate() == 0) )
  {
    setup_webserver();
  }
  else
  {
    WebS_DebugPrintln("Web-server already running");
  }
  webserverstate = RUNNING;
  delay(10);                                            // small pause so background tasks can run
}

void stop_webserver(void)
{
  if ( mySetupData->get_webserverstate() == 1)
  {
    webserver->close();
    delete webserver;             // free the webserver pointer and associated memory/code
    mySetupData->set_webserverstate(0);
    TRACE();
    WebS_DebugPrintln("web server stopped");
    WSpg = "";
  }
  else
  {
    WebS_DebugPrintln("web server not running");
  }
  webserverstate = STOPPED;
  delay(10);                      // small pause so background tasks can run
}
// WEBSERVER END -------------------------------------------------------------
