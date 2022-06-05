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
void OLED_NON::Update_Oled(bool force) {}
void OLED_NON::oled_draw_reboot(void) {}
void OLED_NON::display_on(void) { current_state = oled_on; }
void OLED_NON::display_off(void) { current_state = oled_off; }

void OLED_NON::setConnectionStatus(connection_status ConnectionStatus) { this->curConnectionStatus = ConnectionStatus; }

void OLED_NON::pbControl(unsigned long &ftargetPosition, pbTimer &pbModTimer, pbTimer &pbUpTimer, pbTimer &pbDnTimer, pbTimer &pbSetTimer) {
  long newpos = -1;

  if (!pbModTimer.initial) {
      if (pbUpTimer.last < LONGPRESS && pbUpTimer.current >= LONGPRESS) newpos = ftargetPosition - LONG_STEPS;
      else if (pbUpTimer.last < BOLDPRESS && pbUpTimer.current >= BOLDPRESS) newpos = ftargetPosition - BOLD_STEPS;
      else if (pbUpTimer.last == 0 && pbUpTimer.current > 0) newpos = ftargetPosition - SHORT_STEPS;
      else if (pbDnTimer.last < LONGPRESS && pbDnTimer.current >= LONGPRESS) newpos = ftargetPosition + LONG_STEPS;
      else if (pbDnTimer.last < BOLDPRESS && pbDnTimer.current >= BOLDPRESS) newpos = ftargetPosition + BOLD_STEPS;
      else if (pbDnTimer.last == 0 && pbDnTimer.current > 0) newpos = ftargetPosition + SHORT_STEPS;
  }

  if (newpos != -1) {
      newpos = (newpos < 0 ) ? 0 : newpos;
      newpos = (newpos > (long) mySetupData->get_maxstep()) ? (long) mySetupData->get_maxstep() : newpos;
      newTargetOffset = 0;
      ftargetPosition = newpos;
  }

}

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
  myoled->clear();
  if (mySetupData->get_showstartscreen())
  {
    myoled->setTextAlignment(TEXT_ALIGN_CENTER);
    myoled->setFont(ArialMT_Plain_24);
    myoled->drawString(64, 14, "myFocuser");
    myoled->setFont(ArialMT_Plain_10);
    myoled->drawString(64, 0, mySetupData->get_brdname());
    myoled->drawString(64, 40, "v" + String(programVersion));
    //myoled->drawString(0, 12, ProgramAuthor);
  }
  myoled->display();

  timestamp = millis();
}

