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
// https://www.ebay.com/sch/i.html?_from=R40&_trksid=p2510209.m570.l1313&_nkw=Keyes+KY-023+PS2+style+2-Axis+Joystick+with+Switch&_sacat=0
//
// On ESP32 analog input is 0-4095. GND=GND, VCC=3.3V
// ADC2 pins cannot be used when WiFi is being used
// ADC2 [GPIO4/GPIO2/GPIO15/GPIO13/GPIO12/GPIO14/GPIO27/GPIO26/GPIO25]
// If using WiFi use ADC1 pins
// ADC1 [GPIO33/GPIO32/GPIO35/GPIO34/GPIO39/GPIO36]

// If using JOYSTICK TYPE2 WITH SWITCH
// Wire SW to J15-y HEADER ON PCB, and install jumper on J16-PB0EN

#ifndef joystick_h
#define joystick_h

// ======================================================================
// Includes
// ======================================================================
#include <Arduino.h>

// ======================================================================
// Definitions
// ======================================================================
#define JOYINOUTPIN   34      	// ADC1_6, D34 - Wire to X
#define JOYOTHERPIN   35      	// ADC1_7, D35 - Do not wire
#define JZEROPOINT    1837	    // value read when joystick is centered
#define JTHRESHOLD    300     	// margin of error around center position
#define JMAXVALUE     4095      // maximum value reading of joystick
#define JMINVALUE     0         // minimum value reading of joystick



#endif // pushbuttons_h
