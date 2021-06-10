// ======================================================================
// moonlitecomms.h : myFP2ESP COMMS ROUTINES FOR SUPPORTING MOONLITE
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2021. All Rights Reserved.
// ======================================================================

#ifndef moonlitecomms_h
#define moonlitecomms_h

// ======================================================================
// INCLUDES
// ======================================================================

#include <Arduino.h>
#include "generalDefinitions.h"
#include "focuserconfig.h"                      // boarddefs.h included as part of focuserconfig.h"

#if (PROTOCOL == MOONLITE_PROTOCOL)

#undef  FOCUSERUPPERLIMIT
#define FOCUSERUPPERLIMIT     32000L    // arbitary focuser limit up to 32000 for moonlite

// ======================================================================
// EXTERNALS
// ======================================================================

extern volatile bool halt_alert;
#if defined(ESP8266)
// in esp8266, volatile data_type varname is all that is needed
#else
// in esp32, we should use a Mutex for access
extern portMUX_TYPE  halt_alertMux;
#endif

extern byte          isMoving;
extern int           tprobe1;
extern float         lasttemp;
extern const char*   programVersion;
extern unsigned long ftargetPosition;          // target position
extern SetupData     *mySetupData;
extern DriverBoard   *driverboard;
extern TempProbe     *myTempProbe;

extern void software_Reboot(int);

// ======================================================================
// LOCAL DATA [for moonlite code]
// ======================================================================

// ======================================================================
// NOTES
// ======================================================================
// Moonlite uses a command sequence based on :AD# where
// : is the beginning of the command
// AD are two alphabetic chars (A-Z) representing the command
// # is the end of command teminator
// + and - and C are exceptions and are only one character commands
// numeric values in receive commands and responses are in base 16
//
// in MOONLITE, one specifies MaxStep on the SetupDialog form of the ASCOM driver
// and the driver uses that value as a check for every move. MaxStep is not
// sent to the driver.
// there is no get/set maxstep or get/set maxincrement command
//
// This moonlite protocol file will ignore all myFP2ESP commands
// It will not work with the myFP2ESP Windows Application
// or myFP2ESP ASCOM driver
// It will work with myFocuserPro application and ASCOM driver
// In INDI/KSTARS you select the Moonlite driver
// IN Windows you select the Moonlite ASCOM driver

// ======================================================================
// CODE
// ======================================================================

// convert hex string to long int
long hexstr2long(char *line)
{
  long ret = 0;
  ret = strtol(line, NULL, 16);
  return (ret);
}

// convert string to int
int decstr2int(char *line)
{
  int ret = 0;
  String Str(line);
  ret = Str.toInt();
  return ret;
}

void SendMessage(const char *str)
{
  Comms_DebugPrint("Send: ");
  Comms_DebugPrintln(str);
#if ( (CONTROLLERMODE == ACCESSPOINT) || (CONTROLLERMODE == STATIONMODE) )  // for Accesspoint or Station mode
  myclient.print(str);
  packetssent++;
#elif (CONTROLLERMODE == BLUETOOTHMODE)             // for bluetooth
  SerialBT.print(str);
#elif (CONTROLLERMODE == LOCALSERIAL)
  Serial.print(str);
#endif
}

void SendPacket(const char *str)
{
  char mbuffer[32];
  snprintf(mbuffer, sizeof(mbuffer), "%s%c", str, EOFSTR);
  SendMessage(mbuffer);
}

