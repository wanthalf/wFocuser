// ======================================================================
// displays.cpp : myFP2ESP OLED DISPLAY ROUTINES
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2021. All Rights Reserved.
// ======================================================================

// ======================================================================
// EXTERNALS
// ======================================================================

#include <Arduino.h>
#include "generalDefinitions.h"
#include "focuserconfig.h"                  // boarddefs.h included as part of focuserconfig.h"
#include "myBoards.h"
#include "FocuserSetupData.h"
#include "images.h"
#include "displays.h"
#include "temp.h"

// ======================================================================
// EXTERNALS - PROTOTYPES
// ======================================================================

extern unsigned long ftargetPosition;       // target position
extern float lasttemp;
extern char  ipStr[];                       // ip address
extern char  mySSID[];
extern bool  displaystate;

extern DriverBoard  *driverboard;
extern TempProbe    *myTempProbe;
extern SetupData    *mySetupData;
extern bool TimeCheck(unsigned long, unsigned long);

//__ helper function

bool CheckOledConnected(void)
{
  Display_DebugPrint("I2C SDA: ");
  Display_DebugPrintln(mySetupData->get_brdsda());
  Display_DebugPrint("I2C SCK: ");
  Display_DebugPrintln(mySetupData->get_brdsck());

#if defined(ESP8266)
  Display_DebugPrintln("Setup esp8266 I2C");
  if ( mySetupData->get_brdnumber() == PRO2EL293DNEMA || mySetupData->get_brdnumber() == PRO2EL293D28BYJ48)
  {
    Display_DebugPrintln("Setup PRO2EL293DNEMA/PRO2EL293D28BYJ48 I2C");
    Wire.begin(mySetupData->get_brdsda(), mySetupData->get_brdsck());        // l293d esp8266 shield
  }
  else
  {
    //SSD1306Wire display(0x3c, mySetupData->get_brdsda(), mySetupData->get_brdsck());
    Wire.begin(mySetupData->get_brdsda(), mySetupData->get_brdsck());
    //Wire.begin();
  }
#else
  // ESP32
  // Wire.begin([SDA], [SCL])
  // Wire.begin();
  Wire.begin(mySetupData->get_brdsda(), mySetupData->get_brdsck() );          // esp32
#endif

  Wire.setClock(400000L);                               // 400 kHZ max. speed on I2C

  Wire.beginTransmission(OLED_ADDR);                    // check if OLED display is present
  if (Wire.endTransmission() != 0)
  {
    Display_DebugPrintln("Display not found");
    return false;
  }
  Display_DebugPrintln("Display found");
  return true;
}

// ======================================================================
// CODE NON OLED
// ======================================================================

//void OLED_NON::oledgraphicmsg(String &str, int val, boolean clrscr) {}
//void OLED_NON::oled_draw_Wifi(int j) {}

void OLED_NON::oledtextmsg(String str, int val, boolean clrscr, boolean nl) {}
void OLED_NON::update_oledtext_position(void) {}
void OLED_NON::update_oledtextdisplay(void) {}
void OLED_NON::Update_Oled(const oled_state x, const connection_status y) {}
void OLED_NON::oled_draw_reboot(void) {}
void OLED_NON::display_on(void) {}
void OLED_NON::display_off(void) {}

OLED_NON::OLED_NON()  {}

// ======================================================================
// CODE OLED GRAPHIC MODE
// ======================================================================

//__constructor
#if (OLED_MODE == OLED_GRAPHIC)
OLED_GRAPHIC::OLED_GRAPHIC(void)
{
  Display_DebugPrintln("OLED_GRAPHIC");
#ifdef USE_SSD1306
  myoled = new SSD1306Wire(OLED_ADDR, mySetupData->get_brdsda(), mySetupData->get_brdsck(), GEOMETRY_128_64);
#else
  myoled = new SH1106Wire(OLED_ADDR, mySetupData->get_brdsda(), mySetupData->get_brdsck(), GEOMETRY_128_64);
#endif

  myoled->init();
  delay(1000);

  myoled->flipScreenVertically();
  myoled->setFont(ArialMT_Plain_10);
  myoled->setTextAlignment(TEXT_ALIGN_LEFT);
  myoled->clear();
  if (mySetupData->get_showstartscreen())
  {
    myoled->drawString(0, 0, "myFocuserPro2 v:" + String(programVersion));
    myoled->drawString(0, 12, ProgramAuthor);
  }
  myoled->display();

  timestamp = millis();
}

