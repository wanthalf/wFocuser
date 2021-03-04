#include <Arduino.h>

#ifndef myBoardTypes_h
#define myBoardTypes_h

// Definition for board types and general board definitions like stepmode and motorspeeds

// ----------------------------------------------------------------------------------------------
// 1: BOARD DEFINES -- DO NOT CHANGE
// ----------------------------------------------------------------------------------------------
#ifndef PRO2EDRV8825
#define PRO2EDRV8825          35         // if using a drv8825 you also need to set DRV8825STEPMODE in myBoards.h
#endif
#ifndef PRO2EULN2003
#define PRO2EULN2003          36
#endif
#ifndef PRO2EL293D
#define PRO2EL293D            37
#endif
#ifndef PRO2EL298N
#define PRO2EL298N            38         // uses PCB layout for ULN2003
#endif
#ifndef PRO2EL293DMINI
#define PRO2EL293DMINI        39         // uses PCB layout for ULN2003
#endif
#ifndef PRO2EL9110S
#define PRO2EL9110S           40         // uses PCB layout for ULN2003
#endif
#ifndef PRO2ESP32DRV8825
#define PRO2ESP32DRV8825      41
#endif
#ifndef PRO2ESP32ULN2003
#define PRO2ESP32ULN2003      42
#endif
#ifndef PRO2ESP32L298N
#define PRO2ESP32L298N        43
#endif
#ifndef PRO2ESP32L293DMINI
#define PRO2ESP32L293DMINI    44         // uses PCB layout for ULN2003
#endif
#ifndef PRO2ESP32L9110S
#define PRO2ESP32L9110S       45         // uses PCB layout for ULN2003
#endif

#endif