void OLED_GRAPHIC::Update_Oled(bool force)
{
  if (force || TimeCheck(timestamp, 750))
  {
    timestamp = millis();

    if (current_state == oled_on)
    {
      if (page == PAGE_MAIN)
        oled_draw_main_update();
      else if (page == PAGE_PRESETS)
        oled_draw_presets();
      else if (page == PAGE_SETTINGS)
        oled_draw_settings();
      else if (page == PAGE_NETINFO)
        oled_draw_netinfo();
      else if (page == PAGE_CONFIRM)
        oled_draw_confirm();
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

void OLED_GRAPHIC::oled_draw_main_update()
{
  char buffer[80];

  myoled->clear();
  myoled->setTextAlignment(TEXT_ALIGN_CENTER);
  myoled->setFont(ArialMT_Plain_24);

  char dir = (mySetupData->get_focuserdirection() == moving_in ) ? '<' : '>';
  if (mySetupData->get_brdstepmode() != 1) {
    snprintf(buffer, sizeof(buffer), "%lu:%i %c", driverboard->getposition(), (int)(driverboard->getposition() % mySetupData->get_brdstepmode()), dir);
  } else {
    snprintf(buffer, sizeof(buffer), "%lu %c", driverboard->getposition(), dir);
  }
  myoled->drawString(64, 28, buffer);

  myoled->setFont(ArialMT_Plain_10);
  snprintf(buffer, sizeof(buffer), "µSteps: %i MaxPos: %lu", mySetupData->get_brdstepmode(), mySetupData->get_maxstep());
  myoled->drawString(64, 0, buffer);
  snprintf(buffer, sizeof(buffer), "TargetPos:  %lu", ftargetPosition+newTargetOffset);
  myoled->drawString(64, 12, buffer);
  if (newTargetOffset) {
    uint16_t txtlen = strlen(buffer);
    uint16_t txtwidth = myoled->getStringWidth(buffer, txtlen)+6;
    myoled->drawRect(64-txtwidth/2, 12, txtwidth, 13);
  }

  myoled->setTextAlignment(TEXT_ALIGN_LEFT);

  if ( mySetupData->get_temperatureprobestate() == 1)
  {
    snprintf(buffer, sizeof(buffer), "TEMP: %.2f C", lasttemp);
    myoled->drawString(54, 54, buffer);
  }
  
  snprintf(buffer, sizeof(buffer), "BL: %i", mySetupData->get_backlashsteps_out());
  myoled->drawString(0, 54, buffer);

  char conn = (curConnectionStatus == disconnected) ? ' ' : '>';
  snprintf(buffer, sizeof(buffer), "%c%c", conn, heartbeat[++count_hb % 4]);
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

void OLED_GRAPHIC::adjustValue(pbTimer &pbUpTimer, pbTimer &pbDnTimer, long &curValue, long minValue, long maxValue)
{
  int timespan = 100;
  int inc = 1;
  if ((pbUpTimer.initial && pbUpTimer.current > 4000) || (pbDnTimer.initial && pbDnTimer.current > 4000)) { timespan = 5; inc = 10; }
  else if ((pbUpTimer.initial && pbUpTimer.current > 3000) || (pbDnTimer.initial && pbDnTimer.current > 3000)) { timespan = 10; inc = 2; }
  else if ((pbUpTimer.initial && pbUpTimer.current > 2000) || (pbDnTimer.initial && pbDnTimer.current > 2000)) timespan = 25;
  else if ((pbUpTimer.initial && pbUpTimer.current > 1000) || (pbDnTimer.initial && pbDnTimer.current > 1000)) timespan = 50;
  if (pbUpTimer.initial && (pbUpTimer.current / timespan) > (pbUpTimer.last / timespan)) {
    curValue += inc;
    if (curValue > maxValue) curValue = maxValue;
    Update_Oled(true);
  }
  if (pbDnTimer.initial && (pbDnTimer.current / timespan) > (pbDnTimer.last / timespan)) {
    curValue -= inc;
    if (curValue < minValue) curValue = minValue;
    Update_Oled(true);
  }
}

void OLED_GRAPHIC::pbControl(unsigned long &ftargetPosition, pbTimer &pbModTimer, pbTimer &pbUpTimer, pbTimer &pbDnTimer, pbTimer &pbSetTimer) {

  if (pbModTimer.initial || (pbSetTimer.initial && pbSetTimer.current < LONGPRESS)) display_on(); // Up/Down treated elsewhere

  if (!pbModTimer.initial) {
    // MOD not pressed
    // SET: move to next page
    if (page != PAGE_CONFIRM && pbSetTimer.last > 0 && pbSetTimer.last < LONGPRESS && pbSetTimer.current == 0) {
      if (page == PAGE_MAIN) newTargetOffset = 0;
      if (page == PAGE_SETTINGS) {
        newCurPosOffset = 0;
        newBlInOffset = 0;
        newBlOutOffset = 0;
        newMaxPosOffset = 0;
      }
      if (page == PAGE_NETINFO) page = PAGE_MAIN;
      else page = (displayPage)(page + 1);
      // (re)set initial position value of the current preset in the presets page
      if (page == PAGE_PRESETS) curPresetPos = mySetupData->get_focuserpreset(curPreset);
      Update_Oled(true);
    }
    // SET(long press): turn display off
    if (pbSetTimer.last < LONGPRESS && pbSetTimer.current >= LONGPRESS) {
      display_off();
    }
  }

  if (page == PAGE_MAIN) {
    if (pbModTimer.initial) {
      adjustValue(pbUpTimer, pbDnTimer, newTargetOffset, 0-ftargetPosition, mySetupData->get_maxstep()-ftargetPosition);
      if (!pbSetTimer.initial && pbSetTimer.last >= LONGPRESS) {
        newTargetOffset = 0 - ftargetPosition;
        confType = CONFIRM_MGOTO;
        page = PAGE_CONFIRM;
        Update_Oled(true);
      } else if (newTargetOffset != 0 && !pbSetTimer.initial && pbSetTimer.last > 0 && pbSetTimer.last < LONGPRESS) {
        confType = CONFIRM_MGOTO;
        page = PAGE_CONFIRM;
        Update_Oled(true);
      }
    }
    OLED_NON::pbControl(ftargetPosition, pbModTimer, pbUpTimer, pbDnTimer, pbSetTimer);
  } else if (page == PAGE_PRESETS) {
    // handle preset editor
    if (!pbModTimer.initial) {
      // MOD not pressed
      // UP/DOWN: move to previous/next preset
      if (pbUpTimer.current && !pbUpTimer.last) {
        // store current setting and move to previous preset
        if (curPresetPos != mySetupData->get_focuserpreset(curPreset)){
          confType = CONFIRM_PSTORE;
          page = PAGE_CONFIRM;
        } else {
          curPreset--;
          if (curPreset > 9) curPreset = 9;
          curPresetPos = mySetupData->get_focuserpreset(curPreset);
        }
        Update_Oled(true);
      } else if (pbDnTimer.current && !pbDnTimer.last) {
        // store current setting and move to next preset
        if (curPresetPos != mySetupData->get_focuserpreset(curPreset)) {
          confType = CONFIRM_PSTORE;
          page = PAGE_CONFIRM;
        } else {
          curPreset++;
          if (curPreset > 9) curPreset = 0;
          curPresetPos = mySetupData->get_focuserpreset(curPreset);
        }
        Update_Oled(true);
      }
    } else {
      // MOD pressed
      // MOD+UP/DOWN: increase or decrease curent preset position value (on display only); accellerate with press length
      adjustValue(pbUpTimer, pbDnTimer, curPresetPos, 0, mySetupData->get_maxstep());
      // MOD+SET: set focuser position to preset or set preset to current focuser position
      if (!pbSetTimer.current) {
        // button released: we can test for press length
        if (pbSetTimer.last >= LONGPRESS) {
          // set current preset to current focuser position
          curPresetPos = ftargetPosition;
        } else if (pbSetTimer.last > 0 && pbSetTimer.last < LONGPRESS) {
          // move focuser to current preset position (store the current value if necessary; also switch to main page)
          if (curPresetPos != mySetupData->get_focuserpreset(curPreset)) confType = CONFIRM_PSTORE_GOTO;
          else confType = CONFIRM_PGOTO;
          page = PAGE_CONFIRM;
          Update_Oled(true);
        }
      }
    }
  } else if (page == PAGE_SETTINGS) {
    // handle settings editor
    if (!pbModTimer.initial) {
      // MOD not pressed
      // UP/DOWN: move to previous/next setting
      if (pbUpTimer.current && !pbUpTimer.last) {
        if (curSetting == 9 && newMaxPosOffset) {
          confType = CONFIRM_SETMAXPOS;
          page = PAGE_CONFIRM;
        } else if (curSetting == 4 && newCurPosOffset) {
          confType = CONFIRM_SETCURPOS;
          page = PAGE_CONFIRM;
        } else if (curSetting == 7 && newBlInOffset) {
          confType = CONFIRM_SETBLIN;
          page = PAGE_CONFIRM;
        } else if (curSetting == 8 && newBlOutOffset) {
          confType = CONFIRM_SETBLOUT;
          page = PAGE_CONFIRM;
        } else {
          curSetting--;
          if (curSetting > 9) curSetting = 9;
        }
        Update_Oled(true);
      } else if (pbDnTimer.current && !pbDnTimer.last) {
        if (curSetting == 9 && newMaxPosOffset) {
          confType = CONFIRM_SETMAXPOS;
          page = PAGE_CONFIRM;
        } else {
          curSetting++;
          if (curSetting > 9) curSetting = 0;
        }
        Update_Oled(true);
      }
    } else {
      // MOD pressed
      if (curSetting == 9) {
        adjustValue(pbUpTimer, pbDnTimer, newMaxPosOffset, 0-mySetupData->get_maxstep(), 999999 - mySetupData->get_maxstep());
      } else if (curSetting == 4) {
        adjustValue(pbUpTimer, pbDnTimer, newCurPosOffset, 0-ftargetPosition, 999999 - ftargetPosition);
      } else if (curSetting == 7) {
        adjustValue(pbUpTimer, pbDnTimer, newBlInOffset, 0 - mySetupData->get_backlashsteps_in(), 999999 - mySetupData->get_backlashsteps_in());
      } else if (curSetting == 8) {
        adjustValue(pbUpTimer, pbDnTimer, newBlOutOffset, 0 - mySetupData->get_backlashsteps_out(), 999999 - mySetupData->get_backlashsteps_out());
      } else if (pbUpTimer.current && !pbUpTimer.last) {
        if (curSetting == 0) mySetupData->set_hpswitchenable(mySetupData->get_hpswitchenable() ? (byte) 0 : (byte) 1);
        else if (curSetting == 1) mySetupData->set_temperatureprobestate(mySetupData->get_temperatureprobestate() ? (byte) 0 : (byte) 1);
        else if (curSetting == 2) mySetupData->set_backlash_in_enabled(mySetupData->get_backlash_in_enabled() ? (byte) 0 : (byte) 1);
        else if (curSetting == 3) mySetupData->set_backlash_out_enabled(mySetupData->get_backlash_out_enabled() ? (byte) 0 : (byte) 1);
        else if (curSetting == 5) {
          byte spd = mySetupData->get_motorspeed();
          if (spd == 2) spd = 0;
          else spd++;
          mySetupData->set_motorspeed(spd);
        } else if (curSetting == 6) {
          int step = mySetupData->get_brdstepmode();
          if (step != mySetupData->get_brdmaxstepmode()) step *= 2;
          driverboard->setstepmode(step);
        }
        Update_Oled(true);
      } else
      if (pbDnTimer.current && !pbDnTimer.last) {
        if (curSetting == 0) mySetupData->set_hpswitchenable(mySetupData->get_hpswitchenable() ? (byte) 0 : (byte) 1);
        else if (curSetting == 1) mySetupData->set_temperatureprobestate(mySetupData->get_temperatureprobestate() ? (byte) 0 : (byte) 1);
        else if (curSetting == 2) mySetupData->set_backlash_in_enabled(mySetupData->get_backlash_in_enabled() ? (byte) 0 : (byte) 1);
        else if (curSetting == 3) mySetupData->set_backlash_out_enabled(mySetupData->get_backlash_out_enabled() ? (byte) 0 : (byte) 1);
        else if (curSetting == 5) {
          byte spd = mySetupData->get_motorspeed();
          if (spd == 0) spd = 2;
          else spd--;
          mySetupData->set_motorspeed(spd);
        } else if (curSetting == 6) {
          int step = mySetupData->get_brdstepmode();
          if (step != 1) step /= 2;
          driverboard->setstepmode(step);
        }
        Update_Oled(true);
      }
    }
  } else if (page == PAGE_CONFIRM) {
    // DOWN: Cancel
    if (pbDnTimer.current && !pbDnTimer.last) {
      if (confType == CONFIRM_PGOTO || confType == CONFIRM_PSTORE_GOTO) {
        curPresetPos = mySetupData->get_focuserpreset(curPreset);
        page = PAGE_PRESETS;
      } else if (confType == CONFIRM_MGOTO) {
        newTargetOffset = 0;
        page = PAGE_MAIN;
      } else if (confType == CONFIRM_PSTORE) {
        curPresetPos = mySetupData->get_focuserpreset(curPreset);
        page = PAGE_PRESETS;
      } else if (confType == CONFIRM_SETCURPOS) {
        newCurPosOffset = 0;
        page = PAGE_SETTINGS;
      } else if (confType == CONFIRM_SETBLIN) {
        newBlInOffset = 0;
        page = PAGE_SETTINGS;
      } else if (confType == CONFIRM_SETBLOUT) {
        newBlOutOffset = 0;
        page = PAGE_SETTINGS;
      } else if (confType == CONFIRM_SETMAXPOS) {
        newMaxPosOffset = 0;
        page = PAGE_SETTINGS;
      }
      confType = CONFIRM_NONE;
      Update_Oled(true);
    // UP: Confirm
    } else if (pbUpTimer.current && !pbUpTimer.last) {
      if (confType == CONFIRM_PGOTO) {
        ftargetPosition = curPresetPos;
        page = PAGE_MAIN;
      } else if (confType == CONFIRM_MGOTO) {
        ftargetPosition = ftargetPosition + newTargetOffset;
        newTargetOffset = 0;
        page = PAGE_MAIN;
      } else if (confType == CONFIRM_PSTORE_GOTO) {
        mySetupData->set_focuserpreset(curPreset, curPresetPos);
        confType = CONFIRM_PGOTO;
      } else if (confType == CONFIRM_PSTORE) {
        mySetupData->set_focuserpreset(curPreset, curPresetPos);
        page = PAGE_PRESETS;
      } else if (confType == CONFIRM_SETCURPOS) {
        ftargetPosition = mySetupData->get_fposition()+newCurPosOffset;
        mySetupData->set_fposition(ftargetPosition);
        driverboard->setposition(ftargetPosition);
        newCurPosOffset = 0;
        page = PAGE_SETTINGS;
      } else if (confType == CONFIRM_SETBLIN) {
        mySetupData->set_backlashsteps_in(mySetupData->get_backlashsteps_in()+newBlInOffset);
        newBlInOffset = 0;
        page = PAGE_SETTINGS;
      } else if (confType == CONFIRM_SETBLOUT) {
        mySetupData->set_backlashsteps_out(mySetupData->get_backlashsteps_out()+newBlOutOffset);
        newBlOutOffset = 0;
        page = PAGE_SETTINGS;
      } else if (confType == CONFIRM_SETMAXPOS) {
        mySetupData->set_maxstep(mySetupData->get_maxstep()+newMaxPosOffset);
        newMaxPosOffset = 0;
        page = PAGE_SETTINGS;
      }
      Update_Oled(true);
    }
  }

}

void OLED_GRAPHIC::oled_highlightitem(int x, int y, bool filled) {
  myoled->setColor(INVERSE);
  if (filled)
    myoled->fillRect(x, y, 63, 12);
  else
    myoled->drawRect(x, y, 63, 12);
  myoled->setColor(WHITE);
}

void OLED_GRAPHIC::oled_draw_presets()
{
  char buffer[80];

  myoled->clear();
  myoled->setTextAlignment(TEXT_ALIGN_LEFT);
  myoled->setFont(ArialMT_Plain_10);

  for (int i = 0; i <= 9; i++)
  {
    int x = 0;
    if (i > 4) x = 64;
    int y = (i % 5) * 13;
    unsigned long presetPos = mySetupData->get_focuserpreset(i);
    unsigned long pos = (i == curPreset) ? curPresetPos : presetPos;
    snprintf(buffer, sizeof(buffer), "%i: %7u", i, pos);
    myoled->drawString(x+3, y, buffer);
    if (i == curPreset) oled_highlightitem(x, y, (curPresetPos == presetPos));
  }


  myoled->display();
}

void OLED_GRAPHIC::oled_draw_settings()
{
  char buffer[80];

  myoled->clear();
  myoled->setTextAlignment(TEXT_ALIGN_LEFT);
  myoled->setFont(ArialMT_Plain_10);

  snprintf(buffer, sizeof(buffer), "HPSw: %s", mySetupData->get_hpswitchenable() ? " on" : "off");
  myoled->drawString(3, 0, buffer);
  if (curSetting == 0) oled_highlightitem(0, 0);

  snprintf(buffer, sizeof(buffer), "Temp: %s", mySetupData->get_temperatureprobestate() ? " on" : "off");
  myoled->drawString(3, 13, buffer);
  if (curSetting == 1) oled_highlightitem(0, 13);

  snprintf(buffer, sizeof(buffer), "BlIn: %s", mySetupData->get_backlash_in_enabled() ? " on" : "off");
  myoled->drawString(3, 26, buffer);
  if (curSetting == 2) oled_highlightitem(0, 26);

  snprintf(buffer, sizeof(buffer), "BlOut: %s", mySetupData->get_backlash_out_enabled() ? " on" : "off");
  myoled->drawString(3, 39, buffer);
  if (curSetting == 3) oled_highlightitem(0, 39);

  unsigned long cpos = ftargetPosition + newCurPosOffset;
  snprintf(buffer, sizeof(buffer), "Pos:%s%i", (cpos > 99999) ? "" : " ", cpos);
  myoled->drawString(3, 52, buffer);
  if (curSetting == 4) oled_highlightitem(0, 52, (newCurPosOffset == 0));

  char speed[4] = "SMF";
  snprintf(buffer, sizeof(buffer), "Speed: %c", speed[mySetupData->get_motorspeed()]);
  myoled->drawString(67, 0, buffer);
  if (curSetting == 5) oled_highlightitem(64, 0);

  snprintf(buffer, sizeof(buffer), "µSteps: %i", mySetupData->get_brdstepmode());
  myoled->drawString(67, 13, buffer);
  if (curSetting == 6) oled_highlightitem(64, 13);

  unsigned long bipos = mySetupData->get_backlashsteps_in() + newBlInOffset;
  snprintf(buffer, sizeof(buffer), "BlIn: %i", bipos);
  myoled->drawString(67, 26, buffer);
  if (curSetting == 7) oled_highlightitem(64, 26, (newBlInOffset == 0));

  unsigned long bopos = mySetupData->get_backlashsteps_out() + newBlOutOffset;
  snprintf(buffer, sizeof(buffer), "BlOut: %i", bopos);
  myoled->drawString(67, 39, buffer);
  if (curSetting == 8) oled_highlightitem(64, 39, (newBlOutOffset == 0));

  unsigned long mpos = mySetupData->get_maxstep() + newMaxPosOffset;
  snprintf(buffer, sizeof(buffer), "Max:%s%i", (mpos > 99999) ? "" : " ", mpos);
  myoled->drawString(67, 52, buffer);
  if (curSetting == 9) oled_highlightitem(64, 52, (newMaxPosOffset == 0));

  myoled->display();
}

void OLED_GRAPHIC::oled_draw_netinfo()
{
  char buffer[80];

  myoled->clear();
  myoled->setTextAlignment(TEXT_ALIGN_CENTER);

  myoled->setFont(ArialMT_Plain_10);
  myoled->drawString(64, 0, "SSID: " + String(mySSID));
  snprintf(buffer, sizeof(buffer), "IP: %s", ipStr);
  myoled->drawString(64, 14, buffer);

  snprintf(buffer, sizeof(buffer), "client: %s", (curConnectionStatus == disconnected) ? "none" : "connected");
  myoled->drawString(64, 28, buffer);

  myoled->display();
}

void OLED_GRAPHIC::oled_draw_confirm()
{
  char buffer[80];

  myoled->clear();
  myoled->setFont(ArialMT_Plain_16);
  myoled->setTextAlignment(TEXT_ALIGN_CENTER);

  if (confType == CONFIRM_PGOTO || confType == CONFIRM_MGOTO) {
    unsigned long pos = (confType == CONFIRM_PGOTO) ? curPresetPos : (ftargetPosition + newTargetOffset);
    snprintf(buffer, sizeof(buffer), "Go to %u", pos);
  } else if (confType == CONFIRM_PSTORE_GOTO || confType == CONFIRM_PSTORE) {
    snprintf(buffer, sizeof(buffer), "Store %u", curPresetPos);
  } else if (confType == CONFIRM_SETMAXPOS) {
    snprintf(buffer, sizeof(buffer), "Set to %u", mySetupData->get_maxstep() + newMaxPosOffset);
  } else if (confType == CONFIRM_SETCURPOS) {
    snprintf(buffer, sizeof(buffer), "Set to %u", mySetupData->get_fposition() + newCurPosOffset);
  } else if (confType == CONFIRM_SETBLIN) {
    snprintf(buffer, sizeof(buffer), "Set to %u", mySetupData->get_backlashsteps_in() + newBlInOffset);
  } else if (confType == CONFIRM_SETBLOUT) {
    snprintf(buffer, sizeof(buffer), "Set to %u", mySetupData->get_backlashsteps_out() + newBlOutOffset);
  }
  myoled->drawString(64, 0, buffer);
  //myoled->drawRect(0, 0, 128, 19);
  myoled->setColor(INVERSE);
  myoled->drawString(64, 23, "^ Confirm ^");
  myoled->fillRect(0, 22, 128, 19);
  myoled->drawString(64, 45, "v Cancel v");
  myoled->fillRect(0, 44, 128, 19);
  myoled->setColor(WHITE);

  myoled->display();
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

void OLED_TEXT::Update_Oled(bool force)
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
