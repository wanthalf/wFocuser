// ======================================================================
// focuserconfig.h : myFP2ESP FOCUSER CONFIG DEFINITIONS
// SPECIFY HARDWARE OPTIONS AND CONTROLLER MODES HERE
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

#include <Arduino.h>
#include "boarddefs.h"
#include "generalDefinitions.h"

#ifndef focuserconfig_h
#define focuserconfig_h

// Caution: Do not enable a feature if the associated hardware circuits are 
// not fitted on the board. Enable or disable the specific options below

// ======================================================================
// 1: BOARD DEFINES [do not change names]
// ======================================================================
// Uncomment only your board - ONLY ONE BOARD SHOULD BE UNCOMMENTED

// ESP32 Boards
//#define DRVBRD 	PRO2ESP32DRV8825
#define DRVBRD 	PRO2ESP32ULN2003
//#define DRVBRD 	PRO2ESP32L298N
//#define DRVBRD 	PRO2ESP32L293DMINI
//#define DRVBRD 	PRO2ESP32L9110S
//#define DRVBRD 	PRO2ESP32R3WEMOS
//#define DRVBRD  PRO2ESP32TMC2225
//#define DRVBRD  PRO2ESP32TMC2209
//#define DRVBRD  PRO2ESP32TMC2209P              // this is for Paul using TMC2209 - 58.jsn
//#define DRVBRD 	CUSTOMBRD

// On bootup following a controller firmware update, a default driver board file 
// is created based on your board selection above.
// In the MANAGEMENT server you can edit the board pin numbers and save the config [only if you know what you are doing].

// ======================================================================
// 2: SPECIFY FIXEDSTEPMODE
// ======================================================================
// For ESP8266 boards, set the fixedmode value to match the stepmode jumper
// settings on the board [only one line can be enabled]
// Applies to boards WEMOSDRV8825H, WEMOSDRV8825, PRO2EDRV8825BIG, PRO2EDRV8825
#define FIXEDSTEPMODE   	1
//#define FIXEDSTEPMODE 	2     
//#define FIXEDSTEPMODE 	4
//#define FIXEDSTEPMODE 	8
//#define FIXEDSTEPMODE 	16
//#define FIXEDSTEPMODE 	32
//#define FIXEDSTEPMODE 	64            // for future release
//#define FIXEDSTEPMODE 	128           // for future release
//#define FIXEDSTEPMODE 	256           // for future release

// ======================================================================
// 3: SPECIFY STEPS PER REVOLUTION
// ======================================================================
// stepper motor steps per full revolution using full steps, applies to boards
// PRO2EULN2003, PRO2ESP32ULN2003, PRO2EL298N, PRO2ESP32L298N
// PRO2EL293DMINI, PRO2ESP32L293MINI, PRO2EL9110S, PRO2ESPL9110S
// PRO2EL293DNEMA, PRO2EL293D28BYJ48

#define STEPSPERREVOLUTION 		2048           // 28BYJ-48 stepper motor unipolar with ULN2003 board
//#define STEPSPERREVOLUTION  	200        // NEMA17 FULL STEPPED
//#define STEPSPERREVOLUTION  	400        // NEMA14HM11-0404S 0.9 motor FULL STEPPED
//#define STEPSPERREVOLUTION 	1028        // 17HS13-0404S-PG5
//#define STEPSPERREVOLUTION 	5370        // NEMA17HS13-0404S-PG27
//#define STEPSPERREVOLUTION 	1036        // NEMA14HS13-0804S-PG5
//#define STEPSPERREVOLUTION 	1036        // NEMA16HS13-0604S-PG5

// to work only via USB cable as Serial port, uncomment the next line
#define CONTROLLERMODE  LOCALSERIAL

// ======================================================================
// DO NOT CHANGE:
// ======================================================================

// ======================================================================
// CHECK BOARD AND HW OPTIONS
// ======================================================================
#ifndef DRVBRD
#halt // ERROR you must have DRVBRD defined in myBoards.h
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
#endif // #if defined(JOYSTICK1) || defined(JOYSTICK2)

// Check board availability for a specific controller mode
#if (DRVBRD == WEMOSDRV8825 || DRVBRD == PRO2EDRV8825 || DRVBRD == PRO2EDRV8825BIG \
  || DRVBRD == PRO2EULN2003 || DRVBRD == PRO2EL298N   || DRVBRD == PRO2EL293DMINI \
  || DSRVBRD == PRO2EL9110S || DRVBRD == PRO2EL293D )
// no support for bluetooth mode
#if (CONTROLLERMODE == BLUETOOTHMODE)
#halt // ERROR - BLUETOOTHMODE not supported for WEMOS or NODEMCUV1 ESP8266 chips
#endif
#endif

#if (DRVBRD == PRO2EL293DNEMA) || (DRVBRD == PRO2EL293D28BYJ48)
#if (CONTROLLERMODE == LOCALSERIAL)
#halt // ERROR - LOCALSERIAL not supported L293D Motor Shield [ESP8266] boards
#endif
#endif // #if ((DRVBRD == PRO2EL293DNEMA) || (DRVBRD == PRO2EL293D28BYJ48))

