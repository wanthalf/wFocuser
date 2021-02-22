// ======================================================================
// joystick.h : myFP2ESP JOYSTICK DEFINITIONS
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

// 2-AXIS Analog Thumb Joystick for Arduino
// https://www.ebay.com/itm/1PCS-New-PSP-2-Axis-Analog-Thumb-GAME-Joystick-Module-3V-5V-For-arduino-PSP/401236361097
// https://www.ebay.com/itm/1PCS-New-PSP-2-Axis-Analog-Thumb-GAME-Joystick-Module-3V-5V-For-arduino-PSP/232426858990
//
// Keyes KY-023 PS2 style 2-Axis Joystick with Switch
// https://www.ebay.com/itm/Joy-Stick-Breakout-Module-Shield-PS2-Joystick-Game-Controller-For-Arduino/293033141970
//
// On ESP32 analog input is 0-4095. GND=GND, VCC=3.3V
// ADC2 pins cannot be used when WiFi is being used
// ADC2 [GPIO4/GPIO2/GPIO15/GPIO13/GPIO12/GPIO14/GPIO27/GPIO26/GPIO25]
// If using WiFi use ADC1 pins
// ADC1 [GPIO33/GPIO32/GPIO35/GPIO34/GPIO39/GPIO36]

#include <Arduino.h>

#ifndef joystick_h
#define joystick_h

#if defined(JOYSTICK1) || defined(JOYSTICK2)
#ifdef  JOYSTICK1
#define JOYINOUTPIN   34      	// ADC1_6, D34 - Wire to X
#define JOYOTHERPIN   35      	// ADC1_7, D35 - Do not wire
#define JZEROPOINT    1837	    // value read when joystick is centered
#define JTHRESHOLD    300     	// margin of error around center position
#define JMAXVALUE     4095      // maximum value reading of joystick
#define JMINVALUE     0         // minimum value reading of joystick
#endif

#ifdef  JOYSTICK2
#define JOYINOUTPIN   34        // ADC1_6, D34 - Wire to VRx
#define JOYOTHERPIN   35        // ADC1_7, D35 - Wire to SW
#define JZEROPOINT    1837	    // value read when joystick is centered
#define JTHRESHOLD    300     	// margin of error around center position
#define JMAXVALUE     4095      // maximum value reading of joystick
#define JMINVALUE     0         // minimum value reading of joystick
#endif

#endif // #if defined(JOYSTICK1) || defined(JOYSTICK2)


#endif // pushbuttons_h
