// ULN2003 TEST PROGRAM
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

#include <Arduino.h>
//#include <HalfStepperESP32.h>
#include <Stepper.h>

#define STEPSPERREVOLUTION 2048
// PRO2EULS2003
//#define TEMPPIN       10
//#define I2CDATAPIN    5
//#define I2CCLKPIN     4
//#define IN1           13
//#define IN2           12
//#define IN3           14
//#define IN4           2
//#define MSPEED        16500
// IN1ULN, IN3ULN, IN4ULN, IN2ULN

// PRO2ESP32ULN2003
#define IN1           14
#define IN2           27
#define IN3           26
#define IN4           25
#define MSPEED        16500
Stepper mystepper(STEPSPERREVOLUTION, IN1, IN3, IN4, IN2);

int inputPins[4];

void init_board()
{
  Serial.println("init_board()");
  inputPins[0] = IN1;
  inputPins[1] = IN2;
  inputPins[2] = IN3;
  inputPins[3] = IN4;
  for (int inputCount = 0; inputCount < 4; inputCount++)
  {
    pinMode(inputPins[inputCount], OUTPUT);
  }

}

void release_motor()
{
  Serial.println("release_motor()");
  for (int inputCount = 0; inputCount < 4; inputCount++)
  {
    digitalWrite(inputPins[inputCount], 0);
  }
}

void setup()
{
  Serial.begin(115200);
  delay(100);                                   // keep delays small otherwise issue with ASCOM
  init_board();
  release_motor();
}

void loop()
{
  unsigned long startt, endt;

  mystepper.setSpeed(5);

  Serial.println("-------------------------------------");
  Serial.println("Starting Test1a: 2048 steps : FULL");
  //set_fullstep();
  for (int cnt = 0; cnt < 2048; cnt++ )
  {
    mystepper.step(1);
    delay(10);
  }
  delay(2000);
  Serial.println("Starting Test1b: 2048 steps REVERSE: FULL");
  //set_fullstep();
  for (int cnt = 0; cnt < 2048; cnt++ )
  {
    mystepper.step(-1);
    delay(10);
  }
  delay(2000);
  Serial.println("Starting Test3: 1step : FULL : Timed");
  startt = millis();
  mystepper.step(1);
  endt = millis() - startt;
  Serial.print("Time taken for one step: ");
  Serial.print(endt);
  Serial.println(" milliseconds");
  Serial.println("Going back 1 step");
  mystepper.step(-1);
  delay(2000);
  Serial.println("Starting Test4a: 2048 steps : FULL : step(2048)");
  mystepper.step(2048);
  Serial.println("Starting Test4a: 2048 steps REVERSE: FULL : step(-2048)");
  mystepper.step(-2048);
  delay(5000);
} // end Loop()
