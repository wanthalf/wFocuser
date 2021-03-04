// myFP2ESP - Firmware for ESP8266 and ESP32 myFocuserPro2 Controllers
// BLUETOOTH TEST [ESP32 ONLY]
// Tested on ESP32 DEV1 module, using myTCPCLIENT (Form3) and myFocuserPro2 Windows App
// Pairing does not require a passkey
// When paired, a COM PORT for the device will be created on the Windows Client computer
// Use this COM PORT value in communicating with the ESP32 BT controller
// This program has DEBUG on and messages will be sent to USB serial port of ESP32 BT controller

// (c) Copyright Robert Brown 2014-2019. All Rights Reserved.
// (c) Copyright Holger M, 2019, who wrote large portions of code for state machine and esp comms
//
// CONTRIBUTIONS
// If you wish to make a small contribution in thanks for this project, please use PayPal and send the amount
// to user rbb1brown@gmail.com (Robert Brown). All contributions are gratefully accepted.
//
// 1. Set your CHIPMODEL [section 1] based on selected chipType matching your PCB
// 2. Set your DRVBRD [section 2] in this file so the correct driver board is used
// 3. Set your target CPU to esp32
// 4. Set the correct hardware options [section 4] in this file to match your hardware
// 5. Compile and upload to your controller
//
// ----------------------------------------------------------------------------------------------
// 1: DEFINE CHIP MODEL
// ----------------------------------------------------------------------------------------------
#include "generalDefinitions.h"
#include "chipModels.h"             // include chip definitions and hardware mappings

// GOTO FILE chipModels.h and select the correct chip model that matches your PCB

// DO NOT CHANGE
#ifndef CHIPMODEL                   // error checking, please do NOT change
#halt // ERROR you must have CHIPMODEL defined in chipModels.h
#endif

// ----------------------------------------------------------------------------------------------
// 2: SPECIFY DRIVER BOARD HERE
// ----------------------------------------------------------------------------------------------
// DRIVER BOARDS - Please specify your driver board here, only 1 can be defined, see DRVBRD line
#include "myBoardTypes.h"

//Set DRVBRD to the correct driver board above, ONLY ONE!!!! MUST BE AN ESP32 CHIP
//#define DRVBRD PRO2ESP32DRV8825
#define DRVBRD PRO2ESP32ULN2003
//#define DRVBRD PRO2ESP32L298N
//#define DRVBRD PRO2ESP32L293DMINI
//#define DRVBRD PRO2ESP32L9110S

#include "myBoards.h"

// DO NOT CHANGE
#ifndef DRVBRD    // error checking, please do NOT change
#halt // ERROR you must have DRVBRD defined
#endif

// ----------------------------------------------------------------------------------------------
// 2: SPECIFY STEPPER MOTOR HERE
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 3: SPECIFY ESP32/ESP8266 CHIP TYPE
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 4: SPECIFY HARDWARE OPTIONS HERE
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 5: SPECIFY THE TYPE OF OLED DISPLAY HERE
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 6: SPECIFY THE CONTROLLER MODE HERE - ONLY ONE OF THESE MUST BE DEFINED
// ----------------------------------------------------------------------------------------------

// [ESP32 only]
#define BLUETOOTHMODE 1

// ----------------------------------------------------------------------------------------------
// 7. INCLUDES FOR WIFI
// ----------------------------------------------------------------------------------------------

#include <ArduinoJson.h>
#include <SPI.h>
#include "FocuserSetupData.h"
#include "SPIFFS.h"

// ----------------------------------------------------------------------------------------------
// 8. WIFI NETWORK SSID AND PASSWORD CONFIGURATION
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 9. DUCKDNS DOMAIN AND TOKEN CONFIGURATION
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 10. STATIC IP ADDRESS CONFIGURATION
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
// 11. FIRMWARE CODE START - INCLUDES AND LIBRARIES
// ----------------------------------------------------------------------------------------------
// Compile this with Arduino IDE 1.8.9 with ESP8266 Core library installed v2.5.2 [for ESP8266]
// Make sure target board is set to Node MCU 1.0 (ESP12-E Module) [for ESP8266]

// Project specific includes
#ifdef BLUETOOTHMODE
#include "ESPQueue.h"                       //  By Steven de Salas
#endif