void OLED_GRAPHIC::Update_Oled(const oled_state oled, const connection_status ConnectionStatus)
{
  if (TimeCheck(timestamp, 750))
  {
    timestamp = millis();

    if (oled == oled_on)
    {
      oled_draw_main_update(ConnectionStatus);
    }
    else
    {
      myoled->clear();
      myoled->display();
    }
  }
}

void OLED_GRAPHIC::oledgraphicmsg(String &str, int val, boolean clrscr)
{
  myoled->setTextAlignment(TEXT_ALIGN_LEFT);
  myoled->setFont(ArialMT_Plain_10);

  if (clrscr == true)
  {
    myoled->clear();
    linecount = 0;
  }
  if (val != -1)
  {
    str += String(val);
  }
  myoled->drawString(0, linecount * 12, str);
  myoled->display();
  linecount++;
}

void OLED_GRAPHIC::oled_draw_Wifi(int j)
{
  myoled->clear();
  myoled->setTextAlignment(TEXT_ALIGN_CENTER);
  myoled->setFont(ArialMT_Plain_10);
  myoled->drawString(64, 0, "SSID: " + String(mySSID));
  myoled->drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits); // draw wifi logo

  for (int i = 1; i < 10; i++)
    myoled->drawXbm(12 * i, 56, 8, 8, (i == j) ? activeSymbol : inactiveSymbol);

  myoled->display();
}

const char heartbeat[] = { '-', '/' , '|', '\\'};

void OLED_GRAPHIC::oled_draw_main_update(const connection_status ConnectionStatus)
{
  char buffer[80];

  myoled->clear();
  myoled->setTextAlignment(TEXT_ALIGN_CENTER);
  myoled->setFont(ArialMT_Plain_24);

  if (ConnectionStatus == disconnected)
  {
    myoled->drawString(64, 28, F("offline"));

    myoled->setFont(ArialMT_Plain_10);
    myoled->drawString(64, 0, mySetupData->get_brdname());
    snprintf(buffer, sizeof(buffer), "IP= %s", ipStr);
    myoled->drawString(64, 14, buffer);
  }
  else
  {
    char dir = (mySetupData->get_focuserdirection() == moving_in ) ? '<' : '>';
    snprintf(buffer, sizeof(buffer), "%lu:%i %c", driverboard->getposition(), (int)(driverboard->getposition() % mySetupData->get_brdstepmode()), dir);
    myoled->drawString(64, 28, buffer);

    myoled->setFont(ArialMT_Plain_10);
    snprintf(buffer, sizeof(buffer), "µSteps: %i MaxPos: %lu", mySetupData->get_brdstepmode(), mySetupData->get_maxstep());
    myoled->drawString(64, 0, buffer);
    snprintf(buffer, sizeof(buffer), "TargetPos:  %lu", ftargetPosition);
    myoled->drawString(64, 12, buffer);
  }

  myoled->setTextAlignment(TEXT_ALIGN_LEFT);

  if ( mySetupData->get_temperatureprobestate() == 1)
  {
    snprintf(buffer, sizeof(buffer), "TEMP: %.2f C", lasttemp);
    myoled->drawString(54, 54, buffer);
  }
  else
  {
    snprintf(buffer, sizeof(buffer), "TEMP: %.2f C", 20.0);
  }

  snprintf(buffer, sizeof(buffer), "BL: %i", mySetupData->get_backlashsteps_out());
  myoled->drawString(0, 54, buffer);

  snprintf(buffer, sizeof(buffer), "%c", heartbeat[++count_hb % 4]);
  myoled->drawString(8, 14, buffer);

  myoled->display();
}

void OLED_GRAPHIC::oled_draw_reboot(void)
{
  myoled->clear();
  myoled->setTextAlignment(TEXT_ALIGN_CENTER);
  myoled->setFont(ArialMT_Plain_24);
  myoled->drawString(64, 28, "REBOOT"); // Print currentPosition
  myoled->display();
}

