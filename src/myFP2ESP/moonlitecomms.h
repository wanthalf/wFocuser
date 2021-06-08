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
#include "focuserconfig.h"                  // boarddefs.h included as part of focuserconfig.h"

#if (PROTOCOL == MOONLITE_PROTOCOL)

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

extern void  software_Reboot(int);

// ======================================================================
// LOCAL DATA
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
// In INDI/KSTARS you can select the Moonlite driver
// IN Windows you can select the Moonlitw ASCOM driver

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
#elif (CONTROLLERMODE == BLUETOOTHMODE)  // for bluetooth
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
  int    cmdval;
  long   paramval = 0;
  String cmdString = "";
  String receiveString = "";
  String WorkString = "";
  String replystr = "";
  char   tempCharArray[12];
  static unsigned long newtargetPosition;
  static boolean newtargetpositionset = false;
  static float   mtempoffsetval = 0.0;

#if (CONTROLLERMODE == BLUETOOTHMODE)
  receiveString = STARTCMDSTR + queue.pop();
#elif (CONTROLLERMODE == LOCALSERIAL)
  receiveString = STARTCMDSTR + queue.pop();
#endif

  Comms_DebugPrint("raw receive string=");
  Comms_DebugPrintln(receiveString);

  receiveString = receiveString.substring(1, receiveString.length());    // remove leading :
  Comms_DebugPrint("receive string=");
  Comms_DebugPrintln(receiveString);

  if ( receiveString.length() < 2 )
  {
    cmdString = receiveString.substring(0, 1);                  // take care of :C#
    cmdval = (int) cmdString[0];
  }
  else
  {
    cmdString = receiveString.substring(0, 2);                  // commands are always 2 chars
    cmdval = (int) cmdString[0] + ((int) cmdString[1] * 256);   // for moonlite get first two chars and generate a value
  }
  Comms_DebugPrint("cmdstr=");
  Comms_DebugPrintln(cmdString);
  Comms_DebugPrint("cmdval=");
  Comms_DebugPrintln(cmdval);
  if ( receiveString.length() > 2)
  {
    WorkString = receiveString.substring(2, receiveString.length() ); // extract any parameters associated with the command
  }
  Comms_DebugPrint("WorkString = ");
  Comms_DebugPrintln(WorkString);

  switch (cmdval)
  {
    // :GP# get the current focuser position
    case 20551:
      sprintf(tempCharArray, "%04X", (unsigned int) driverboard->getposition());
      SendPacket(tempCharArray);
      break;
    // :GN# get the new motor position (target)
    case 20039:
      // not implemented in INDI driver
      sprintf(tempCharArray, "%04X", (unsigned int) newtargetPosition);
      SendPacket(tempCharArray);
      break;
    // :GT# get the current temperature
    // Do not forget to apply temperature offset (mtempoffsetval = 0.0)
    case 21575:
      {
        float tmp = ((lasttemp + mtempoffsetval) * 2);
        sprintf(tempCharArray, "%04X", (int) tmp);
        SendPacket(tempCharArray);
      }
      break;
    // :GD# Get the Motor 1 speed, valid options are "02, 04, 08, 10, 20"
    case 17479:
      switch (mySetupData->get_motorspeed() )
      {
        case SLOW:
          sprintf(tempCharArray, "%02X", 8);      // slow
          break;
        case MED:
          sprintf(tempCharArray, "%02X", 4);      // medium
          break;
        case FAST:
          sprintf(tempCharArray, "%02X", 2);      // fast
          break;
        default:
          sprintf(tempCharArray, "%02X", 2);
          break;
      }
      SendPacket(tempCharArray);
      break;
    // GH   XX    "FF" if half step is set, otherwise "00"
    case 18503:
      if ( mySetupData->get_brdstepmode() == STEP2 )
      {
        replystr = "FF";
      }
      else
      {
        replystr = "00";
      }
      replystr.toCharArray(tempCharArray, replystr.length() + 1);
      SendPacket(tempCharArray);
      break;
    // GI   XX    "01" if the motor is moving, otherwise "00"
    case 18759:
      if ( isMoving == 1 )
      {
        replystr = "01";
      }
      else
      {
        replystr = "00";
      }
      replystr.toCharArray(tempCharArray, replystr.length() + 1);
      SendPacket(tempCharArray);
      break;
    // GB   XX    The current RED Led Backlight value, Unsigned Hexadecimal
    case 16967:
      // not implemented in INDI driver
      SendPacket("00");
      break;
    // GV   XX    Code for current firmware version
    case 22087:
      SendPacket("10");
      break;
    // :SPxxxx# Set current position to received position - no move SPXXXX
    case 20563:
      // in INDI driver, only used to set to 0 SP0000 in reset()
      WorkString.toCharArray(tempCharArray, sizeof(WorkString));
      paramval = hexstr2long(tempCharArray);
      paramval = (paramval < 0) ? 0 : paramval;
      if ( paramval > mySetupData->get_maxstep())
      {
        paramval = mySetupData->get_maxstep();
      }
      ftargetPosition = paramval;
      mySetupData->set_fposition(ftargetPosition);
      driverboard->setposition(ftargetPosition);
      break;
    // :SNxxxx# set new target position SNXXXX - this is a move command
    // but must be followed by a FG command to start the move
    case 20051:
      WorkString.toCharArray(tempCharArray, sizeof(WorkString));
      paramval = hexstr2long(tempCharArray);
      paramval = (paramval < 0) ? 0 : paramval;
      if ( paramval > mySetupData->get_maxstep())
      {
        paramval = mySetupData->get_maxstep();
      }
      newtargetPosition = paramval;
      newtargetpositionset = true;
      break;
    // Basic rule for setting stepmode in this order
    // 1. Set mySetupData->set_brdstepmode(xx);       // this saves config setting
    // 2. Set driverboard->setstepmode(xx);           // this sets the physical pins
    // SF       Set Motor 1 to Full Step
    case 18003:
      mySetupData->set_brdstepmode(STEP1);
      driverboard->setstepmode(STEP1);
      break;
    // SH       Set Motor 1 to Half Step
    case 18515:
      mySetupData->set_brdstepmode(STEP2);
      driverboard->setstepmode(STEP2);
      break;
    // SD   xx    Set the Motor 1 speed, valid options are "02, 04, 08, 10, 20"
    // correspond to a stepping delay of 250, 125, 63, 32 and 16 steps
    // per second respectively. Moonlite only
    case 17491:
      WorkString.toCharArray(tempCharArray, sizeof(WorkString));
      paramval = hexstr2long(tempCharArray);
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
      }
      break;
    // FG        Start a Motor 1 move, moves the motor to the New Position.
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
    // FQ       Halt Motor 1 move, position is retained, motor is stopped.
    case 20806:
      varENTER_CRITICAL(&halt_alertMux);
      halt_alert = true;
      varEXIT_CRITICAL(&halt_alertMux);
      Comms_DebugPrintln("FQ: halt_alert = true");
      break;
    // :PO# temperature calibration offset POXX in 0.5 degree increments (hex)
    case 20304:
      {
        // this adds/subtracts an offset from the temperature reading in 1/2 degree C steps
        // FA -3, FB -2.5, FC -2, FD -1.5, FE -1, FF -.5, 00 0, 01 0.5, 02 1.0, 03 1.5, 04 2.0, 05 2.5, 06 3.0
        mtempoffsetval = 0.0;
        // param is a char []
        String parm1 = WorkString;
        if ( parm1 == "FA" )
          mtempoffsetval = -3.0;
        else if ( parm1 == "FB")
          mtempoffsetval = -2.5;
        else if ( parm1 == "FC")
          mtempoffsetval = -2.0;
        else if ( parm1 == "FD")
          mtempoffsetval = -1.5;
        else if ( parm1 == "FE")
          mtempoffsetval = -1.0;
        else if ( parm1 == "FF")
          mtempoffsetval = -0.5;
        else if ( parm1 == "00")
          mtempoffsetval = 0.0;
        else if ( parm1 == "01")
          mtempoffsetval = 0.5;
        else if ( parm1 == "02")
          mtempoffsetval = 1.0;
        else if ( parm1 == "03")
          mtempoffsetval = 1.5;
        else if ( parm1 == "04")
          mtempoffsetval = 2.0;
        else if ( parm1 == "05")
          mtempoffsetval = 2.5;
        else if ( parm1 == "06")
          mtempoffsetval = 3.0;
      }
      break;
    // PS   xx    Adjust Temperature Scale, Signed Hexadecimal
    case 21328: // :PS    Set temperature precision (9-12 = 0.5, 0.25, 0.125, 0.0625)
      paramval = WorkString.toInt();
      paramval = (paramval < 9) ? 9 : paramval;
      paramval = (paramval > 12) ? 12 : paramval;
      mySetupData->set_tempresolution((byte) paramval);
      if ( mySetupData->get_temperatureprobestate() == 1 )    // if temp probe is enabled
      {
        if ( tprobe1 != 0 )                                   // if probe was found
        {
          myTempProbe->temp_setresolution((byte) paramval);   // set probe resolution
          mySetupData->set_tempresolution((byte) paramval);   // and save for future use
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
    // PR   xx    Adjust Red Backlight Brightness
    case 21072:
      Comms_DebugPrintln(":PR# ignored");
      break;
    // PG   xx    Adjust Green Backlight Brightness
    case 18256:
      Comms_DebugPrintln(":PG# ignored");
      break;

    // PB   xx    Adjust Blue Backlight Brightness
    case 16976:
      Comms_DebugPrintln(":PB# ignored");
      break;

    // PC   xx    Adjust LCD Contrast
    case 17232:
      Comms_DebugPrintln(":PC# ignored");
      break;

    // PX   xxxx    Adjust the Scale for Motor 1
    case 22608:
      Comms_DebugPrintln(":PX# ignored");
      break;

    // PH   xx    Find home for Motor, valid options are "01", "02"
    case 18512:
      // not implemented in INDI driver
      ftargetPosition = 0;
      break;

    // :C#  N/A   Initiate a temperature conversion
    case 67:
      Comms_DebugPrintln(":C# ignored");
      break;

    // :GC# XX#   Returns the temperature coefficient where XX is a two-digit signed (2’s complement)
    case 17223:
      sprintf(tempCharArray, "%02X", mySetupData->get_tempcoefficient());
      SendPacket(tempCharArray);
      break;

    case 17235: // :SCxx# set temperature co-efficient XX
      // Set the new temperature coefficient where XX is a two-digit, signed (2’s complement) hex number
      // MSB = 1 is negative, MSB = 0 number is positive
      // The two's complement is calculated by inverting the digits and adding one
      {
        short byteval;
        WorkString.toCharArray(tempCharArray, sizeof(WorkString));
        sscanf(tempCharArray, "%hd", &byteval);       // h is half or short, signed
        mySetupData->set_tempcoefficient((byte)byteval);
        if ( byteval < 0 )
        {
          mySetupData->set_tcdirection(1);            // negative temperature coefficient, focuser tube moves out
        }
        else
        {
          mySetupData->set_tcdirection(0);            // positive temperature coefficient, focuser tube moves in
        }
      }
      break;
    case 43: // + activate temperature compensation focusing
      mySetupData->set_tempcompenabled(1);
      break;
    case 45: // - disable temperature compensation focusing
      mySetupData->set_tempcompenabled(0);
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