// ----------------------------------------------------------------------------------------------
// 12. BLUETOOTH MODE - Do not change
// ----------------------------------------------------------------------------------------------
#ifdef BLUETOOTHMODE
#include "BluetoothSerial.h"                // needed for Bluetooth comms
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
#endif

#ifdef BLUETOOTHMODE
String btline;                              // buffer for serial data
String BLUETOOTHNAME = "MYFP3ESP32BT";      // default name for Bluetooth controller, this name you can change
BluetoothSerial SerialBT;                   // define BT adapter to use
#endif // BLUETOOTHMODE

// ----------------------------------------------------------------------------------------------
// 13. GLOBAL DATA -- DO NOT CHANGE
// ----------------------------------------------------------------------------------------------

//  StateMachine definition
#define State_Idle            0
#define State_InitMove        1
#define State_ApplyBacklash   2
#define State_Moving          3
#define State_DelayAfterMove  4
#define State_FinishedMove    5
#define move_in               0
#define move_out              1
//           reversedirection
//__________________________________
//               0   |   1
//__________________________________
//move_out  1||  1   |   0
//move_in   0||  0   |   1

#ifdef DRVBRD
#if( DRVBRD == PRO2ESP32DRV8825)
char programName[]  = "myFP2ESP32.DRV8825";
#endif
#if( DRVBRD == PRO2ESP32ULN2003)
char programName[]  = "myFP2ESP32.ULN2003";
#endif
#if( DRVBRD == PRO2ESP32L298N)
char programName[]  = "myFP2ESP32.L298N";
#endif
#if( DRVBRD == PRO2ESP32L293DMINI)
char programName[]  = "myFP2ESP32.L293DMINI";
#endif
#if( DRVBRD == PRO2ESP32L9110S)
char programName[]  = "myFP2ESP32.L9110S";
#endif

DriverBoard* driverboard;

#endif

char programVersion[] = "228";
char ProgramAuthor[]  = "(c) R BROWN 2018";
char ontxt[]          = "ON ";
char offtxt[]         = "OFF";
char coilpwrtxt[]     = "Coil power  =";
char revdirtxt[]      = "Reverse Dir =";

unsigned long fcurrentPosition;          // current focuser position
unsigned long ftargetPosition;           // target position
unsigned long tmppos;

byte tprobe1;                           // indicate if there is a probe attached to myFocuserPro2
byte isMoving;                          // is the motor currently moving
byte motorspeedchangethresholdsteps;    // step number where when pos close to target motor speed changes
byte motorspeedchangethresholdenabled;  // used to enable/disable motorspeedchange when close to target position

#ifdef BLUETOOTHMODE
Queue queue(QUEUELENGTH);               // receive serial queue of commands
String line;                            // buffer for serial data
#endif

String ipStr;

int packetsreceived;
int packetssent;

SetupData *mySetupData;

// ----------------------------------------------------------------------------------------------
// 14. CODE START - CHANGE AT YOUR OWN PERIL
// ----------------------------------------------------------------------------------------------

void software_Reboot()
{
  delay(1000);
  ESP.restart();
}

// STEPPER MOTOR ROUTINES
void setstepperspeed( byte spd )
{
  driverboard->setmotorspeed(spd);
}

void setsteppermode(byte smode)
{
  driverboard->setstepmode(smode);
}

void enablesteppermotor(void)
{
  driverboard->enablemotor();
}

void releasesteppermotor(void)
{
  driverboard->releasemotor();
}

void steppermotormove(byte dir )           // direction move_in, move_out ^ reverse direction
{
#ifdef INOUTLEDS
  ( dir == move_in ) ? digitalWrite(INLED, 1) : digitalWrite(OUTLED, 1);
#endif
  driverboard->movemotor(dir);
#ifdef INOUTLEDS
  ( dir == move_in ) ? digitalWrite(INLED, 0) : digitalWrite(OUTLED, 0);
#endif
}

void SendPaket(String str)
{
  DebugPrint(F("Send: "));
  DebugPrintln(str);
#ifndef BLUETOOTHMODE
  // for Accesspoint or Station mode
  myclient.print(str);
  packetssent++;
#else
  // for bluetooth
  SerialBT.print(str);
#endif
}