void OLED_GRAPHIC::display_on(void)
{
  // do nothing here - sort out code later
}

void OLED_GRAPHIC::display_off(void)
{
  // do nothing here - sort out code later
}
#endif

// ======================================================================
// Section OLED TEXT
// ======================================================================
#if (OLED_MODE == OLED_TEXT)
OLED_TEXT::OLED_TEXT(void)
{
  // Wire.begin();
  // Wire.begin([SDA], [SCL])
  // For ESP8266, sda":4,"sck":5,
  // this should mot be needed
  // Wire.begin(mySetupData->get_brdsda(), mySetupData->get_brdsck() );

  // Setup the OLED, screen size is 128 x 64, so using characters at 6x8 this gives 21chars across and 8 lines down
  myoled = new SSD1306AsciiWire();                      // instantiate the OLED object
#ifdef USE_SSD1306
  // For the OLED 128x64 0.96" display using the SSD1306 driver
  myoled->begin(&Adafruit128x64, OLED_ADDR);
#endif // #ifdef USE_SSD1306
#ifdef USE_SSH1106
  // For the OLED 128x64 1.3" display using the SSH1106 driver
  myoled->begin(&SH1106_128x64, OLED_ADDR);
#endif // #ifdef USE_SSH1106
  myoled->setFont(Adafruit5x7);
  myoled->clear();                                      // clrscr OLED

  displaystate = true;

  myoled->Display_Normal();                             // black on white
  myoled->InverseCharOff();
  myoled->Display_On();                                 // display ON

  Display_DebugPrint("Page time: ");
  Display_DebugPrintln(mySetupData->get_oledpagetime());

  if ( mySetupData->get_showstartscreen() )
  {
    Display_DebugPrintln("Show start screen");
    myoled->println(mySetupData->get_brdname());        // print startup screen
    myoled->println(programVersion);
    myoled->println(ProgramAuthor);
  }
}

void OLED_TEXT::oledtextmsg(String str, int val, boolean clrscr, boolean nl)
{
  if ( clrscr == true)                                  // clear the screen?
  {
    myoled->clear();
    myoled->setCursor(0, 0);
  }
  if ( nl == true )                                     // need to print a new line?
  {
    if ( val != -1)                                     // need to print a value?
    {
      myoled->print(str);
      myoled->println(val);
    }
    else
    {
      myoled->println(str);
    }
  }
  else
  {
    myoled->print(str);
    if ( val != -1 )
    {
      myoled->print(val);
    }
  }
}

/*
  void OLED_TEXT::update_oledtextdisplay(void)
  {
  static int displaybitmask = 1;
  static unsigned long currentMillis;
  static unsigned long olddisplaytimestampNotMoving = millis();

  currentMillis = millis();                               // see if the display needs updating
  if (((currentMillis - olddisplaytimestampNotMoving) > ((int)mySetupData->get_oledpagetime() * 1000)) || (currentMillis < olddisplaytimestampNotMoving))
  {
    olddisplaytimestampNotMoving = currentMillis;         // update the timestamp
    myoled->clear();                                      // clrscr OLED
    int page = 0;
    String mypage = String(mySetupData->get_oledpageoption(), BIN);
    // assign leading 0's if necessary
    while ( mypage.length() < 3)
    {
      mypage = "0" + mypage;
    }
    Display_DebugPrint("mypage option: ");
    Display_DebugPrintln(mypage);
    Display_DebugPrint("mypage length: ");
    Display_DebugPrintln(mypage.length());
    for (int i = 0; i < mypage.length(); i++)
    {
      page *= 2;
      if (mypage[i] == '1')
      {
        page++;
      }
    }
    // page now is a total of the string as int, string = 111, page = 7
    // find the next page to display
    while ( (page & displaybitmask) == 0 )          // mask off one bit at a time
    {
      displaybitmask *= 2;                          // skip this page as it is not enabled
      if ( displaybitmask > 4 )                     // pages 1 to 3 [note: displaybitmask is multiples of 2]
      {
        displaybitmask = 1;
        break;
      }
    } // while ( (page & displaybitmask) == 0 )

    Display_DebugPrint("Bit Mask: ");
    Display_DebugPrintln(displaybitmask);
    switch (displaybitmask)                         // displaybitmask is now the page to display, 1=pg1, 2=pg2, 4=pg3, 8=pg4 etc
    {
      case 1:
        Display_DebugPrintln("Show page 1");
        display_oledtext_page1();
        break;
      case 2:
        Display_DebugPrintln("Show page 2");
        display_oledtext_page2();
        break;
      case 4:
        Display_DebugPrintln("Show page 3");
        display_oledtext_page3();
        break;
      default:
        Display_DebugPrintln("err: Page 1");
        display_oledtext_page1();
        break;
    }
    displaybitmask *= 2;                            // next page
  }
  }
*/

