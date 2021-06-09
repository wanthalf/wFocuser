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

extern void  software_Reboot(int);

// ======================================================================
// LOCAL DATA [for moonlite code]
// ======================================================================
int THRESHOLD = 200;             // position at which stepper slows down at it approaches target position
int motorspeedchange = 1;        // should motor speed change if nearing THRESHOLD
int savedmotorSpeed;

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
  int    tmpint;
  int    cmdval;
  long   paramval = 0;
  String cmdString = "";
  String receiveString = "";
  String WorkString = "";
  String replystr = "";
  char   tempCharArray[32];
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
    WorkString.toCharArray(tempCharArray, sizeof(WorkString));
    paramval = hexstr2long(tempCharArray);
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
    // GB   Get the current RED Led Backlight value, Unsigned Hexadecimal
    case 16967:
      // not implemented in INDI driver
      SendPacket("00");
      break;
    // GV   Get current firmware version
    case 22087:
      SendPacket("10");
      break;
    // :SPxxxx# Set current position to received position - no move SPXXXX
    case 20563:
      // in INDI driver, only used to set to 0 SP0000 in reset()
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
      tmpint = (int) paramval;
      switch (tmpint)
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
    // :PO# SET temperature calibration offset POXX in 0.5 degree increments (hex)
    case 20304:
      {
        // this adds/subtracts an offset from the temperature reading in 1/2 degree C steps
        // FA -3, FB -2.5, FC -2, FD -1.5, FE -1, FF -.5, 00 0, 01 0.5, 02 1.0, 03 1.5, 04 2.0, 05 2.5, 06 3.0
        mtempoffsetval = 0.0;
        // param is a char []
        String parm1 = WorkString;
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
    // PS   xx    Adjust Temperature Scale, Signed Hexadecimal
    case 21328: // :PS    Set temperature precision (9-12 = 0.5, 0.25, 0.125, 0.0625)
      tmpint = (int)(paramval);
      tmpint = (tmpint < 9) ? 9 : tmpint;
      tmpint = (tmpint > 12) ? 12 : tmpint;
      if ( mySetupData->get_temperatureprobestate() == 1 )    // if temp probe is enabled
      {
        if ( tprobe1 != 0 )                                   // if probe was found
        {
          myTempProbe->temp_setresolution(tmpint);           // set probe resolution
          mySetupData->set_tempresolution(tmpint);           // and save for future use
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

    //case 21072: // PR   xx    Adjust Red Backlight Brightness
    //  Comms_DebugPrintln(":PR# ignored");
    //  break;
    //case 18256: // PG Adjust Green Backlight Brightness
    //  Comms_DebugPrintln(":PG# ignored");
    //  break;
    case 16976: // PB Adjust Blue Backlight Brightness
      Comms_DebugPrintln(":PB# ignored");
      break;
    case 17232: // PC Adjust LCD Contrast
      Comms_DebugPrintln(":PC# ignored");
      break;
    case 22608: // PX Adjust the Scale for Motor 1
      Comms_DebugPrintln(":PX# ignored");
      break;
    case 18512: // PH Find home for Motor, valid options are "01", "02"
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
        short int tcval = (short int)(paramval);
        sscanf(tempCharArray, "%hd", &tcval);         // h is half or short, signed
        mySetupData->set_tempcoefficient((byte) tcval);
      }
      break;
    case 43: // + activate temperature compensation focusing
      mySetupData->set_tempcompenabled(1);
      break;
    case 45: // - disable temperature compensation focusing
      mySetupData->set_tempcompenabled(0);
      break;

    // now list the myFocuserPro extensions to Moonlite
    case 17991: // :GF# get firmware value
      {
        String reply = "myFP2ESP-M\r" + String(programVersion);
        reply.toCharArray(tempCharArray, reply.length() + 1);
        SendPacket(tempCharArray);
      }
      break;
    case 19783: // :GM# get the MaxSteps
    case 22855: // :GY# get the maxIncrement - set to MaxSteps
      sprintf(tempCharArray, "%04X", (unsigned int) mySetupData->get_maxstep());
      SendPacket(tempCharArray);
      break;
    case 21319: // get stepmode
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_brdstepmode());
      SendPacket(tempCharArray);
      break;
    case 20295: // :GO# get the coilPwr setting
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_coilpower());
      SendPacket(tempCharArray);
      break;
    case 21063: // :GR# get the Reverse Direction setting
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_reversedirection());
      SendPacket(tempCharArray);
      break;
    case 21069: // :MR# get Motor Speed
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_motorspeed());
      SendPacket(tempCharArray);
      break;
    case 21837: // :MU# get the MotorSpeed Threshold
      sprintf(tempCharArray, "%02d", THRESHOLD);
      SendPacket(tempCharArray);
      break;
    case 22349: // :MW# get if motorspeedchange enabled/disabled
      sprintf(tempCharArray, "%02d", (int) motorspeedchange);
      SendPacket(tempCharArray);
      break;
    case 18244: // :DG# get display state on or off
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_displayenabled());
      SendPacket(tempCharArray);
      break;
    case 22599: // :GX# get the time that an LCD screen is displayed for (in milliseconds, eg 2500 = 2.5seconds
      sprintf(tempCharArray, "%02X", (int) mySetupData->get_oledpagetime());
      SendPacket(tempCharArray);
      break;
    case 18256: // :PG get temperature precision (9-12)
      sprintf(tempCharArray, "%02d",(int)mySetupData->get_tempresolution() );
      SendPacket(tempCharArray);
      break;
    case 20048: // :PN# get update of position on lcd when moving (00=disable, 01=enable)
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_oledupdateonmove());
      SendPacket(tempCharArray);
      break;
    case 20816: // :PQ# get if stepsize is enabled in controller (1 or 0, 0/1)
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_stepsizeenabled());
      SendPacket(tempCharArray);
      break;
    case 21072: // :PR# get step size in microns (if enabled by controller)
      sprintf(tempCharArray, "%02f", mySetupData->get_stepsize());
      SendPacket(tempCharArray);
      break;
    case 19782: // :FM# get Display temp mode (Celsius=1, Fahrenheit=0)
      sprintf(tempCharArray, "%02d", (int) mySetupData->get_tempmode());
      SendPacket(tempCharArray);
      break;
    case 21331: // set stepmode
      tmpint = (int)(paramval);
      driverboard->setstepmode(tmpint);
      break;
    case 20307: // :SOxxxx# set the coilPwr setting
      tmpint = (int)(paramval & 0x01);
      mySetupData->set_coilpower(tmpint);
      break;
    case 21075: // :SRxx# set the Reverse Direction setting
      tmpint = (int)(paramval & 0x01);
      mySetupData->set_reversedirection(tmpint);
      break;
    case 19780: // :DMx# set displaystate C or F
      tmpint = (int)(paramval & 0x01);
      mySetupData->set_tempmode(tmpint);
      break;
    case 21325: // set motorSpeed - time delay between pulses, acceptable values are 00, 01 and 02 which
      // correspond to a slow, med, high
      // myfocuser command
      tmpint = (int)(paramval & 0x03);
      savedmotorSpeed = tmpint;           // remember the speed setting
      mySetupData->set_motorspeed(tmpint);
      break;
    case 21581: // :MTxxx# set the MotorSpeed Threshold
      // myfocuser command
      tmpint = (int)(paramval);
      if ( tmpint < 50 )
      {
        tmpint = 50;
      }
      else if ( tmpint > 200 )
      {
        tmpint = 200;
      }
      THRESHOLD = tmpint;
      break;
    case 22093: // :MVx# Set Enable/Disable motorspeed change when moving
      tmpint = (int)(paramval & 0x01);
      motorspeedchange = tmpint;
      break;
    case 22605: // :MX# Save settings to EEPROM
      // copy current settings and write the data to EEPROM
      mySetupData->set_fposition(driverboard->getposition());       // need to save setting
      mySetupData->SaveNow();
      break;
    case 19795: // :SMxxx# set new maxSteps position SMXXXX
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
    case 22867: // :SYxxxx# set new maxIncrement SYXXXX
      // myfocuser command
      // ignore
      break;
    case 21316: // :DSx# disable or enable the display setting
      if ( displaystate == true )
      {
        tmpint = (int)(paramval & 0x01);
        if ( tmpint == 0 )
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
    case 22611: // :SXxxxx# None    Set updatedisplayNotMoving (length of time an LCD page is displayed for in milliseconds
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
    case 16724: // :TA#  Reboot Arduino
      software_Reboot(2000);
      break;
    case 19792: // :PMxx# set update of position on lcd when moving (00=disable, 01=enable)
      tmpint = (int)(paramval & 0x01);
      mySetupData->set_oledupdateonmove(tmpint);
      break;
    case 23111: // :GZ# get the current temperature
      dtostrf(lasttemp, 4, 3, tempCharArray);
      SendPacket(tempCharArray);
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