#if defined(JOYSTICK1) || defined(JOYSTICK2)
#ifdef PUSHBUTTONS
#halt // ERROR - you cannot have PUSHBUTTONS and JOYSTICK enabled at the same time
#endif
#endif // #if defined(JOYSTICK1) || defined(JOYSTICK2)

#ifdef JOYSTICK1
#ifdef JOYSTICK2
#halt // ERROR - you cannot have both JOYSTICK1 or JOYSTICK2 defined at the same time
#endif
#endif // #ifdef JOYSTICK1

// ======================================================================
// CHECK CONTROLLER OPTIONS
// ======================================================================

#if defined(OTAUPDATES)
#if (CONTROLLERMODE == BLUETOOTHMODE) || (CONTROLLERMODE == LOCALSERIAL)
#halt //ERROR you cannot have both OTAUPDATES with either BLUETOOTHMODE or LOCALSERIAL enabled at the same time
#endif
#if (CONTROLLERMODE == ACCESSPOINT)
#halt //ERROR you cannot use ACCESSPOINT with OTAUPDATES
#endif
#endif // #if defined(OTAUPDATES)

#if defined(MDNSSERVER)
#if (CONTROLLERMODE == BLUETOOTHMODE) || (CONTROLLERMODE == LOCALSERIAL) || (CONTROLLERMODE == ACCESSPOINT)
#halt // ERROR, mDNS only available with CONTROLLERMODE == STATIONMODE
#endif
#endif // MDNSSERVER

// Check management server only available in accesspoint or stationmode
#ifdef MANAGEMENT
#if (CONTROLLERMODE == BLUETOOTHMODE) || (CONTROLLERMODE == LOCALSERIAL)
#halt // ERROR You cannot run the MANAGEMENT service in Bluetooth or Local Serial modes
#endif
#endif

// cannot use DuckDNS with ACCESSPOINT, BLUETOOTHMODE or LOCALSERIAL mode
#ifdef USEDUCKDNS
#if (CONTROLLERMODE == BLUETOOTHMODE) || (CONTROLLERMODE == LOCALSERIAL) || (CONTROLLERMODE == ACCESSPOINT)
#halt // Error- DUCKDNS only works with STATIONMODE
#endif
#ifndef STATIONMODE
#halt // Error- DUCKDNS only works with STATIONMODE, you must enable STATIONMODE
#endif
#endif

// DO NOT CHANGE
#if defined(READWIFICONFIG)
#if (CONTROLLERMODE == BLUETOOTH) || (CONTROLLERMODE == LOCALSERIAL) || (CONTROLLERMODE == ACCESSPOINT)
#halt // ERROR, READWIFICONFIG only available with CONTROLLERMODE == STATIONMODE
#endif
#endif // #if defined(READWIFICONFIG)


// ======================================================================
// CHECK CONTROLLER MODES
// ======================================================================

#if !defined(CONTROLLERMODE) 
#halt // Error CONTROLLERMODE NOT DEFINED
#endif

// check bluetooth mode, cannot be used for esp8266 and accesspoint and stationmode and localserial
#if (CONTROLLERMODE == BLUETOOTHMODE)
#if defined(ESP8266)
#halt // ERROR Bluetooth only available on ESP32 boards
#endif
#ifdef OTAUPDATES
#halt // Error Cannot enable OTAUPDATES with BLUETOOTHMODE
#endif
#ifdef MDNSSERVER
#halt // Error Cannot enable MDNSSERVER with BLUETOOTHMODE
#endif
#ifdef MANAGEMENT
#halt // Error Cannot enable MANAGEMENT with BLUETOOTHMODE
#endif
#ifdef READWIFICONFIG
#halt // ERROR, Cannot enabled READWIFICONFIG with BLUETOOTHMODE
#endif
#ifdef USEDUCKDNS
#halt // ERROR, Cannot enable DUCKDNS with BLUETOOTHMODE
#endif
#endif // #if (CONTROLLERMODE == BLUETOOTHMODE)

// check localserial mode
#if (CONTROLLERMODE == LOCALSERIAL)
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
#ifdef READWIFICONFIG
#halt // ERROR, READWIFICONFIG only available with CONTROLLERMODE == STATIONMODE
#endif
#ifdef USEDUCKDNS
#halt // ERROR, Cannot enable DUCKDNS with BLUETOOTHMODE
#endif
#endif // #if (CONTROLLERMODE == LOCALSERIAL)

// check accesspoint mode
#if (CONTROLLERMODE == ACCESSPOINT)
#ifdef MDNSSERVER
#halt // Error Cannot enable MDNSSERVER with ACCESSPOINT
#endif
#ifdef OTAUPDATES
#halt // Error Cannot enable OTAUPDATES with ACCESSPOINT
#endif
#ifdef USEDUCKDNS
#halt // ERROR, Cannot enable DUCKDNS with ACCESSPOINT
#endif
#endif // #if (CONTROLLERMODE == ACCESSPOINT)


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

// To make the firmware return the correct firmware value when talking to a
// myFocuserpro2 INDI driver [use only for INDI support], uncomment the following line
// This has moved to MANAGEMENT SERVER

#endif