void OLED_TEXT::update_oledtextdisplay(void)
{
  static int pagetodisplay = 0;
  static unsigned long currentMillis;
  static unsigned long olddisplaytimestampNotMoving = millis();

  currentMillis = millis();                               // see if the display needs updating
  if (((currentMillis - olddisplaytimestampNotMoving) > ((int)mySetupData->get_oledpagetime() * 1000)) || (currentMillis < olddisplaytimestampNotMoving))
  {
    olddisplaytimestampNotMoving = currentMillis;         // update the timestamp
    myoled->clear();                                      // clrscr OLED

    String mypage = String(mySetupData->get_oledpageoption(), BIN);
    // assign leading 0's if necessary
    while ( mypage.length() < 3)
    {
      mypage = "0" + mypage;
    }
    Display_DebugPrint("mypage option: ");
    Display_DebugPrintln(mypage);
    Display_DebugPrint("pagetodisplay: ");
    Display_DebugPrintln(pagetodisplay);
    // the last element of mypage is actually page0, it is in reverse order
    // mypage[0] = page 2
    // mypage[1] = page 1
    // mypage[2] = page 0
    if ( mypage[pagetodisplay] == '1' )
    {
      // display the page
      switch (pagetodisplay)
      {
        case 0:
          Display_DebugPrintln("Show page 2");
          display_oledtext_page2();
          break;
        case 1:
          Display_DebugPrintln("Show page 1");
          display_oledtext_page1();
          break;
        case 2:
          Display_DebugPrintln("Show page 0");
          display_oledtext_page0();
          break;
        default:
          Display_DebugPrintln("err: Page 0");
          display_oledtext_page0();
          break;
      }
    }
    pagetodisplay++;
    if ( pagetodisplay > 2 )
    {
      pagetodisplay = 0;
    }
  }
}

void OLED_TEXT::update_oledtext_position(void)
{
  myoled->setCursor(0, 0);
  myoled->print(CURRENTPOSSTR);
  myoled->print(driverboard->getposition());
  myoled->clearToEOL();
  myoled->println();

  myoled->print(TARGETPOSSTR);
  myoled->print(ftargetPosition);
  myoled->clearToEOL();
  myoled->println();
  //  display();
}

void OLED_TEXT::display_oledtext_page0(void)           // display screen
{
  char tempString[20];

  myoled->home();
  myoled->print(CURRENTPOSSTR);
  myoled->print(driverboard->getposition());
  myoled->clearToEOL();

  myoled->println();
  myoled->print(TARGETPOSSTR);
  myoled->print(ftargetPosition);
  myoled->clearToEOL();
  myoled->println();

  myoled->print(COILPWRSTR);
  myoled->print(mySetupData->get_coilpower());
  myoled->clearToEOL();
  myoled->println();

  myoled->print(REVDIRSTR);
  myoled->print(mySetupData->get_reversedirection());
  myoled->clearToEOL();
  myoled->println();

  // stepmode setting
  myoled->print(STEPMODESTR);
  myoled->print(mySetupData->get_brdstepmode());
  myoled->clearToEOL();
  myoled->println();

  // Temperature
  myoled->print(TEMPSTR);
  myoled->print(String(lasttemp, 2));
  myoled->print(" c");
  myoled->clearToEOL();
  myoled->println();

  // Motor Speed
  myoled->print(MOTORSPEEDSTR);
  myoled->print(mySetupData->get_motorspeed());
  myoled->clearToEOL();
  myoled->println();

  // MaxSteps
  myoled->print(MAXSTEPSSTR);
  ltoa(mySetupData->get_maxstep(), tempString, 10);
  myoled->print(tempString);
  myoled->clearToEOL();
  myoled->println();
}

