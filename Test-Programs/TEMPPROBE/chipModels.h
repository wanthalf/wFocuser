#include <Arduino.h>

#ifndef chipModels_h
#define chipModels_h

// ----------------------------------------------------------------------------------------------
// 0: DEFINE CHIP MODEL HERE
// ----------------------------------------------------------------------------------------------
// YOU MUST SELECT (uncomment one) the correct chip model that matches your PCB
//#define CHIPMODEL WEMOS
#define CHIPMODEL NODEMCUV1
//#define CHIPMODEL ESP32WROOM

// Hardware Definitions of All Pins used byeach CPU board
// Option pins listed first (temperature etc), followed by driver board pins and definitions
// ----------------------------------------------------------------------------------------------
// WEMOS ESP8266
// ----------------------------------------------------------------------------------------------
#if( CHIPMODEL == WEMOS)                  // ESP8266
#define CHIPESP8266           1
#define TEMPPIN               2           // ds18b20 temperature probe WEMOS 
#define OLED_ADDR             0x3C        // some OLED displays maybe at 0x3F, use I2Cscanner to find the correct address#endif // wemos
#define I2CDATAPIN            2
#define I2CCLOCKPIN           1
#define DRV8825DIR            13
#define DRV8825STEP           12
#define DRV8825ENABLE         14
#define MOTORPULSETIME        2           // must be al least 2us, time in microseconds
#define DRVFAST               1           // delays for motorspeed in microseconds
#define DRVMED                2000
#define DRVSLOW               10000

// ----------------------------------------------------------------------------------------------
// NODEMCUV1 ESP8266
// ----------------------------------------------------------------------------------------------
#elif  ( CHIPMODEL == NODEMCUV1)          // ESP8266
#define CHIPESP8266           1
#define TEMPPIN               10          // ds18b20 temperature probe 
#define OLED_ADDR             0x3C        // some OLED displays maybe at 0x3F, use I2Cscanner to find the correct address#endif // wemos
#define I2CDATAPIN            5
#define I2CCLOCKPIN           4
#define DRV8825DIR            13
#define DRV8825STEP           12
#define DRV8825ENABLE         14
#define MOTORPULSETIME        2           // must be al least 2us, time in microseconds
#define DRVFAST               1           // delays for motorspeed in microseconds
#define DRVMED                2000
#define DRVSLOW               10000
#define IN1ULN                13
#define IN2ULN                12
#define IN3ULN                14
#define IN4ULN                2
#define ULNFAST               1           // delays for motorspeed in microseconds [250-1000]
#define ULNMED                1000
#define ULNSLOW               8000        // do not use values > 15000 due to accuracy issues
// GPIO13-D7-J2-IN1 - IN1
// GPIO12-D6-J2-IN2 - IN2
// GPIO14-D5-J2-IN3 - IN3
// GPIO02-D4-J2-IN4 - IN4
#define IN1L298N              13
#define IN2L298N              12
#define IN3L298N              14
#define IN4L298N              2
#define L298NFAST             1           // delays for motorspeed in microseconds [250-1000]
#define L298NMED              1000
#define L298NSLOW             8000        // do not use values > 15000 due to accuracy issues
// GPIO13-D7-J2-IN1 - IN1
// GPIO12-D6-J2-IN2 - IN2
// GPIO14-D5-J2-IN3 - IN3
// GPIO02-D4-J2-IN4 - IN4
#define IN1L293DMINI          13
#define IN2L293DMINI          12
#define IN3L293DMINI          14
#define IN4L293DMINI          2
#define L293DMINIFAST         1           // delays for motorspeed in microseconds [250-1000]
#define L293DMINIMED          1000
#define L293DMINISLOW         8000        // do not use values > 15000 due to accuracy issues
// GPIO13-D7-J2-IN1 - A1A
// GPIO12-D6-J2-IN2 - A1B
// GPIO14-D5-J2-IN3 - B1A
// GPIO02-D4-J2-IN4 - B1B
#define IN1L9110S             13
#define IN2L9110S             12
#define IN3L9110S             14
#define IN4L9110S             2
#define L9110SFAST            1           // delays for motorspeed in microseconds [250-1000]
#define L9110SMED             1000
#define L9110SSLOW            8000        // do not use values > 15000 due to accuracy issues

// ----------------------------------------------------------------------------------------------
// DEFINES FOR ESP32 BOARDS
// ----------------------------------------------------------------------------------------------
#elif( CHIPMODEL == ESP32WROOM)           // ESP32 WROOM
#define CHIPESP32             2
#define I2CDATAPIN            21        // i2c for oled 
#define I2CCLOCKPIN           22
#define TEMPPIN               13        // ds18b20 temperature probe
#define INLED                 18        // IN direction LED, prewired
#define OUTLED                19        // OUT direction LED, prewired
#define INPB                  34        // has 10K pulldown resistor, so will be active high
#define OUTPB                 35        // has 10K pulldown resistor, so will be active high
#define IRPIN                 15        // Infra-red remote controller 
#define OLED_ADDR             0x3C      // some OLED displays maybe at 0x3F, use I2Cscanner to find the correct address
#define DRV8825DIR            32        // drv8825 driver board
#define DRV8825STEP           33        // drv8825 driver board
#define DRV8825ENABLE         14        // drv8825 driver board
#define DRV8825M3             25        // drv8825 driver board
#define DRV8825M2             26        // drv8825 driver board
#define DRV8825M1             27        // drv8825 driver board
#define MOTORPULSETIME        5         // drv8825 driver board
#define DRVFAST               500       // delays for motorspeed
#define DRVMED                1000
#define DRVSLOW               2000
#define IN1ULN                14
#define IN2ULN                27
#define IN3ULN                26
#define IN4ULN                25
#define ULNFAST               1                 // delays for motorspeed in microseconds [250-1000]
#define ULNMED                1000
#define ULNSLOW               8000              // do not use values > 15000 due to accuracy issues
// 14 - IN1
// 27 - IN2
// 26 - IN3
// 25 - IN4
#define IN1L298N              14
#define IN2L298N              27
#define IN3L298N              26
#define IN4L298N              25
#define L298NFAST             1                 // delays for motorspeed in microseconds [250-1000]
#define L298NMED              1000
#define L298NSLOW             8000              // do not use values > 15000 due to accuracy issues
// 14 - IN1
// 27 - IN2
// 26 - IN3
// 25 - IN4
#define IN1L293DMINI          14
#define IN2L293DMINI          27
#define IN3L293DMINI          26
#define IN4L293DMINI          25
#define L293DMINIFAST         1                 // delays for motorspeed in microseconds [250-1000]
#define L293DMINIMED          1000
#define L293DMINISLOW         8000              // do not use values > 15000 due to accuracy issues
// 14-IN1 - A1A
// 27-IN2 - A1B
// 26-IN3 - B1A
// 25-IN4 - B1B
#define IN1L9110S             14
#define IN2L9110S             27
#define IN3L9110S             26
#define IN4L9110S             25
#define L9110SFAST            1                 // delays for motorspeed in microseconds [250-1000]
#define L9110SMED             1000
#define L9110SSLOW            8000              // do not use values > 15000 due to accuracy issues
#endif
// ----------------------------------------------------------------------------------------------

#endif // chipModels.h
