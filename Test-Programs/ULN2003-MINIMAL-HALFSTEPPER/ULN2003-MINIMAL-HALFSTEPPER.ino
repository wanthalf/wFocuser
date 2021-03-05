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

/*
  NOTES
  SetDirection does not work
  mystepper.step(2048); is a blocking call and control does not continue till the move is finished
  HalfStepper.h works with HalfStepper mystepper(STEPSPERREVOLUTION, IN1, IN3, IN4, IN2);
  HalfStepperESP32.h works with HalfStepper mystepper(STEPSPERREVOLUTION, IN1, IN3, IN4, IN2);
*/

#include <Arduino.h>
#include <HalfStepperESP32.h>

#define STEPSPERREVOLUTION 2048
#ifdef (ESP8266) // PRO2EULS2003
#define TEMPPIN       10
#define I2CDATAPIN    5
#define I2CCLKPIN     4
#define IN1           13
#define IN2           12
#define IN3           14
#define IN4           2
#define MSPEED        16500
//HalfStepper* mystepper;
// IN1ULN, IN3ULN, IN4ULN, IN2ULN
#else
// ESP32
// PRO2ESP32ULN2003
#define IN1           14
#define IN2           27
#define IN3           26
#define IN4           25
#define MSPEED        16500
#endif

//HalfStepper mystepper(STEPSPERREVOLUTION, IN1, IN3, IN4, IN2);    // rotation is only one way, half stepping does not work
HalfStepper mystepper(STEPSPERREVOLUTION, IN1, IN2, IN3, IN4);      // rotates both directions, half stepping works

int inputPins[4];

#define FORWARD false
#define REVERSE true

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
  // FULL | HALF
  // DUAL | SINGLE
  // ALTERNATING | SEQUENTIAL
  // mystepper = new HalfStepper(STEPSPERREVOLUTION, IN1, IN3, IN4, IN2, FULL, DUAL, ALTERNATING);
  // mystepper = new HalfStepper(STEPSPERREVOLUTION, IN1, IN2, IN3, IN4);
  // mystepper = new Stepper(STEPSPERREVOLUTION, IN1, IN3, IN4, IN2);
  /*
    void setSpeed(long);
    int GetSpeedRPMs();
    int version(void);

    void SetDirection(Direction);
    Direction GetDirection() const;
    FORWARD = false,
    REVERSE = true

    void SetPosition(dword);
    dword GetPosition() const;

    void step(int);
    void Step(long = 1);

    void StepForward(dword = 1);
    void StepBackward(dword = 1);

    void StepTo(dword);

    word GetSpeedRPMs() const;

    void SetSteppingMode(SteppingMode);
    SteppingMode GetSteppingMode() const;
    FULL = false,
    HALF = true

    void SetPhasingMode(PhasingMode);
    PhasingMode GetPhasingMode() const;
    SINGLE = false,
    DUAL = true

    void SetSequenceType(SequenceType);
    SequenceType GetSequenceType() const;
    SEQUENTIAL = false,
    ALTERNATING = true
  */
}

void release_motor()
{
  Serial.println("release_motor()");
  for (int inputCount = 0; inputCount < 4; inputCount++)
  {
    digitalWrite(inputPins[inputCount], 0);
  }
}

void set_fullstep()
{
  mystepper.SetSteppingMode(SteppingMode::FULL);
}

void set_halfstep()
{
  mystepper.SetSteppingMode(SteppingMode::HALF);
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
  unsigned int ss;
  bool sm;
  bool pm;
  bool st;
  bool di;
  int po;

  mystepper.setSpeed(5);
  ss = mystepper.GetSpeedRPMs();
  sm = (boolean) mystepper.GetSteppingMode();
  pm = (boolean) mystepper.GetPhasingMode();
  st = (boolean) mystepper.GetSequenceType();
  di = (boolean) mystepper.GetDirection();
  po = mystepper.GetPosition();

  Serial.println("-------------------------------------");
  Serial.print("Position      : "); Serial.println(po);
  Serial.print("Direction     : "); Serial.println(di);
  Serial.print("Stepper speed : "); Serial.println(ss);
  Serial.print("Step Mode     : "); if ( sm ) Serial.println("HALF"); else Serial.println("FULL");
  Serial.print("Phasing mode  : "); if ( pm ) Serial.println("DUAL"); else Serial.println("SINGLE");
  Serial.print("Sequence Type : "); if ( st ) Serial.println("ALTERNATING"); else Serial.println("SEQUENTIAL");
  Serial.println("-------------------------------------");
  Serial.println("Starting Test1a: 2048 steps : FULL");
  set_fullstep();
  //mystepper.SetDirection((Direction)FORWARD);
  for (int cnt = 0; cnt < 2048; cnt++ )
  {
    mystepper.step(1);
    delay(10);
  }
  delay(2000);
  Serial.println("Starting Test1b: 2048 steps REVERSE: FULL");
  set_fullstep();
  //mystepper.SetDirection((Direction)REVERSE);
  for (int cnt = 0; cnt < 2048; cnt++ )
  {
    mystepper.step(-1);
    delay(10);
  }
  delay(2000);
  Serial.println("Starting Test2a: 2048 steps : HALF");
  set_halfstep();
  //mystepper.SetDirection((Direction)FORWARD);
  for (int cnt = 0; cnt < 2048; cnt++ )
  {
    mystepper.step(1);
    delay(10);
  }
  delay(2000);
  Serial.println("Starting Test2: 2048 steps REVERSE: HALF");
  set_halfstep();
  //mystepper.SetDirection((Direction)REVERSE);
  for (int cnt = 0; cnt < 2048; cnt++ )
  {
    mystepper.step(-1);
    delay(10);
  }
  delay(2000);
  Serial.println("Starting Test3: 1step : FULL : Timed");
  set_fullstep();
  //mystepper.SetDirection((Direction)FORWARD);
  startt = micros();
  mystepper.step(1);
  endt = micros() - startt;
  Serial.print("Time taken for one step: ");
  Serial.print(endt);
  Serial.println(" microseconds");
  Serial.println("Going back 1 step");
  //mystepper.SetDirection((Direction)REVERSE);
  mystepper.step(-1);
  delay(2000);
  Serial.println("Starting Test4a: 2048 steps : FULL : step(2048)");
  set_fullstep();
  //mystepper.SetDirection((Direction)FORWARD);
  mystepper.step(2048);
  delay(2000);
  Serial.println("Starting Test4b: 2048 steps REVERSE: FULL : step(-2048)");
  set_fullstep();
  //mystepper.SetDirection((Direction)REVERSE);
  mystepper.step(-2048);
  delay(5000);
} // end Loop()