void ESP_Communication( byte mode )
{
  byte cmdval;
  String receiveString = "";
  String WorkString = "";
  long paramval = 0;
  String replystr = "";

  switch ( mode )
  {
    case BTDATA:
      // for bluetooth
      receiveString = ":" + queue.pop();
      break;
  }

  receiveString += '#';                          // put back terminator
  String cmdstr = receiveString.substring(1, 3);
  cmdval = cmdstr.toInt();                       // convert command to an integer
  DebugPrint(F("- receive string="));
  DebugPrintln(receiveString);
  DebugPrint(F("- cmdstr="));
  DebugPrintln(cmdstr);
  DebugPrint(F("- cmdval="));
  DebugPrintln(cmdval);
  switch (cmdval)
  {
    // all the get go first followed by set
    case 0: // get focuser position
      SendPaket('P' + String(fcurrentPosition) + '#');
      break;
    case 1: // ismoving
      SendPaket('I' + String(isMoving) + '#');
      break;
    case 2: // get controller status
      SendPaket("EOK#");
      break;
    case 3: // get firmware version
      SendPaket('F' + String(programVersion) + '#');
      break;
    case 4: // get firmware name
      SendPaket('F' + String(programName) + '\r' + '\n' + String(programVersion) + '#');
      break;
    case 6: // get temperature
      SendPaket("Z20.00#");
      break;
    case 8: // get maxStep
      SendPaket('M' + String(mySetupData->get_maxstep()) + '#');
      break;
    case 10: // get maxIncrement
      SendPaket('Y' + String(mySetupData->get_maxstep()) + '#');
      break;
    case 11: // get coilpower
      SendPaket('O' + String(mySetupData->get_coilpower()) + '#');
      break;
    case 13: // get reverse direction setting, 00 off, 01 on
      SendPaket('R' + String(mySetupData->get_reversedirection()) + '#');
      break;
    case 21: // get temp probe resolution
      SendPaket('Q' + String(mySetupData->get_tempprecision()) + '#');
      break;
    case 24: // get status of temperature compensation (enabled | disabled)
      SendPaket('1' + String(mySetupData->get_tempcompenabled()) + '#');
      break;
    case 25: // get IF temperature compensation is available
      SendPaket("A0#");
      break;
    case 26: // get temperature coefficient steps/degree
      SendPaket('B' + String(mySetupData->get_tempcoefficient()) + '#');
      break;
    case 29: // get stepmode
      SendPaket('S' + String(mySetupData->get_stepmode()) + '#');
      break;
    case 32: // get if stepsize is enabled
      SendPaket('U' + String(mySetupData->get_stepsizeenabled()) + '#');
      break;
    case 33: // get stepsize
      SendPaket('T' + String(mySetupData->get_stepsize()) + '#');
      break;
    case 34: // get the time that an LCD screen is displayed for
      SendPaket('X' + String(mySetupData->get_lcdpagetime()) + '#');
      break;
    case 37: // get displaystatus
      SendPaket('D' + String(mySetupData->get_displayenabled()) + '#');
      break;
    case 38: // :38#   Dxx#      Get Temperature mode 1=Celsius, 0=Fahrenheight
      SendPaket('b' + String(mySetupData->get_tempmode()) + '#');
      break;
    case 39: // get the new motor position (target) XXXXXX
      SendPaket('N' + String(ftargetPosition) + '#');
      break;
    case 43: // get motorspeed
      SendPaket('C' + String(mySetupData->get_motorSpeed()) + '#');
      break;
    case 45: // get motorspeedchange threshold value
      SendPaket('G' + String(motorspeedchangethresholdsteps) + '#');
      break;
    case 47: // get motorspeedchange enabled? on/off
      SendPaket('J' + String(motorspeedchangethresholdenabled) + '#');
      break;
    case 49: // aXXXXX
      SendPaket("ab552efd25e454b36b35795029f3a9ba7#");
      break;
    case 51: // return ESP8266Wifi Controller IP Address
      SendPaket('d' + ipStr + '#');
      break;
    case 52: // return ESP32 Controller number of TCP packets sent
      SendPaket('e' + String(packetssent) + '#');
      break;
    case 53: // return ESP32 Controller number of TCP packets received
      SendPaket('f' + String(packetsreceived) + '#');
      break;
    case 54: // gstr#  return ESP32 Controller SSID
      SendPaket("g0000000#");
      break;
    case 62: // get update of position on lcd when moving (00=disable, 01=enable)
      SendPaket("L0#");
      break;
    case 63: // get status of home position switch (0=off, 1=closed, position 0)
      SendPaket("H0#");
      break;
    case 66: // Get jogging state enabled/disabled
      SendPaket("K0#");
      break;
    case 68: // Get jogging direction, 0=IN, 1=OUT
      SendPaket("V0#");
      break;
    case 72: // get DelayAfterMove
      SendPaket('3' + String(mySetupData->get_DelayAfterMove()) + '#');
      break;
    case 74: // get backlash in enabled status
      SendPaket((mySetupData->get_backlash_in_enabled() == 0) ? "40#" : "41#");
      break;
    case 76: // get backlash OUT enabled status
      SendPaket((mySetupData->get_backlash_out_enabled() == 0) ? "50#" : "51#");
      break;
    case 78: // return number of backlash steps IN
      SendPaket('6' + String(mySetupData->get_backlashsteps_in()) + '#');
      break;
    case 80: // return number of backlash steps OUT
      SendPaket('7' + String(mySetupData->get_backlashsteps_out()) + '#');
      break;
    case 83: // get if there is a temperature probe
      SendPaket('c' + String(tprobe1) + '#');
      break;
    case 87: // get tc direction
      SendPaket('k' + String(mySetupData->get_tcdirection()) + '#');
      break;
    case 89:
      SendPaket("91#");
      break;
    // only the set commands are listed here as they do not require a response
    case 28:              // :28#       None    home the motor to position 0
      ftargetPosition = 0; // if this is a home then set target to 0
      break;
    case 5: // :05xxxxxx# None    Set new target position to xxxxxx (and focuser initiates immediate move to xxxxxx)
      // only if not already moving
      if ( isMoving == 0 )
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        ftargetPosition = (unsigned long)WorkString.toInt();
        if (ftargetPosition > mySetupData->get_maxstep())
          ftargetPosition = mySetupData->get_maxstep();
      }
      break;
    case 7: // set maxsteps
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      tmppos = (unsigned long)WorkString.toInt();
      delay(5);
      // check to make sure not above largest value for maxstep
      tmppos = (tmppos > FOCUSERUPPERLIMIT) ? FOCUSERUPPERLIMIT : tmppos;
      // check if below lowest set value for maxstep
      tmppos = (tmppos < FOCUSERLOWERLIMIT) ? FOCUSERLOWERLIMIT : tmppos;
      // check to make sure its not less than current focuser position
      tmppos = (tmppos < fcurrentPosition) ? fcurrentPosition : tmppos;
      mySetupData->set_maxstep(tmppos);
      break;
    case 12: // set coil power
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = (byte)WorkString.toInt() & 1;
      ( paramval == 1 ) ? enablesteppermotor() : releasesteppermotor();
      mySetupData->set_coilpower(paramval);
      break;
    case 14: // set reverse direction
      if ( isMoving == 0 )
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        mySetupData->set_reversedirection((byte)WorkString.toInt() & 1);
      }
      break;
    case 15: // set motorspeed
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = (byte)WorkString.toInt() & 3;
      mySetupData->set_motorSpeed((byte) paramval);
      setstepperspeed((byte) paramval);
      break;
    case 16: // set display to celsius
      mySetupData->set_tempmode(1); // temperature display mode, Celcius=1, Fahrenheit=0
      break;
    case 17: // set display to fahrenheit
      mySetupData->set_tempmode(0); // temperature display mode, Celcius=1, Fahrenheit=0
      break;
    case 18:
      // :180#    None    set the return of user specified stepsize to be OFF - default
      // :181#    None    set the return of user specified stepsize to be ON - reports what user specified as stepsize
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      mySetupData->set_stepsizeenabled((byte)WorkString.toInt() & 1);
      break;
    case 19: // :19xxxx#  None   set the step size value - double type, eg 2.1
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        float tempstepsize = (float)WorkString.toFloat();
        if (tempstepsize < 0)
          tempstepsize = 0; // set default maximum stepsize
        mySetupData->set_stepsize(tempstepsize);
      }
      break;
    case 20: // set the temperature resolution setting for the DS18B20 temperature probe
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = WorkString.toInt();
      mySetupData->set_tempprecision((byte) paramval);
      break;
    case 22: // set the temperature compensation value to xxx
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = WorkString.toInt();
      mySetupData->set_tempcoefficient((byte)paramval);
      break;
    case 23: // set the temperature compensation ON (1) or OFF (0)
      break;
    case 27: // stop a move - like a Halt
      ftargetPosition = fcurrentPosition;
      break;
    case 30: // set step mode
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = WorkString.toInt();
#if (DRVBRD == PRO2ESP32ULN2003 || DRVBRD == PRO2ESP32L298N || DRVBRD == PRO2ESP32L293DMINI || DRVBRD == PRO2ESP32L9110S)
      paramval = (byte)(paramval & 3);      // STEP1 - STEP2
