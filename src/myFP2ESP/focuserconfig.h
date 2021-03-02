// ======================================================================
// focuserconfig.h : myFP2ESP FOCUSER CONFIG DEFINITIONS
// SPECIFY HARDWARE OPTIONS AND CONTROLLER MODES HERE
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

#include <Arduino.h>
#include "boarddefs.h"

#ifndef focuserconfig_h
#define focuserconfig_h

// Caution: Do not enable a feature if the associated hardware circuits are 
// not fitted on the board. Enable or disable the specific options below

// ======================================================================
// 1: BOARD DEFINES [do not change names]
// ======================================================================
// Uncomment only your board - ONLY ONE BOARD SHOULD BE UNCOMMENTED

// ESP8266 Boards
#define DRVBRD WEMOSDRV8825H                    // driver definition for Holger
//#define DRVBRD WEMOSDRV8825
//#define DRVBRD PRO2EULN2003
//#define DRVBRD PRO2EDRV8825
//#define DRVBRD PRO2EL293DNEMA
//#define DRVBRD PRO2EL293D28BYJ48
//#define DRVBRD PRO2EL298N
//#define DRVBRD PRO2EL293DMINI
//#define DRVBRD PRO2EL9110S
//#define DRVBRD CUSTOMBRD
// ESP32 Boards
//#define DRVBRD PRO2ESP32DRV8825
//#define DRVBRD PRO2ESP32ULN2003
//#define DRVBRD PRO2ESP32L298N
//#define DRVBRD PRO2ESP32L293DMINI
//#define DRVBRD PRO2ESP32L9110S
//#define DRVBRD PRO2ESP32R3WEMOS
//#define DRVBRD CUSTOMBRD

// On bootup following a controller firmware update, a default driver board file 
// is created based on your board selection above.
// In the MANAGEMENT server you can edit the board pin numbers and save the config.

// ======================================================================
// 2: SPECIFY FIXEDSTEPMODE
// ======================================================================
// For ESP8266 boards, set the fixedmode value to match the stepmode jumper
// settings on the board [only one line can be enabled]
// Applies to boards WEMOSDRV8825H, WEMOSDRV8825, PRO2EDRV8825BIG, PRO2EDRV8825
#define FIXEDSTEPMODE 1
//#define FIXEDSTEPMODE 2
//#define FIXEDSTEPMODE 4
//#define FIXEDSTEPMODE 8
//#define FIXEDSTEPMODE 16
//#define FIXEDSTEPMODE 32

// ======================================================================
// 3A: SPECIFY OLED DISPLAY AND DRIVER TYPE: 
// ======================================================================
// If you need no support for OLED skip this section. To enable the 
// OLED TEXT or GRAPHIC DISPLAY uncomment the related line. 

//#define OLED_MODE OLED_TEXT       // OLED text only mode
//#define OLED_MODE OLED_GRAPHIC  // OLED graphic mode

// ======================================================================
// 3B: SPECIFY OLED DISPLAY AND DRIVER TYPE: 
// ======================================================================
// only uncomment one of the following USE_SSxxxx lines depending upon your lcd type
// For the OLED 128x64 0.96" display using the SSD1306 driver, uncomment the following line
#define USE_SSD1306   1

// For the OLED 128x64 1.3" display using the SSH1106 driver, uncomment the following line
//#define USE_SSH1106   2


// ======================================================================
// 4: SPECIFY HARDWARE OPTIONS
// ======================================================================

// To enable the 2-Axis Joystick [ESP32 only], uncomment the next line
//#define JOYSTICK1 1

// To enable the Keyes KY-023 PS2 2-Axis Joystick [ESP32 only],
// uncomment the next line
//#define JOYSTICK2 2

// To enable the Infrared remote controller [ESP32 only], uncomment the next line
//#define INFRAREDREMOTE

// ======================================================================
// 5: SPECIFY CONTROLLER OPTIONS
// ======================================================================

// to enable Bluetooth mode, uncomment the next line [ESP32 only]
//#define BLUETOOTHMODE 1

// to work as an access point, define accesspoint - cannot use DUCKDNS
#define ACCESSPOINT 2

// to work as a station accessing a AP, define stationmode
//#define STATIONMODE 3

