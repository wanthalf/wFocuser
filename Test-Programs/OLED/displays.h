// ======================================================================
// displays.h : myFP2ESP DISPLAY ROUTINES AND DEFINITIONS
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2021. All Rights Reserved.
// ======================================================================
#ifndef displays_h
#define displays_h

// check for SSD1306 display
#ifdef USE_SSD1306                            // for the OLED 128x64 0.96" display using the SSD1306 driver
// check for graphic display
#if OLED_MODE == OLED_GRAPHICS
#include <SSD1306Wire.h>                      // requires esp8266-oled-ssd1306 library
#endif // #if OLED_MODE == OLED_GRAPHICS
//check for text display
#if OLED_MODE == OLED_TEXT                    // do NOT use else!
//#include <mySSD1306Ascii.h>
#include <mySSD1306AsciiWire.h>               // requires myOLED library
#endif // if OLED_MODE == OLED_TEXT 
#endif // #ifdef USE_SSD1306

// check for SSH1106 display
#ifdef USE_SSH1106                            // for the OLED 128x64 1.3" display using the SSH1106 driver
// check for graphic display
#if OLED_MODE == OLED_GRAPHICS
#include <SH1106Wire.h>                       // requires esp8266-oled-ssd1306 library
#endif // #ifdef OLED_MODE == OLED_GRAPHICS
// check for text display
#if OLED_MODE == OLED_TEXT                    // do NOT use else!
#include <mySSD1306Ascii.h>
#include <mySSD1306AsciiWire.h>               // requires myOLED library
#endif // if OLED_MODE == OLED_TEXT 
#endif // #ifdef USE_SSH1106 

// ======================================================================
// DEFINITIONS
// ======================================================================

#define SCREEN_WIDTH          128           // OLED display width, in pixels
#define SCREEN_HEIGHT         64            // OLED display height, in pixels
#define OLED_ADDR             0x3C          // some OLED displays maybe at 0x3F, use I2Cscanner to find the correct address

//__helper function

extern bool CheckOledConnected(void);

// ======================================================================
// class boddies
// ======================================================================
class OLED_NON
{
  public:
    OLED_NON();
    //virtual void oledgraphicmsg(String &, int, bool);
    //virtual void oled_draw_Wifi(int);

    virtual void oledtextmsg(String, int, boolean, boolean);
    virtual void update_oledtext_position(void);
    virtual void update_oledtextdisplay(void);
    virtual void Update_Oled(const oled_state, const connection_status);
    virtual void oled_draw_reboot(void);
    virtual void display_on(void);
    virtual void display_off(void);

    byte  current_status = oled_on;
    byte  linecount = 0;
};

class OLED_TEXT : public SSD1306AsciiWire, public OLED_NON
{
  public:
    OLED_TEXT();
    void oledtextmsg(String , int , boolean , boolean);
    void update_oledtext_position(void);
    void update_oledtextdisplay(void);
    void display_on(void);
    void display_off(void);
  private:
    void displayoledpage0(void);      // displaylcd screen
    void displayoledpage1(void);
    void displayoledpage2(void);
    void Update_OledText(void);
    void UpdatePositionOledText(void);
    void display_oledtext_page0(void);
    void display_oledtext_page1(void);
    void display_oledtext_page2(void);
};

#ifdef USE_SSD1306
class OLED_GRAPHIC : public SSD1306Wire, public OLED_NON
#else // Assume USE_SSH1106
class OLED_GRAPHIC : public SH1106Wire, public OLED_NON
#endif // #ifdef USE_SSD1306
{
  public:
    OLED_GRAPHIC();
    void Update_Oled(const oled_state, const connection_status);
    void oledgraphicmsg(String &, int, boolean);
    void oled_draw_Wifi(int);
    void oled_draw_reboot(void);
    void display_on(void);
    void display_off(void);

  private:
    void oled_draw_main_update(const connection_status);
    byte count_hb = 0;      // heart beat counter
    long timestamp;
};

#endif // #ifdef displays_h