#endif
#if (DRVBRD == PRO2ESP32DRV8825)
      paramval = (paramval < STEP1 ) ? STEP1 : paramval;
      paramval = (paramval > STEP32 ) ? STEP32 : paramval;
#endif
      mySetupData->set_stepmode((byte)paramval);
      setsteppermode((byte) paramval);
      break;
    case 31: // set focuser position
      if ( isMoving == 0 )
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        {
          long tpos = (long)WorkString.toInt();
          tpos = (tpos < 0) ? 0 : tpos;
          tmppos = ((unsigned long) tpos > mySetupData->get_maxstep()) ? mySetupData->get_maxstep() : (unsigned long) tpos;
          fcurrentPosition = ftargetPosition = tmppos;
          mySetupData->set_fposition(tmppos);
        }
      }
      break;
    case 35: // set length of time an LCD page is displayed for in seconds
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = WorkString.toInt();
      paramval = (paramval < LCDPAGETIMEMIN ) ? LCDPAGETIMEMIN : paramval;
      paramval = (paramval > LCDPAGETIMEMAX ) ? LCDPAGETIMEMAX : paramval;
      mySetupData->set_lcdpagetime((byte)paramval);
      break;
    case 36:
      // :360#    None    Disable Display
      // :361#    None    Enable Display
      break;
    case 40: // reset Arduino myFocuserPro2E controller
      software_Reboot();
      //server.restart();
      break;
    case 44: // set motorspeed threshold when moving - switches to slowspeed when nearing destination
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      motorspeedchangethresholdsteps  = (byte) (WorkString.toInt() & 255);
      break;
    case 46: // enable/Disable motorspeed change when moving
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      delay(5);
      motorspeedchangethresholdenabled = (byte) (WorkString.toInt() & 1);
      break;
    case 48: // save settings to EEPROM
      mySetupData->set_fposition(fcurrentPosition); // need to forth save setting????????
      break;
    case 61: // set update of position on lcd when moving (00=disable, 01=enable)
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      mySetupData->set_lcdupdateonmove((byte)WorkString.toInt() & 1);
      break;
    case 64: // move a specified number of steps
      if ( isMoving == 0 )
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        tmppos = (unsigned long)WorkString.toInt() + fcurrentPosition;
        ftargetPosition = ( tmppos > mySetupData->get_maxstep()) ? mySetupData->get_maxstep() : tmppos;
      }
      break;
    case 65: // set jogging state enable/disable
      // ignore
      break;
    case 67: // set jogging direction, 0=IN, 1=OUT
      // ignore
      break;
    case 42: // reset focuser defaults
      if ( isMoving == 0 )
      {
        mySetupData->SetFocuserDefaults();
        ftargetPosition = fcurrentPosition = mySetupData->get_fposition();
      }
      break;
    case 71: // set DelayAfterMove in milliseconds
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      mySetupData->set_DelayAfterMove((byte)WorkString.toInt());
      break;
    case 73: // Disable/enable backlash IN (going to lower focuser position)