// to work only via USB cable as Serial port, uncomment the next line
//#define LOCALSERIAL 4

// To enable OTA updates, uncomment the next line [only works in STATIONMODE]
//#define OTAUPDATES 5

// mdns support [myfp2eap.local:8080]
// to enable multicast DNS, uncomment the next line [only works in STATIONMODE]
//#define MDNSSERVER 8

// Management Server Control Interface [Port 6060] - DO NOT CHANGE
#define MANAGEMENT 9

// To make the firmware return the correct firmware value when talking to a
// myFocuserpro2 INDI driver [use only for INDI support], uncomment the following line
//#define INDI 10

// Cannot use DuckDNS with ACCESSPOINT, BLUETOOTHMODE or LOCALSERIAL mode
// To enable DUCKDNS [STATIONMODE only]
//#define USEDUCKDNS 1

// to enable reading SSID and PASSWORD 
// from SPIFFS file wificonfig at boot time, uncomment the following file
//#define READWIFICONFIG 1


// ======================================================================
// DO NOT CHANGE: THESE DEFINITIONS NOW DEPRECATED
// ======================================================================

// To enable In and Out Pushbuttons [ESP32 only], uncomment the next line
// This has moved to MANAGEMENT SERVER

// do NOT uncomment HOMEPOSITIONSWITCH if you do not have the switch fitted
// To enable the HOMEPOSITION SWITCH [ESP32 only], uncomment the next line
// This has moved to MANAGEMENT SERVER

// To enable TEMPERATUREPROBE, uncomment the next line
// This has moved to MANAGEMENT SERVER

// To enable BACKLASH IN/OUT in this firmware, uncomment the next line
// This has moved to MANAGEMENT SERVER

// To enable INOUTLEDS [ESP32 only], uncomment the next line
// This has moved to MANAGEMENT SERVER

// To enable SHOWSTARTSCRN - the start boot screen showing startup messages, 
// uncomment the next line
// This has moved to MANAGEMENT SERVER

// To enable SHOWHPSWMESSAGES - display Home Position Switch Messages 
// on the display, uncomment the next line
// This has moved to MANAGEMENT SERVER

// to enable this focuser for ASCOMREMOTE support [Port 4040], uncomment the next line
// This has moved to MANAGEMENT SERVER

// [recommend use Internet Explorer or Microsoft Edge Browser]
// to enable WEBSERVER interface [Port 80], uncomment the next line 
// This has moved to MANAGEMENT SERVER

// To download the file [MANAGEMENTFORCEDOWNLOAD] instead of displaying file 
// content in web browser, uncomment the next line
// This has moved to MANAGEMENT SERVER

// ======================================================================
// DO NOT CHANGE: OPTIONS DRIVER BOARD CHECKS
// ======================================================================

#ifndef DRVBRD
#halt // ERROR you must have DRVBRD defined in myBoards.h
#endif

#if defined(USE_SSD1306) && defined(USE_SSH1106)
#halt //Error - you can must define either USE_SSD1306 or USE_SSH1106 if using an OLEDDISPLAY
#endif

#ifndef USE_SSD1306
#ifndef USE_SSH1106
#halt //Error - you  must define either USE_SSD1306 or USE_SSH1106 if using an OLEDDISPLAY
#endif
#endif

// DO NOT CHANGE
#if (DRVBRD == WEMOSDRV8825 || DRVBRD == PRO2EDRV8825 || DRVBRD == PRO2EDRV8825BIG \
  || DRVBRD == PRO2EULN2003 || DRVBRD == PRO2EL298N   || DRVBRD == PRO2EL293DMINI \
  || DSRVBRD == PRO2EL9110S || DRVBRD == PRO2EL293D   || DRVBRD == PRO2ESP32R3WEMOS )
// no support for pushbuttons, inout leds, irremote
#ifdef PUSHBUTTONS
#halt // ERROR - PUSHBUTTONS not supported for WEMOS or NODEMCUV1 ESP8266 chips
#endif
#ifdef INFRAREDREMOTE
#halt // ERROR - INFRAREDREMOTE not supported for WEMOS or NODEMCUV1 ESP8266 chips
#endif
#if defined(JOYSTICK1) || defined(JOYSTICK2)
#halt // ERROR - JOYSTICK not supported for WEMOS or NODEMCUV1 ESP8266 chips
#endif
#endif // 

