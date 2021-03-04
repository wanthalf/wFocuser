// ULN2003 TEST PROGRAM
// DOES NOT WORK
//
// ---------------------------------------------------------------------------
// COPYRIGHT
// ---------------------------------------------------------------------------
// (c) Copyright Robert Brown 2014-2020. All Rights Reserved.
// (c) Copyright Holger M, 2019-2020. All Rights Reserved.
// (c) Copyright Pieter P - OTA code/SPIFFs file handling/upload based on examples

// ---------------------------------------------------------------------------
// SPECIAL LICENSE
// ---------------------------------------------------------------------------
// This code is released under license. If you copy or write new code based on
// the code in these files. you MUST include to link to these files AND you MUST
// include references to the authors of this code.

// ---------------------------------------------------------------------------
// CONTRIBUTIONS
// ---------------------------------------------------------------------------
// It is costly to continue development and purchase boards and components.
// Your support is needed to continue development of this project. Please
// contribute to this project, and use PayPal to send your donation to user
// rbb1brown@gmail.com (Robert Brown). All contributions are gratefully accepted.

// ---------------------------------------------------------------------------
// 1: SPECIFY DRIVER BOARD in myBoards.h
// ---------------------------------------------------------------------------
// Please specify your driver board in myBoards.h, only 1 can be defined,
// see DRVBRD line

#include "generalDefinitions.h"
#include "myBoards.h"

// ---------------------------------------------------------------------------
// 6: INCLUDES
// ---------------------------------------------------------------------------


// ----------------------------------------------------------------------------------------
// 15: GLOBAL DATA -- DO NOT CHANGE
// ----------------------------------------------------------------------------------------




String programName;
DriverBoard* driverboard;

volatile bool halt_alert;
int steps;
int spd = 2;
bool ledstate = false;
bool DirOfTravel = true;

void steppermotormove(byte dir )                // direction move_in, move_out ^ reverse direction
{
  driverboard->movemotor(dir);
}

void setup()
{
  Serial.begin(SERIALPORTSPEED);
  delay(100);                                   // keep delays small otherwise issue with ASCOM

  driverboard = new DriverBoard(PRO2ESP32ULN2003);
  delay(5);
}

extern volatile uint32_t stepcount;             // number of steps to go in timer interrupt service routine
extern volatile bool timerSemaphore;

void loop()
{
  Serial.print("Move motor 2048 steps forward");
  steps = 2048;
  driverboard->initmove(DirOfTravel, steps, spd, ledstate);
  while( stepcount > 0 )
  ;
  Serial.println("Move completed");
  
  Serial.print("Move motor 2048 steps reverse");      
  steps = 2048;
  driverboard->initmove(!DirOfTravel, steps, spd, ledstate);
  while( stepcount > 0 )
  ;
  
} // end Loop()