#ifdef BACKLASH
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = WorkString.toInt();
      mySetupData->set_backlash_in_enabled((byte)(paramval & 1));
#endif
      break;
    case 75: // Disable/enable backlash OUT (going to lower focuser position)
#ifdef BACKLASH
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = (byte)WorkString.toInt();
      mySetupData->set_backlash_out_enabled((byte)(paramval & 1));
#endif
      break;
    case 77: // set backlash in steps
#ifdef BACKLASH
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      mySetupData->set_backlashsteps_in((byte)WorkString.toInt());
#endif
      break;
    case 79: // set backlash OUT steps
#ifdef BACKLASH
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      mySetupData->set_backlashsteps_out((byte)WorkString.toInt());
#endif
      break;
    case 88: // set tc direction
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = WorkString.toInt();
      mySetupData->set_tcdirection((byte)paramval & 1);
      break;
  }
}

void setup()
{
#if defined DEBUG || defined LOCALSERIAL      // Open serial port if debugging or open serial port if LOCALSERIAL
  Serial.begin(SERIALPORTSPEED);
#endif

  mySetupData = new SetupData(Mode_SPIFFS);   //instantiate object SetUpData with SPIFFS file instead of using EEPROM, init SPIFFS

#ifdef BLUETOOTHMODE                          // open Bluetooth port and set bluetooth device name if defined
  SerialBT.begin(BLUETOOTHNAME);              // Bluetooth device name
  btline = "";
  clearbtPort();
  DebugPrintln(F("Bluetooth started."));
#endif

  delay(250);                                 // keep delays small otherwise issue with ASCOM

  DebugPrint(F(" fposition : "));             // Print Loaded Values from SPIFF
  DebugPrintln(mySetupData->get_fposition());
  DebugPrint(F(" focuserdirection : "));
  DebugPrintln(mySetupData->get_focuserdirection());
  DebugPrint(F(" maxstep : "));
  DebugPrintln(mySetupData->get_maxstep());
  DebugPrint(F(" stepsize : "));;
  DebugPrintln(mySetupData->get_stepsize());;
  DebugPrint(F(" DelayAfterMove : "));
  DebugPrintln(mySetupData->get_DelayAfterMove());
  DebugPrint(F(" backlashsteps_in : "));
  DebugPrintln(mySetupData->get_backlashsteps_in());
  DebugPrint(F(" backlashsteps_out : "));
  DebugPrintln(mySetupData->get_backlashsteps_out());
  DebugPrint(F(" tempcoefficient : "));
  DebugPrintln(mySetupData->get_tempcoefficient());
  DebugPrint(F(" tempprecision : "));
  DebugPrintln(mySetupData->get_tempprecision());
  DebugPrint(F(" stepomode : "));
  DebugPrintln(mySetupData->get_stepmode());
  DebugPrint(F(" coilpower : "));
  DebugPrintln(mySetupData->get_coilpower());
  DebugPrint(F(" reversedirection : "));
  DebugPrintln(mySetupData->get_reversedirection());
  DebugPrint(F(" stepsizeenabled : "));
  DebugPrintln(mySetupData->get_stepsizeenabled());
  DebugPrint(F(" tempmode : "));
  DebugPrintln(mySetupData->get_tempmode());
  DebugPrint(F(" lcdupdateonmove : "));
  DebugPrintln(mySetupData->get_lcdupdateonmove());
  DebugPrint(F(" lcdpagedisplaytime : "));
  DebugPrintln(mySetupData->get_lcdpagetime());
  DebugPrint(F(" tempcompenabled : "));
  DebugPrintln(mySetupData->get_tempcompenabled());
  DebugPrint(F(" tcdirection : "));
  DebugPrintln(mySetupData->get_tcdirection());
  DebugPrint(F(" motorSpeed : "));
  DebugPrintln(mySetupData->get_motorSpeed());
  DebugPrint(F(" displayenabled : "));
  DebugPrintln(mySetupData->get_displayenabled());

  // it is Bluetooth so set some globals
  ipStr = "0.0.0.0";

  // assign to current working values
  ftargetPosition = fcurrentPosition = mySetupData->get_fposition();

#if( DRVBRD == PRO2ESP32DRV8825)
  driverboard = new DriverBoard(PRO2ESP32DRV8825, String(programName), mySetupData->get_stepmode(), mySetupData->get_motorSpeed());
#endif
#if( DRVBRD == PRO2ESP32ULN2003)
  driverboard = new DriverBoard(PRO2ESP32ULN2003, String(programName), mySetupData->get_stepmode(), mySetupData->get_motorSpeed(), IN1ULN, IN3ULN, IN4ULN, IN2ULN);
#endif
#if( DRVBRD == PRO2ESP32L298N)
  driverboard = new DriverBoard(PRO2ESP32L298N, String(programName), mySetupData->get_stepmode(), mySetupData->get_motorSpeed(), IN1L298N, IN2L298N, IN3L298N, IN4L298N);
#endif
#if( DRVBRD == PRO2ESP32L293DMINI)
  driverboard = new DriverBoard(PRO2ESP32L293DMINI, String(programName), mySetupData->get_stepmode(), mySetupData->get_motorSpeed(), IN1L293DMINI, IN2L293DMINI, IN3L293DMINI, IN4L293DMINI);
#endif
#if( DRVBRD == PRO2ESP32L9110S)
  driverboard = new DriverBoard(PRO2ESP32L9110S, String(programName), mySetupData->get_stepmode(), mySetupData->get_motorSpeed(), IN1L9110S, IN2L9110S, IN3L9110S, IN4L9110S);
#endif

  delay(5);


  // range check focuser variables
  mySetupData->set_coilpower((mySetupData->get_coilpower() >= 1) ?  1 : 0);
  mySetupData->set_reversedirection((mySetupData->get_reversedirection() >= 1) ?  1 : 0);
  mySetupData->set_lcdpagetime((mySetupData->get_lcdpagetime() < 2) ? mySetupData->get_lcdpagetime() : 2);
  mySetupData->set_lcdpagetime((mySetupData->get_lcdpagetime() > 10) ? 10 : mySetupData->get_lcdpagetime());
  mySetupData->set_displayenabled((mySetupData->get_displayenabled() & 1));
  mySetupData->set_maxstep((mySetupData->get_maxstep() < FOCUSERLOWERLIMIT) ? FOCUSERLOWERLIMIT : mySetupData->get_maxstep());
  //mySetupData->set_fposition((mySetupData->get_fposition() < 0 ) ? 0 : mySetupData->get_fposition());
  //mySetupData->set_fposition((mySetupData->get_fposition() > mySetupData->get_maxstep()) ? mySetupData->get_maxstep() : mySetupData->get_fposition());
  mySetupData->set_stepsize((float)(mySetupData->get_stepsize() < 0.0 ) ? 0 : mySetupData->get_stepsize());
  mySetupData->set_stepsize((float)(mySetupData->get_stepsize() > DEFAULTSTEPSIZE ) ? DEFAULTSTEPSIZE : mySetupData->get_stepsize());

  if (mySetupData->get_coilpower() == 0)
  {
    driverboard->releasemotor();
  }

  delay(5);

  motorspeedchangethresholdsteps = MOTORSPEEDCHANGETHRESHOLD;
  motorspeedchangethresholdenabled = 0;
  isMoving = 0;
}