void OLED_TEXT::display_oledtext_page1(void)
{
  // temperature compensation
  myoled->print(TCOMPSTEPSSTR);
  myoled->print(mySetupData->get_tempcoefficient());
  myoled->clearToEOL();
  myoled->println();

  myoled->print(TCOMPSTATESTR);
  myoled->print(mySetupData->get_tempcompenabled());
  myoled->clearToEOL();
  myoled->println();

  myoled->print(TCOMPDIRSTR);
  myoled->print(mySetupData->get_tcdirection());
  myoled->clearToEOL();
  myoled->println();

  myoled->print(BACKLASHINSTR);
  myoled->print(mySetupData->get_backlash_in_enabled());
  myoled->clearToEOL();
  myoled->println();

  myoled->print(BACKLASHOUTSTR);
  myoled->print(mySetupData->get_backlash_out_enabled());
  myoled->clearToEOL();
  myoled->println();

  myoled->print(BACKLASHINSTEPSSTR);
  myoled->print(mySetupData->get_backlashsteps_in());
  myoled->clearToEOL();
  myoled->println();

  myoled->print(BACKLASHOUTSTEPSSTR);
  myoled->print(mySetupData->get_backlashsteps_out());
  myoled->clearToEOL();
  myoled->println();
}

void OLED_TEXT::display_oledtext_page2(void)
{
#if ((CONTROLLERMODE == ACCESSPOINT) ||(CONTROLLERMODE == STATIONMODE) )
  myoled->setCursor(0, 0);
#if (CONTROLLERMODE == ACCESSPOINT)
  myoled->print("Access Point");
  myoled->clearToEOL();
  myoled->println();
#endif
#if (CONTROLLERMODE == STATIONMODE)
  myoled->print("Station mode");
  myoled->clearToEOL();
  myoled->println();
#endif
  myoled->print("SSID:");
  myoled->print(mySSID);
  myoled->clearToEOL();
  myoled->println();
  myoled->print("IP  :");
  myoled->print(ipStr);
  myoled->clearToEOL();
  myoled->println();
#endif // #if ((CONTROLLERMODE == ACCESSPOINT) ||(CONTROLLERMODE == STATIONMODE) )

  if ( mySetupData->get_webserverstate() == 1)
  {
    //setCursor(0, 0);
    myoled->print("Web Server");
    myoled->clearToEOL();
    myoled->println();
    myoled->print("IP  :");
    myoled->print(ipStr);
    myoled->clearToEOL();
    myoled->println();
    myoled->print("Port:");
    myoled->print(String(mySetupData->get_webserverport()));
    myoled->clearToEOL();
    myoled->println();
  }
  if ( mySetupData->get_ascomserverstate() == 1)
  {
    myoled->print("ASCOM REMOTE");
    myoled->clearToEOL();
    myoled->println();
    myoled->print("IP  :");
    myoled->print(ipStr);
    myoled->clearToEOL();
    myoled->println();
    myoled->print("Port:");
    myoled->print(mySetupData->get_ascomalpacaport());
    myoled->clearToEOL();
    myoled->println();
  }

#if (CONTROLLERMODE == BLUETOOTHMODE)
  myoled->setCursor(0, 0);
  myoled->print("Bluetooth");
  myoled->clearToEOL();
  myoled->println();
#endif

#if (CONTROLLERMODE == LOCALSERIAL)
  myoled->setCursor(0, 0);
  myoled->println("Local Serial");
#endif
}

void OLED_TEXT::Update_Oled(const oled_state oled, const connection_status ConnectionStatus)
{
  // do nothing
}

void OLED_TEXT::display_on()
{
  myoled->Display_On();
}


void OLED_TEXT::display_off()
{
  myoled->Display_Off();
}
#endif
