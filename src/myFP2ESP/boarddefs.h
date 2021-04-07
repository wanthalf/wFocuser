// ======================================================================
// boarddefs.h : myFP2ESP BOARD DEFINITIONS
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

// DO NOT CHANGE

#ifndef boarddefs_h
#define boarddefs_h

#include <Arduino.h>

// ======================================================================
// DEFINES FOR BOARD TYPES
// ======================================================================
#ifndef CUSTOMBRD
#define CUSTOMBRD             99          // For a user custom board see 0.jsn in /data/boards folder
#endif
#ifndef WEMOSDRV8825
#define WEMOSDRV8825          35          // if using a drv8825 you also need to set DRV8825STEPMODE in myBoards.h
#endif
#ifndef PRO2EDRV8825                     
#define PRO2EDRV8825          36          // if using a drv8825 you also need to set DRV8825STEPMODE in myBoards.h
#endif
#ifndef PRO2EDRV8825BIG                   // PRO2EDRV8825BIG Board is now deprecated
#define PRO2EDRV8825BIG       37          // if using a drv8825 you also need to set DRV8825STEPMODE in myBoards.h
#endif
#ifndef PRO2EULN2003
#define PRO2EULN2003          38
#endif
#ifndef PRO2EL293DNEMA
#define PRO2EL293DNEMA        39          // Motor shield ESP8266 with NEMA motor
#endif
#ifndef PRO2EL293D28BYJ48
#define PRO2EL293D28BYJ48     40          // Motor shield ESP8266 with 28BYJ48 motor
#endif
#ifndef PRO2EL298N
#define PRO2EL298N            41          // uses PCB layout for ULN2003
#endif
#ifndef PRO2EL293DMINI
#define PRO2EL293DMINI        42          // uses PCB layout for ULN2003
#endif
#ifndef PRO2EL9110S
#define PRO2EL9110S           43          // uses PCB layout for ULN2003
#endif
#ifndef PRO2ESP32DRV8825
#define PRO2ESP32DRV8825      44
#endif
#ifndef PRO2ESP32ULN2003
#define PRO2ESP32ULN2003      45
#endif
#ifndef PRO2ESP32L298N
#define PRO2ESP32L298N        46
#endif
#ifndef PRO2ESP32L293DMINI
#define PRO2ESP32L293DMINI    47          // uses PCB layout for ULN2003
#endif
#ifndef PRO2ESP32L9110S
#define PRO2ESP32L9110S       48          // uses PCB layout for ULN2003
#endif
#ifndef PRO2ESP32R3WEMOS
#define PRO2ESP32R3WEMOS      49          // https://www.ebay.com/itm/R3-Wemos-UNO-D1-R32-ESP32-WIFI-Bluetooth-CH340-Devolopment-Board-For-Arduino/264166013552
#endif
#ifndef WEMOSDRV8825H
#define WEMOSDRV8825H         50          // this is for Holger
#endif
#ifndef PRO2ESP32TMC2225
#define PRO2ESP32TMC2225      51
#endif
#ifndef PRO2ESP32TMC2209
#define PRO2ESP32TMC2209      52        
#endif

// ======================================================================
// DEFINES FOR MOTOR SPEEDS
// ======================================================================

#ifndef SLOW
#define SLOW                  0           // motorspeeds
#endif
#ifndef MED
#define MED                   1
#endif
#ifndef FAST
#define FAST                  2
#endif

// ======================================================================
// DEFINES FOR MOTOR MICRO-STEPS
// ======================================================================

#ifndef STEP1
#define STEP1                 1           // stepmodes
#endif
#ifndef STEP2
#define STEP2                 2
#endif
#ifndef STEP4
#define STEP4                 4
#endif
#ifndef STEP8
#define STEP8                 8
#endif
#ifndef STEP16
#define STEP16                16
#endif
#ifndef STEP32
#define STEP32                32
#endif
#ifndef STEP64
#define STEP64                64
#endif
#ifndef STEP128
#define STEP128               128
#endif
#ifndef STEP256
#define STEP256               256
#endif




#endif
