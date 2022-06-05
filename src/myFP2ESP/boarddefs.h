// ======================================================================
// boarddefs.h
// DEFINES FOR BOARD TYPES
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

#ifndef boarddefs_h
#define boarddefs_h

// ======================================================================
// TMCxxxx DEFINES
// ======================================================================
#define STALL_VALUE         150           // [0... 255]
#define TMC2209CURRENT      400           // 600mA for 8HS15-0604S NEMA8 stepper motor with tmc2209
#define TMC2225CURRENT      300           // 300mA for recommended stepper NEMA motor - you can change this - with tmc2225
#define TMC2225SPEED        57600
#define TMC2209SPEED        57600
#define TOFF_VALUE          3             // [1... 15]
#define DRIVER_ADDRESS      0b00          // TMC2209 Driver address according to MS1 and MS2
#define R_SENSE             0.11f         // Match to your driver
// SilentStepStick series use 0.11
// UltiMachine Einsy and Archim2 boards use 0.2
// Panucatt BSD2660 uses 0.1
// Watterott TMC5160 uses 0.075

// ---------------------------------------------------------------------------
// BOARD TYPE DEFINES
// ---------------------------------------------------------------------------      

// myFP2 Boards
#define A4998                 0
#define AMSV2                 1   // ADAfruit motor shield v2
#define DRV8825               2
#define DRV8825RE             3
#define DRV8825TFT22          4
#define DRV8825SOLDERLESS     5  
#define DRV8825HW203          6 
#define DRV8825HW203KEYPAD    7         
#define DRV8825HW203TFT22     8
#define EASYDRIVERPK          9
#define EASYDRIVERRE          10
#define L293D                 11
#define L293DMINI             12
#define L293DMININOKIA        13
#define L293DMINITFT22        14
#define L298N                 15
#define L298P                 16
#define L9110S                17
#define MYFP2TMC2225          18
#define MYFP2TMC2209          19
#define L9110SNOKIA           20
#define RAPS128               21
#define ST6128                22
#define ST6128DHT22           23
#define TB6612FNG             24
#define ULN2003               25  
#define ULN20038S             26
#define ULN2003H              27
#define ULN2003HNOKIA         28
#define MFP2CLOSEDLOOP        29

// myFP2N boards
#define MYFP2NDRV8825         30
#define MYFP2NULN2003         31
#define MYFP2NTMC2225         32
#define MYFP2NTMC2209         33

// myFP2ESP boards
#define WEMOSDRV8825          35          // if using a drv8825 you also need to set DRV8825STEPMODE in myBoards.h         
#define PRO2EDRV8825          36          // if using a drv8825 you also need to set DRV8825STEPMODE in myBoards.h
#define PRO2EDRV8825BIG       37          // if using a drv8825 you also need to set DRV8825STEPMODE in myBoards.h
#define PRO2EULN2003          38
#define PRO2EL293DNEMA        39          // Motor shield ESP8266 with NEMA motor
#define PRO2EL293D28BYJ48     40          // Motor shield ESP8266 with 28BYJ48 motor
#define PRO2EL298N            41          // uses PCB layout for ULN2003
#define PRO2EL293DMINI        42          // uses PCB layout for ULN2003
#define PRO2EL9110S           43          // uses PCB layout for ULN2003
#define PRO2ESP32DRV8825      44
#define PRO2ESP32ULN2003      45
#define PRO2ESP32L298N        46
#define PRO2ESP32L293DMINI    47          // uses PCB layout for ULN2003
#define PRO2ESP32L9110S       48          // uses PCB layout for ULN2003
#define PRO2ESP32R3WEMOS      49          // https://www.ebay.com/itm/R3-Wemos-UNO-D1-R32-ESP32-WIFI-Bluetooth-CH340-Development-Board-For-Arduino/264166013552 
#define WEMOSDRV8825H         50          // this is for Holger
#define PRO2ESP32TMC2225      56
#define PRO2ESP32TMC2209      57
#define PRO2ESP32TMC2209P     58          // This is board for Paul using TMC2209 for testing
#define PRO2ESP32ST6128       59          // This is board for CLOSED LOOP ST6128 driver

// myFP2M boards
#define PRO2MDRV8825          52
#define PRO2MULN2003          53
#define PRO2ML293DMINI        54
#define PRO2ML298N            55

#define CUSTOMBRD             99          // For a user custom board see 0.jsn in /data/boards folder

// ---------------------------------------------------------------------------
// 2: STEP MODE DEFINES
// ---------------------------------------------------------------------------            
#ifndef STEP1
#define STEP1                 1             // stepmodes
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

// ---------------------------------------------------------------------------
// 3: SPEED DEFINES
// ---------------------------------------------------------------------------    
#ifndef SLOW
#define SLOW                  0             // motorspeeds
#endif
#ifndef MED
#define MED                   1
#endif
#ifndef FAST
#define FAST                  2
#endif


#endif