void ESP_Communication()
{
  int    cmdval;                                    // the CMD not as "AD" but as integer value
  long   paramval = 0;                              // holds the parameter associated with the command

  String receiveString = "";
  String WorkString = "";
  char   tempCharArray[32];
  static unsigned long newtargetPosition;           // used in moonlite to set a target position without moving
  static boolean newtargetpositionset = false;      // used in moonlite to to indicate if a new target has been set
  static float   mtempoffsetval = 0.0;              // used in moonlite to set a temperature correction value

#if (CONTROLLERMODE == BLUETOOTHMODE)
  receiveString = STARTCMDSTR + queue.pop();
#elif (CONTROLLERMODE == LOCALSERIAL)
  receiveString = STARTCMDSTR + queue.pop();
#endif

  Comms_DebugPrint("raw receive string=");
  Comms_DebugPrintln(receiveString);

  receiveString = receiveString.substring(1, receiveString.length()); // remove leading :
  Comms_DebugPrint("receive string=");
  Comms_DebugPrintln(receiveString);

  if ( receiveString.length() < 2 )
  {
    cmdval = (int) receiveString[0];                                  // take care of :C# :+# and :-#
  }
  else                                                                // it is a TWO char command
  {
    cmdval = (int) receiveString[0] + ((int) receiveString[1] * 256); // for moonlite get first two chars and generate a value
  }
  Comms_DebugPrint("cmdval = ");
  Comms_DebugPrintln(cmdval);

  if ( receiveString.length() > 2)                                    // if the command has arguments
  {
    WorkString = receiveString.substring(2, receiveString.length() ); // extract any parameters associated with the command
    WorkString.toCharArray(tempCharArray, sizeof(WorkString));        // convert to char array
    paramval = hexstr2long(tempCharArray);                            // convert from Base 16 to long
  }
  Comms_DebugPrint("Arguments = ");
  Comms_DebugPrintln(WorkString);

  switch (cmdval)
  {
    // GP get the current focuser position
    case 20551:
      sprintf(tempCharArray, "%04X", (unsigned int) driverboard->getposition());
      SendPacket(tempCharArray);
      break;

    // GN get the new motor position (target)
    case 20039:
      // not implemented in INDI driver
      sprintf(tempCharArray, "%04X", (unsigned int) newtargetPosition);
      SendPacket(tempCharArray);
      break;

    // GT get the current temperature
    case 21575:
      {
        float tmp = ((lasttemp + mtempoffsetval) * 2);  // Do not forget to apply temperature offset (mtempoffsetval = 0.0)
        sprintf(tempCharArray, "%04X", (int) tmp);
        SendPacket(tempCharArray);
      }
      break;

    // GD get the Motor 1 step delay, valid options are "02, 04, 08, 10, 20"
    // which correspond to a stepping delay of 250, 125, 63, 32 and 16 steps per second respectively
    case 17479:
      switch ( mySetupData->get_motorspeed() )
      {
        case 0:
          sprintf(tempCharArray, "%02X", 2);
          break;
        case 1:
          sprintf(tempCharArray, "%02X", 4);
          break;
        case 2:
          sprintf(tempCharArray, "%02X", 8);
          break;
        default:
          sprintf(tempCharArray, "%02X", 2);
          break;
      }
      SendPacket(tempCharArray);
      break;

    // GH "FF" if half step is set, otherwise "00"
    case 18503:
      if ( mySetupData->get_brdstepmode() == STEP2 )
      {
        SendPacket("FF");
      }
      else
      {
        SendPacket("00");
      };
      break;

    // GI "01" if the motor is moving, otherwise "00"
    case 18759:
      if ( isMoving == 1 )
      {
        SendPacket("01");
      }
      else
      {
        SendPacket("00");
      }
      break;

    // GB get the current RED Led Backlight value, Unsigned Hexadecimal
    case 16967:
      // not implemented in INDI driver
      SendPacket("00");
      break;

    // GV get current firmware version
    case 22087:
      SendPacket("10");
      break;

    // SP set current position to received position - THIS IS NOT A MOVE
    // in INDI driver, only used to set to 0 SP0000 in reset()
    case 20563:
      {
        paramval = (paramval < 0) ? 0 : paramval;               // we cannot have a negative position
        unsigned long tmppos = ((unsigned long) paramval > mySetupData->get_maxstep()) ? mySetupData->get_maxstep() : (unsigned long) paramval;
        ftargetPosition = tmppos;
        driverboard->setposition(tmppos);
        mySetupData->set_fposition(tmppos);
        mySetupData->SaveNow();
      }
      break;

    // SN set new target position SNXXXX - this is a move command
    // but must be followed by a FG command to start the move
    case 20051:
      paramval = (paramval < 0) ? 0 : paramval;               // we cannot have a negative position
      if ( paramval > mySetupData->get_maxstep())             // check against maxstep
      {
        paramval = mySetupData->get_maxstep();
      }
      newtargetPosition = paramval;
      newtargetpositionset = true;
      break;

    // Basic rule for setting stepmode in this order
    // 1. Set mySetupData->set_brdstepmode(xx);               // this saves config setting
    // 2. Set driverboard->setstepmode(xx);                   // this sets the physical pins
    // SF set Motor 1 to Full Step
    case 18003:
      mySetupData->set_brdstepmode(STEP1);
      driverboard->setstepmode(STEP1);
      break;

    // SH set Motor 1 to Half Step
    case 18515:
      mySetupData->set_brdstepmode(STEP2);
      driverboard->setstepmode(STEP2);
      break;

    // SD set the Motor 1 speed, valid options are "02, 04, 08, 10, 20"
    // correspond to a stepping delay of 250, 125, 63, 32 and 16 steps
    // per second respectively. Moonlite only
    case 17491:
      switch (paramval)
      {
        case 2:
          mySetupData->set_motorspeed(FAST);
          break;
        case 4:
          mySetupData->set_motorspeed(MED);
          break;
        case 8:
          mySetupData->set_motorspeed(SLOW);
          break;
        default:
          mySetupData->set_motorspeed(FAST);
          break;
      }
      break;

    // FG Start a Motor 1 move, moves the motor to the New Position.
    case 18246:
      if ( newtargetpositionset == true)
      {
        ftargetPosition = newtargetPosition;
        newtargetpositionset = false;
      }
      else
      {
        // ignore
        Comms_DebugPrintln("FG invalid - target position not set");
      }
      break;

    // FQ Halt Motor 1 move, position is retained, motor is stopped.
    case 20806:
      varENTER_CRITICAL(&halt_alertMux);
      halt_alert = true;
      varEXIT_CRITICAL(&halt_alertMux);
      Comms_DebugPrintln("FQ: halt_alert = true");
      break;

    // PO SET temperature calibration offset POXX in 0.5 degree increments (hex)
    case 20304:
      {
        // this adds/subtracts an offset from the temperature reading in 1/2 degree C steps
        // FA -3, FB -2.5, FC -2, FD -1.5, FE -1, FF -.5, 00 0, 01 0.5, 02 1.0, 03 1.5, 04 2.0, 05 2.5, 06 3.0
        mtempoffsetval = 0.0;
        if ( WorkString == "FA" )
        {
          mtempoffsetval = -3.0;
        }
        else if ( WorkString == "FB")
        {
          mtempoffsetval = -2.5;
        }
        else if ( WorkString == "FC")
        {
          mtempoffsetval = -2.0;
        }
        else if ( WorkString == "FD")
        {
          mtempoffsetval = -1.5;
        }
        else if ( WorkString == "FE")
        {
          mtempoffsetval = -1.0;
        }
        else if ( WorkString == "FF")
        {
          mtempoffsetval = -0.5;
        }
        else if ( WorkString == "00")
        {
          mtempoffsetval = 0.0;
        }
        else if ( WorkString == "01")
        {
          mtempoffsetval = 0.5;
        }
        else if ( WorkString == "02")
        {
          mtempoffsetval = 1.0;
        }
        else if ( WorkString == "03")
        {
          mtempoffsetval = 1.5;
        }
        else if ( WorkString == "04")
        {
          mtempoffsetval = 2.0;
        }
        else if ( WorkString == "05")
        {
          mtempoffsetval = 2.5;
        }
        else if ( WorkString == "06")
        {
          mtempoffsetval = 3.0;
        }
      }
      break;

    // PS Adjust Temperature Scale, Signed Hexadecimal
    //case 21328:
    // ignore
    //  break;

    //case 21072: // PR Adjust Red Backlight Brightness
    //  Comms_DebugPrintln(":PR# ignored");
    //  break;
    //case 18256: // PG Adjust Green Backlight Brightness
    //  Comms_DebugPrintln(":PG# ignored");
    //  break;

    // PB Adjust Blue Backlight Brightness
    case 16976:
      Comms_DebugPrintln(":PB# ignored");
      break;

    // PC Adjust LCD Contrast
    case 17232:
      Comms_DebugPrintln(":PC# ignored");
      break;

    // PX Adjust the Scale for Motor 1
    case 22608:
      Comms_DebugPrintln(":PX# ignored");
      break;

    // PH Find home for Motor, valid options are "01", "02"
    case 18512:
      // not implemented in INDI driver
      ftargetPosition = 0;
      break;

    // C Initiate a temperature conversion
    case 67:
      Comms_DebugPrintln(":C# ignored");
      break;

    // GC get the temperature coefficient where XX is a two-digit signed (2’s complement)
    case 17223:
      sprintf(tempCharArray, "%02X", mySetupData->get_tempcoefficient());
      SendPacket(tempCharArray);
      break;

    // SC set temperature co-efficient XX
    // this does not work as a myfocuserpro controller does not support temperature compensation
    case 17235:
      // Set the new temperature coefficient where XX is a two-digit, signed (2’s complement) hex number
      // MSB = 1 is negative, MSB = 0 number is positive
      // The two's complement is calculated by inverting the digits and adding one
      {
        short int tcval = (short int)(paramval);
        sscanf(tempCharArray, "%hd", &tcval);         // h is half or short, signed
        mySetupData->set_tempcoefficient((byte) tcval);
      }
      break;

    // + activate temperature compensation focusing
    // a myfocuserpro controller does not support temperature compensation
    case 43:
      Comms_DebugPrintln(":+# ignored");
      //mySetupData->set_tempcompenabled(1);
      break;

    // - disable temperature compensation focusing
    // a myfocuserpro controller does not support temperature compensation
    case 45:
      Comms_DebugPrintln(":-# ignored");
      //mySetupData->set_tempcompenabled(0);
      break;

    // now list the myFocuserPro extensions to Moonlite

    // DG get display state on or off
    case 18244:
      Comms_DebugPrint("Get Display State : ");
      Comms_DebugPrintln((int) mySetupData->get_displayenabled());
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_displayenabled());
      SendPacket(tempCharArray);
      break;

    // DM set displaystate C or F
    case 19780:
      Comms_DebugPrint("Set Display Temp Mode : ");
      Comms_DebugPrintln((byte)(paramval & 0x01));
      mySetupData->set_tempmode((byte)(paramval & 0x01));
      break;

    // DS disable or enable the display setting
    case 21316:
      Comms_DebugPrint("Set Display State : ");
      Comms_DebugPrintln((int)(paramval & 0x01));
      if ( displaystate == true )
      {
        if ( (int)(paramval & 0x01) == 0 )
        {
          mySetupData->set_displayenabled(0);
          if ( displayfound == true )
          {
            myoled->display_off();
          }
        }
        else
        {
          mySetupData->set_displayenabled(1);
          if ( displayfound == true )
          {
            myoled->display_on();
          }
        }
      }
      break;

    // FM get Display temp mode (Celsius=1, Fahrenheit=0)
    case 19782:
      Comms_DebugPrint("Get Display Temp Mode : ");
      Comms_DebugPrintln((int) mySetupData->get_tempmode());
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_tempmode());
      SendPacket(tempCharArray);
      break;

    // GF get firmware value
    case 17991:
      {
        String reply = "myFP2ESP-M\r\n" + String(programVersion);
        reply.toCharArray(tempCharArray, reply.length() + 1);
        SendPacket(tempCharArray);
      }
      break;

    // GM get the MaxSteps
    // GY get the maxIncrement - set to MaxSteps
    case 19783:
    case 22855:
      Comms_DebugPrint("Get Max Step : ");
      Comms_DebugPrintln((unsigned int) mySetupData->get_maxstep());
      sprintf(tempCharArray, "%04X", (unsigned int) mySetupData->get_maxstep());
      SendPacket(tempCharArray);
      break;

    // GO get the coilPwr setting
    case 20295:
      Comms_DebugPrint("Get Coil Power : ");
      Comms_DebugPrintln( (int) mySetupData->get_coilpower());
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_coilpower());
      SendPacket(tempCharArray);
      break;

    // GR get the Reverse Direction setting
    case 21063:
      Comms_DebugPrint("Set Reverse Direction : ");
      Comms_DebugPrintln((int) mySetupData->get_reversedirection());
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_reversedirection());
      SendPacket(tempCharArray);
      break;

    // GS get stepmode
    case 21319:
      Comms_DebugPrint("get step mode : ");
      Comms_DebugPrintln((int) mySetupData->get_brdstepmode());
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_brdstepmode());
      SendPacket(tempCharArray);
      break;

    // GX get the time that an LCD screen is displayed for (in seconds, eg 2seconds
    case 22599:
      Comms_DebugPrint("Get Page Display Time : ");
      Comms_DebugPrintln((int) mySetupData->get_oledpagetime());
      sprintf(tempCharArray, "%02X", (int) mySetupData->get_oledpagetime());
      SendPacket(tempCharArray);
      break;

    // GZ get the current temperature
    case 23111:
      dtostrf(lasttemp, 4, 3, tempCharArray);
      SendPacket(tempCharArray);
      break;

    // MR get Motor Speed
    case 21069:
      Comms_DebugPrint("Set Motor Speed : ");
      Comms_DebugPrintln((int) mySetupData->get_motorspeed());
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_motorspeed());
      SendPacket(tempCharArray);
      break;

    // MS set motorSpeed - time delay between pulses, acceptable values are 00, 01 and 02 which
    // correspond to a slow, med, high
    case 21325:
      // myfocuser command
      Comms_DebugPrint("Set Motor Speed : ");
      Comms_DebugPrintln((byte)(paramval & 0x03));
      mySetupData->set_motorspeed((byte)(paramval & 0x03));
      break;

    // MT set the MotorSpeed Threshold
    case 21581:
      // myfocuser command
      Comms_DebugPrintln(":MT# not implemented");
      // ignore, motorspeedchange not implemented
      break;

    // MU get the MotorSpeed Threshold
    case 21837:
      Comms_DebugPrintln(":MU# not implemented");
      //sprintf(tempCharArray, "%02d", THRESHOLD);
      SendPacket("00");
      break;

    // MV Set Enable/Disable motorspeed change when moving
    case 22093:
      Comms_DebugPrintln(":MV# not implemented");
      // ignore, motorspeedchange not implemented
      break;

    // MW get if motorspeedchange enabled/disabled
    case 22349:
      Comms_DebugPrintln(":MW# not implemented");
      //sprintf(tempCharArray, "%02d", (int) motorspeedchange);
      SendPacket("00");
      break;

    // MX Save settings to File System
    case 22605:
      // copy current settings and write the data to file
      mySetupData->set_fposition(driverboard->getposition());       // need to save setting
      mySetupData->SaveNow();
      break;

    // PG get temperature precision (9-12)
    case 18256:
      Comms_DebugPrint("get temperature precision : ");
      Comms_DebugPrintln((int) mySetupData->get_tempresolution());
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_tempresolution() );
      SendPacket(tempCharArray);
      break;

    // PN get update of position on lcd when moving (00=disable, 01=enable)
    case 20048:
      Comms_DebugPrint("get update of position on lcd when moving : ");
      Comms_DebugPrintln((int) mySetupData->get_oledupdateonmove());
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_oledupdateonmove());
      SendPacket(tempCharArray);
      break;

    // PM set update of position on lcd when moving (00=disable, 01=enable)
    case 19792:
      Comms_DebugPrint("set update of position on lcd when moving : ");
      Comms_DebugPrintln((int)(paramval & 0x01));
      mySetupData->set_oledupdateonmove((int)(paramval & 0x01));
      break;

    // PP set the step size value - double type, eg 2.1
    case 20560:
      {
        Comms_DebugPrint("set stepsize : ");
        Comms_DebugPrintln(WorkString);
        float tempstepsize = (float)WorkString.toFloat();
        tempstepsize = (tempstepsize < MINIMUMSTEPSIZE ) ? MINIMUMSTEPSIZE : tempstepsize;
        tempstepsize = (tempstepsize > MAXIMUMSTEPSIZE ) ? MAXIMUMSTEPSIZE : tempstepsize;
        mySetupData->set_stepsize(tempstepsize);
      }
      break;

    // PQ get if stepsize is enabled in controller (1 or 0, 0/1)
    case 20816:
      Comms_DebugPrint("get if stepsize is enabled : ");
      Comms_DebugPrintln((int) mySetupData->get_stepsizeenabled());
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_stepsizeenabled());
      SendPacket(tempCharArray);
      break;

    // PR get step size in microns (if enabled by controller)
    case 21072:
      Comms_DebugPrint("get step size in microns : ");
      Comms_DebugPrintln(mySetupData->get_stepsize());
      sprintf(tempCharArray, "%02f", mySetupData->get_stepsize());
      SendPacket(tempCharArray);
      break;

    // PS set temperature precision (9-12 = 0.5, 0.25, 0.125, 0.0625)
    case 21328:
      Comms_DebugPrint("set temperature precision : ");
      Comms_DebugPrintln(paramval);
      // Note paramval is in hex, 9, A, B, C is 9,10,11,12
      paramval = (paramval < 9) ? 9 : paramval;
      paramval = (paramval > 12) ? 12 : paramval;
      Comms_DebugPrint("Probe precision: ");
      Comms_DebugPrintln(paramval);
      if ( mySetupData->get_temperatureprobestate() == 1 )    // if temp probe is enabled
      {
        if ( tprobe1 != 0 )                                   // if probe was found
        {
          myTempProbe->temp_setresolution((byte)paramval);    // set probe resolution
          mySetupData->set_tempresolution((byte)paramval);    // and save for future use
        }
        else
        {
          Comms_DebugPrintln("Probe not found");
        }
      }
      else
      {
        Comms_DebugPrintln("Probe not enabled");
      }
      break;

    // PZ set stepsize enabled to be OFF - default (0) or ON (1)
    case 23120:
      Comms_DebugPrint("Set step size enabled : ");
      Comms_DebugPrintln((byte)(paramval & 0x01));
      mySetupData->set_stepsizeenabled((byte)(paramval & 0x01));
      break;

    // SO set the coilPwr setting
    case 20307:
      Comms_DebugPrint("Set Coil Power : ");
      Comms_DebugPrintln((byte)(paramval & 0x01));
      mySetupData->set_coilpower((byte)(paramval & 0x01));
      break;

    // SR set the Reverse Direction setting
    case 21075:
      Comms_DebugPrint("Set reverse direction : ");
      Comms_DebugPrintln((byte)(paramval & 0x01));
      mySetupData->set_reversedirection((byte)(paramval & 0x01));
      break;

    // SS set stepmode
    // Basic rule for setting stepmode in this order
    // 1. Set mySetupData->set_brdstepmode(xx);       // this saves config setting
    // 2. Set driverboard->setstepmode(xx);           // this sets the physical pins
    case 21331:
      Comms_DebugPrint("Set Step Mode : ");
      Comms_DebugPrintln((int)paramval);
      mySetupData->set_brdstepmode((int)(paramval));
      driverboard->setstepmode((int)(paramval));
      break;

    // SM set new maxSteps position SMXXXX
    case 19795:
      Comms_DebugPrint("Set Maxstep : ");
      Comms_DebugPrintln(paramval);
      // check to make sure not above largest value for maxstep
      paramval = (paramval > FOCUSERUPPERLIMIT) ? FOCUSERUPPERLIMIT : paramval;
      // check if below lowest set value for maxstep
      paramval = (paramval < FOCUSERLOWERLIMIT) ? FOCUSERLOWERLIMIT : paramval;
      // check to make sure its not less than current focuser position
      paramval = (paramval < driverboard->getposition()) ? driverboard->getposition() : paramval;
      // for NEMA17 at 400 steps this would be 5 full rotations of focuser knob
      // for 28BYG-28 this would be less than 1/2 a revolution of focuser knob
      mySetupData->set_maxstep(paramval);
      break;

    // SX set updatedisplayNotMoving (length of time an LCD page is displayed for in milliseconds
    case 22611:
      Comms_DebugPrint("Set page display time : ");
      Comms_DebugPrintln(paramval);
      if ( paramval < OLEDPAGETIMEMIN )
      {
        paramval = OLEDPAGETIMEMIN;
      }
      if ( paramval > OLEDPAGETIMEMAX )
      {
        paramval = OLEDPAGETIMEMAX;
      }
      mySetupData->set_oledpagetime(paramval);
      break;

    // SY set new maxIncrement SYXXXX
    case 22867:
      Comms_DebugPrintln(":SY# not implemented");
      // myfocuser command
      // ignore
      break;

    // TA  Reboot Arduino
    case 16724:
      Comms_DebugPrintln(":TA# reboot controller");
      software_Reboot(2000);
      break;

    default:
      Comms_DebugPrintln("Unknown command");
      break;
  }
}

void clearSerialPort()
{
  while (Serial.available())
  {
    Serial.read();
  }
}

void processserial()
{
  // Serial.read() only returns a single char so build a command line one char at a time
  // : starts the command, # ends the command, do not store these in the command buffer
  // read the command until the terminating # character
  while (Serial.available() )
  {
    char inChar = Serial.read();
    switch ( inChar )
    {
      case STARTCMDSTR :     // start
        serialline = "";
        break;
      case '\r' :
      case '\n' :
        // ignore
        break;
      case EOFSTR :       // eoc
        queue.push(serialline);
        break;
      default :           // anything else
        serialline = serialline + inChar;
        break;
    }
  }
}

#if (CONTROLLERMODE == BLUETOOTHMODE)
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
      case STARTCMDSTR :     // start
        btline = "";
        break;
      case '\r' :
      case '\n' :
        // ignore
        break;
      case EOFSTR :       // eoc
        queue.push(btline);
        break;
      default :           // anything else
        btline = btline + inChar;
        break;
    }
  }
}
#endif // #if (CONTROLLERMODE == BLUETOOTHMODE)

#endif // #if (PROTOCOL == MOONLITE_PROTOCOL)
#endif // #ifndef moonlitecomms_h