#if defined(JOYSTICK1) || defined(JOYSTICK2)
#ifdef PUSHBUTTONS
#halt // ERROR - you cannot have PUSHBUTTONS and JOYSTICK enabled at the same time
#endif
#endif

#ifdef JOYSTICK1
#ifdef JOYSTICK2
#halt // ERROR - you cannot have both JOYSTICK1 or JOYSTICK2 defined at the same time
#endif
#endif

// DO NOT CHANGE
#if defined(MDNSSERVER)
#if defined(BLUETOOTHMODE) || defined(LOCALSERIAL) || defined(ACCESSPOINT)
#halt // ERROR, mDNS only available with STATIONMODE
#endif
#endif // MDNSSERVER

#if defined(OTAUPDATES)
#if defined(BLUETOOTHMODE) || defined(LOCALSERIAL)
#halt //ERROR you cannot have both OTAUPDATES with either BLUETOOTHMODE or LOCALSERIAL enabled at the same time
#endif
#if defined(ACCESSPOINT)
#halt //ERROR you cannot use ACCESSPOINT with OTAUPDATES
#endif
#endif

#ifdef ACCESSPOINT
#ifdef STATIONMODE
#halt // ERROR - Cannot have both ACCESSPOINT and STATIONMODE defined at the same time
#endif
#endif

#if defined(ACCESSPOINT) || defined(STATIONMODE)
#if defined(BLUETOOTHMODE)
#halt //ERROR you cannot have BLUETOOTHMODE with either ACCESSPOINT and STATIONMODE
#endif
#endif

#if defined(STATIONMODE) || defined(ACCESSPOINT) || defined(BLUETOOTHMODE)
#if defined(LOCALSERIAL)
#halt // ERROR - Cannot have both STATIONMODE/ACCESSPOINT/BLUETOOTHMODE and LOCALSERIAL mode defined at the same time
#endif
#endif

#if (DRVBRD == WEMOSDRV8825 || DRVBRD == PRO2EDRV8825 || DRVBRD == PRO2EDRV8825BIG \
  || DRVBRD == PRO2EULN2003 || DRVBRD == PRO2EL298N   || DRVBRD == PRO2EL293DMINI \
  || DSRVBRD == PRO2EL9110S || DRVBRD == PRO2EL293D )
// no support for bluetooth mode
#ifdef BLUETOOTHMODE
#halt // ERROR - BLUETOOTHMODE not supported for WEMOS or NODEMCUV1 ESP8266 chips
#endif
#endif

#if ((DRVBRD == PRO2EL293DNEMA) || (DRVBRD == PRO2EL293D28BYJ48))
#ifdef LOCALSERIAL
#halt // ERROR - LOCALSERIAL not supported L293D Motor Shield [ESP8266] boards
#endif
#endif

// cannot use DuckDNS with ACCESSPOINT, BLUETOOTHMODE or LOCALSERIAL mode
#ifdef USEDUCKDNS
#if defined(BLUETOOTHMODE) || defined(LOCALSERIAL) || defined(ACCESSPOINT)
#halt // Error- DUCKDNS only works with STATIONMODE
#endif
#ifndef STATIONMODE
#halt // Error- DUCKDNS only works with STATIONMODE, you must enable STATIONMODE
#endif
#endif

#ifdef LOCALSERIAL
#ifdef OTAUPDATES
#halt // Error Cannot enable OTAUPDATES with LOCALSERIAL
#endif
#ifdef MDNSSERVER
#halt // Error Cannot enable MDNSSERVER with LOCALSERIAL
#endif
#ifdef MANAGEMENT
#halt // Error Cannot enable MANAGEMENT with LOCALSERIAL
#endif
#ifdef DEBUG
#halt // Error Cannot enable DEBUG with LOCALSERIAL
#endif
#endif

#ifdef MANAGEMENT
#if !defined(ACCESSPOINT) && !defined(STATIONMODE)
#halt // ERROR You must have ACCESSPOINT or STATIONMODE defined to enable the MANAGEMENT console
#endif
#endif

#ifdef BLUETOOTHMODE
#if defined(ESP8266)
#halt // ERROR Bluetooth only available on ESP32 boards
#endif
#endif


#endif