//_____________________ loop()___________________________________________

void loop()
{
  static byte MainStateMachine = State_Idle;
  static byte DirOfTravel = mySetupData->get_focuserdirection();
  static byte backlash_count = 0;
  static byte backlash_enabled = 0;
  static byte updatecount = 0;

#ifdef BLUETOOTHMODE
  if ( SerialBT.available() )
  {
    processbt();
  }
  // if there is a command from Bluetooth
  if ( queue.count() >= 1 )                 // check for serial command
  {
    ESP_Communication(BTDATA);
  }
#endif // end Bluetoothmode

  switch (MainStateMachine)
  {
    case State_Idle:
      if (fcurrentPosition != ftargetPosition)
      {
        enablesteppermotor();
        MainStateMachine = State_InitMove;
        DebugPrint(F("Idle => InitMove Target "));
        DebugPrint(ftargetPosition);
        DebugPrint(F(" Current "));
        DebugPrintln(fcurrentPosition);
      }
      else
      {
#ifdef OLEDDISPLAY
#ifdef OLEDGRAPHICS
        if ( mySetupData->get_displayenabled() == 1)
        {
          oled_draw_main(oled_stay);
        }
#endif
#ifdef OLEDTEXT
        if ( mySetupData->get_displayenabled() == 1)
        {
          Update_OledText();
        }
#endif
#endif
#ifdef TEMPERATUREPROBE
        Update_Temp();
#endif
        byte status = mySetupData->SaveConfiguration(fcurrentPosition, DirOfTravel); // save config if needed
        if ( status == true )
        {
#ifdef OLEDDISPLAY
#ifdef OLEDGRAPHICS
          oled_draw_main(oled_off);           // Display off after config saved
#endif
#endif
          DebugPrint("new Config saved: ");
          DebugPrintln(status);
        }
        break;
      }

    case State_InitMove:
      isMoving = 1;
      DirOfTravel = (ftargetPosition > fcurrentPosition) ? move_out : move_in;
      //driverboard.setDirOfTravel(DirOfTravel ^ mySetupData->get_reversedirection());    // Dir and Enable motor driver
      enablesteppermotor();
      if (mySetupData->get_focuserdirection() == DirOfTravel)
      {
        // move is in same direction, ignore backlash
        MainStateMachine = State_Moving;
        DebugPrintln(F("=> State_Moving"));
      }
      else
      {
        // move is in opposite direction, check for backlash enabled
        // get backlash settings
        if ( DirOfTravel == move_in)
        {
          backlash_count = mySetupData->get_backlashsteps_in();
          backlash_enabled = mySetupData->get_backlash_in_enabled();
        }
        else
        {
          backlash_count = mySetupData->get_backlashsteps_out();
          backlash_enabled = mySetupData->get_backlash_out_enabled();
        }
        // backlash needs to be applied, so get backlash values and states
        if ( backlash_enabled == 1 )
        {
          // apply backlash
          // save new direction of travel
          mySetupData->set_focuserdirection(DirOfTravel);
          setstepperspeed(BACKLASHSPEED);
          MainStateMachine = State_ApplyBacklash;
          DebugPrint(F("Idle => State_ApplyBacklash"));
        }
        else
        {
          // do not apply backlash, go straight to moving
          MainStateMachine = State_Moving;
          DebugPrint(F("Idle => State_Moving"));
        }
      }
      break;

    case State_ApplyBacklash:
      if ( backlash_count )
      {
        steppermotormove(DirOfTravel);
        backlash_count--;
      }
      else
      {
        setstepperspeed(mySetupData->get_motorSpeed());
        MainStateMachine = State_Moving;
        DebugPrintln(F("=> State_Moving"));
      }
      break;

    case State_Moving:
      if ( fcurrentPosition != ftargetPosition )      // must come first else cannot halt
      {
        if (DirOfTravel == move_out )
          fcurrentPosition++;
        else
          fcurrentPosition--;
        steppermotormove(DirOfTravel);
#ifdef OLEDDISPLAY
        if ( mySetupData->get_displayenabled() == 1)
        {
          if ( mySetupData->get_lcdupdateonmove() == 1 )
          {
            updatecount++;
            if ( updatecount > LCDUPDATEONMOVE )
            {
              delay(5);
              updatecount = 0;
#ifdef OLEDGRAPHICS
              // TODO Holger - updates position and target values on OLED when moving
              // UpdatePositionOledGraphics();  // ????
#endif
#ifdef OLEDTEXT
              UpdatePositionOledText();
#endif
            }
          }
        }
#endif
      }
      else
      {
        MainStateMachine = State_DelayAfterMove;
        DebugPrintln(F("=> State_DelayAfterMove"));
      }
      break;

    case State_DelayAfterMove:
      // apply Delayaftermove, this MUST be done here in order to get accurate timing for DelayAfterMove
      delay(mySetupData->get_DelayAfterMove());
      MainStateMachine = State_FinishedMove;
      DebugPrintln(F("=> State_FinishedMove"));
      break;

    case State_FinishedMove:
      isMoving = 0;
      if ( mySetupData->get_coilpower() == 0 )
        releasesteppermotor();
      MainStateMachine = State_Idle;
      DebugPrintln(F("=> State_Idle"));
      break;

    default:
      MainStateMachine = State_Idle;
      break;
  }
} // end Loop()

#ifdef BLUETOOTHMODE
void clearbtPort()
{
  while (SerialBT.available())
  {
    SerialBT.read();
  }
}

void processbt()
{
  // SerialBT.read() only returns a single char so build a command line one char at a time
  // : starts the command, # ends the command, do not store these in the command buffer
  // read the command until the terminating # character
  while (SerialBT.available() )
  {
    char inChar = SerialBT.read();
    switch ( inChar )
    {
      case ':' :     // start
        btline = "";
        break;
      case '\r' :
      case '\n' :
        // ignore
        break;
      case '#' :     // eoc
        queue.push(btline);
        break;
      default :      // anything else
        btline = btline + inChar;
        break;
    }
  }
}
#endif
