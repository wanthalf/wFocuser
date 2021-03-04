// ---------------------------------------------------------------------------
// TITLE: myFP2ESP BOARD TYPE DEFINITIONS
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// COPYRIGHT
// ---------------------------------------------------------------------------
// (c) Copyright Robert Brown 2014-2020. All Rights Reserved.
// (c) Copyright Holger M, 2019-2020. All Rights Reserved.
// ---------------------------------------------------------------------------

#ifndef myBoardTypes_h
#define myBoardTypes_h

// Definition for board types and general board definitions like stepmode and motorspeeds

// ---------------------------------------------------------------------------
// 1: BOARD DEFINES -- DO NOT CHANGE
// ---------------------------------------------------------------------------
#ifndef WEMOSDRV8825
#define WEMOSDRV8825          35         // if using a drv8825 you also need to set DRV8825STEPMODE in myBoards.h
#endif
#ifndef PRO2EDRV8825                     
#define PRO2EDRV8825          36         // if using a drv8825 you also need to set DRV8825STEPMODE in myBoards.h
#endif
#ifndef PRO2EDRV8825BIG                  // PRO2EDRV8825BIG Board is now deprecated
#define PRO2EDRV8825BIG       37         // if using a drv8825 you also need to set DRV8825STEPMODE in myBoards.h
#endif
#ifndef PRO2EULN2003
#define PRO2EULN2003          38
#endif
#ifndef PRO2EL293DNEMA
#define PRO2EL293DNEMA        39        // Motor shield ESP8266 with NEMA motor
#endif
#ifndef PRO2EL293D28BYJ48
#define PRO2EL293D28BYJ48     40        // Motor shield ESP8266 with 28BYJ48 motor
#endif
#ifndef PRO2EL298N
#define PRO2EL298N            41         // uses PCB layout for ULN2003
#endif
#ifndef PRO2EL293DMINI
#define PRO2EL293DMINI        42         // uses PCB layout for ULN2003
#endif
#ifndef PRO2EL9110S
#define PRO2EL9110S           43         // uses PCB layout for ULN2003
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
#define PRO2ESP32L293DMINI    47         // uses PCB layout for ULN2003
#endif
#ifndef PRO2ESP32L9110S
#define PRO2ESP32L9110S       48         // uses PCB layout for ULN2003
#endif
#ifndef PRO2ESP32R3WEMOS
#define PRO2ESP32R3WEMOS      49         // https://www.ebay.com/itm/R3-Wemos-UNO-D1-R32-ESP32-WIFI-Bluetooth-CH340-Devolopment-Board-For-Arduino/264166013552
#endif

#endif
