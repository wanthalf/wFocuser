// ======================================================================
// comms.h : myFP2ESP COMMS ROUTINES AND DEFINITIONS
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

#ifndef comms_h
#define comms_h

#include "generalDefinitions.h"
#include "focuserconfig.h"                  // boarddefs.h included as part of focuserconfig.h"

// ======================================================================
// EXTERNS
// ======================================================================

extern volatile bool halt_alert;
#if defined(ESP8266)
// in esp8266, volatile data_type varname is all that is needed
#else
// in esp32, we should use a Mutex for access
extern portMUX_TYPE  halt_alertMux;
#endif

extern OLED_NON *myoled;

extern byte          isMoving;
extern char          ipStr[];
extern int           tprobe1;
extern float         lasttemp;
extern char          mySSID[64];
extern const char*   programVersion;
extern unsigned long ftargetPosition;          // target position
extern bool          displaystate;
extern SetupData     *mySetupData;
extern DriverBoard   *driverboard;
extern TempProbe     *myTempProbe;

extern void  software_Reboot(int);
extern void  start_ascomremoteserver(void);
extern void  stop_ascomremoteserver(void);
extern void  start_webserver();
extern void  stop_webserver();
extern long  getrssi(void);
extern void  software_Reboot(int);
extern bool  init_leds(void);
extern bool  init_pushbuttons(void);

// ======================================================================
// DATA
// ======================================================================
String board_data;

// ======================================================================
// CODE
// ======================================================================

// forward declaration to keep compiler happy
void send_boardconfig_file(void);


char *ftoa(char *a, double f, int precision)
{
  long p[] = {0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000};

  char *ret = a;
  long heiltal = (long)f;
  itoa(heiltal, a, 10);
  while (*a != '\0') a++;
  *a++ = '.';
  long desimal = abs((long)((f - heiltal) * p[precision]));
  itoa(desimal, a, 10);
  return ret;
}

void SendMessage(const char *str)
{
  Comms_DebugPrint("Send:");
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

void SendPaket(const char token, String str, bool bigstr)
{
  char mbuffer[320];
  board_data.toCharArray(mbuffer, board_data.length() + 1);
  //snprintf(buffer, sizeof(buffer), "%c%s%c", token,  str, EOFSTR);
  SendMessage(mbuffer);
}

void SendPaket(const char token, const char *str)
{
  char mbuffer[32];
  snprintf(mbuffer, sizeof(mbuffer), "%c%s%c", token,  str, EOFSTR);
  SendMessage(mbuffer);
}

void SendPaket(const char token, const unsigned char val)
{
  char mbuffer[32];

  snprintf(mbuffer, sizeof(mbuffer), "%c%u%c", token,  val, EOFSTR);
  SendMessage(mbuffer);
}

void SendPaket(const char token, const int val)
{
  char mbuffer[32];
  snprintf(mbuffer, sizeof(mbuffer), "%c%i%c", token,  val, EOFSTR);
  SendMessage(mbuffer);
}

void SendPaket(const char token, const unsigned long val)
{
  char mbuffer[32];
  snprintf(mbuffer, sizeof(mbuffer), "%c%lu%c", token,  val, EOFSTR);
  SendMessage(mbuffer);
}

void SendPaket(const char token, const long val)
{
  char mbuffer[32];
  snprintf(mbuffer, sizeof(mbuffer), "%c%ld%c", token,  val, EOFSTR);
  SendMessage(mbuffer);
}

void SendPaket(const char token, const float val, int i)    // i => decimal place
{
  char buff[32];
  char temp[10];
  // Note Arduino snprintf does not support .2f
  //dtostrf(val, 7, i, temp);
  ftoa(temp, val, i);
  snprintf(buff, sizeof(buff), "%c%s%c", token, temp, EOFSTR);
  SendMessage(buff);
}

void ESP_Communication()
{
  byte cmdval;
  String receiveString = "";
  String WorkString = "";
  long paramval = 0;
  String drvbrd = mySetupData->get_brdname();

#if (CONTROLLERMODE == BLUETOOTHMODE)
  receiveString = STARTCMDSTR + queue.pop();
#elif (CONTROLLERMODE == LOCALSERIAL)
  receiveString = STARTCMDSTR + queue.pop();
#else   // for Accesspoint or Station mode
  packetsreceived++;
  receiveString = myclient.readStringUntil(EOFSTR);    // read until terminator
#endif

  receiveString += EOFSTR;                                // put back terminator
  String cmdstr = receiveString.substring(1, 3);
  cmdval = cmdstr.toInt();                                // convert command to an integer
  Comms_DebugPrint("recstr=" + receiveString + "  ");
  Comms_DebugPrintln("cmdstr=" + cmdstr);
  switch (cmdval)
  {
    // all the get values first followed by set values
    case 0: // get focuser position
      SendPaket('P', driverboard->getposition());
      break;
    case 1: // ismoving
      SendPaket('I', isMoving);
      break;
    case 2: // get controller status
      SendPaket('E', "OK");
      break;
    case 3: // get firmware version
      SendPaket('F', programVersion);
      break;
    case 4: // get firmware name
      {
        char buffer[32];
        char tempstr[20];
        String brdname = mySetupData->get_brdname();
        brdname.toCharArray(tempstr, brdname.length() + 1);
        snprintf(buffer, sizeof(buffer), "%s\r\n%s", tempstr, programVersion );
        SendPaket('F', buffer);
      }
      break;
    case 5: // :05xxxxxx# None    Set new target position to xxxxxx (and focuser initiates immediate move to xxxxxx)
      // only if not already moving
      if ( isMoving == 0 )
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        ftargetPosition = (unsigned long)WorkString.toInt();
        ftargetPosition = (ftargetPosition > mySetupData->get_maxstep()) ? mySetupData->get_maxstep() : ftargetPosition;
        // main loop will update focuser positions
      }
      break;
    case 6: // get temperature
      SendPaket('Z', lasttemp, 3);
      break;
    case 7: // set maxsteps
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        unsigned long tmppos = (unsigned long)WorkString.toInt();
        delay(5);
        // check to make sure not above largest value for maxstep
        tmppos = (tmppos > FOCUSERUPPERLIMIT) ? FOCUSERUPPERLIMIT : tmppos;
        // check if below lowest set value for maxstep
        tmppos = (tmppos < FOCUSERLOWERLIMIT) ? FOCUSERLOWERLIMIT : tmppos;
        // check to make sure its not less than current focuser position
        tmppos = (tmppos < driverboard->getposition()) ? driverboard->getposition() : tmppos;
        mySetupData->set_maxstep(tmppos);
      }
      break;
    case 8: // get maxStep
      SendPaket('M', mySetupData->get_maxstep());
      break;
    case 10: // get maxIncrement
      SendPaket('Y', mySetupData->get_maxstep());
      break;
    case 11: // get coil power
      SendPaket('O', mySetupData->get_coilpower());
      break;
    case 12: // set coil power
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = (byte) WorkString.toInt();
      ( paramval == 1 ) ? driverboard->enablemotor()    : driverboard->releasemotor();
      ( paramval == 1 ) ? mySetupData->set_coilpower(1) : mySetupData->set_coilpower(0);
      break;
    case 13: // get reverse direction setting, 00 off, 01 on
      SendPaket('R', mySetupData->get_reversedirection());
      break;
    case 14: // set reverse direction
      if ( isMoving == 0 )
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        paramval = (byte) WorkString.toInt();
        ( paramval == 1 ) ? mySetupData->set_reversedirection(1) : mySetupData->set_reversedirection(0);
      }
      break;
    case 15: // set motor speed
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = (byte)WorkString.toInt() & 3;
      mySetupData->set_motorspeed((byte) paramval);
      break;
    case 16: // set display to celsius
      mySetupData->set_tempmode(1); // temperature display mode, Celsius=1, Fahrenheit=0
      break;
    case 17: // set display to fahrenheit
      mySetupData->set_tempmode(0); // temperature display mode, Celsius=1, Fahrenheit=0
      break;
    case 18:
      // :180#    None    set the return of user specified stepsize to be OFF - default
      // :181#    None    set the return of user specified stepsize to be ON - reports what user specified as stepsize
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = (byte)WorkString.toInt();
      mySetupData->set_stepsizeenabled((byte) (paramval));
      break;
    case 19: // :19xxxx#  None   set the step size value - double type, eg 2.1
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        float tempstepsize = (float)WorkString.toFloat();
        tempstepsize = (tempstepsize < MINIMUMSTEPSIZE ) ? MINIMUMSTEPSIZE : tempstepsize;
        tempstepsize = (tempstepsize > MAXIMUMSTEPSIZE ) ? MAXIMUMSTEPSIZE : tempstepsize;
        mySetupData->set_stepsize(tempstepsize);
      }
      break;
    case 20: // set the temperature resolution setting for the DS18B20 temperature probe
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = WorkString.toInt();
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
    case 21: // get temp probe resolution
      SendPaket('Q', mySetupData->get_tempresolution());
      break;
    case 22: // set the temperature compensation value to xxx
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = WorkString.toInt();
      mySetupData->set_tempcoefficient((byte)paramval);
      break;
    case 23: // set the temperature compensation ON (1) or OFF (0)
      if ( mySetupData->get_temperatureprobestate() == 1)
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        paramval = (byte)WorkString.toInt();
        mySetupData->set_tempcompenabled((byte) (paramval));
      }
      break;
    case 24: // get status of temperature compensation (enabled | disabled)
      SendPaket('1', mySetupData->get_tempcompenabled());
      break;
    case 25: // get IF temperature compensation is available
      if ( mySetupData->get_temperatureprobestate() == 1 )
      {
        SendPaket('A', "1"); // this focuser supports temperature compensation
      }
      else
      {
        SendPaket('A', "0");
      }
      break;
    case 26: // get temperature coefficient steps/degree
      SendPaket('B', mySetupData->get_tempcoefficient());
      break;
    case 27: // stop a move - like a Halt
      varENTER_CRITICAL(&halt_alertMux);
      halt_alert = true;
      varEXIT_CRITICAL(&halt_alertMux);
      break;
    case 28: // home the motor to position 0
      ftargetPosition = 0; // if this is a home then set target to 0
      break;
    case 29: // get stepmode
      SendPaket('S', mySetupData->get_brdstepmode());
      break;
    // ======================================================================
    // Basic rule for setting stepmode
    // Set driverboard->setstepmode(xx);                 // this sets the physical pins and saves new stepmode
    // ======================================================================
    case 30: // set step mode
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        paramval = WorkString.toInt();
        int brdnum = mySetupData->get_brdnumber();
        if (brdnum == PRO2EULN2003 || brdnum == PRO2EL298N || brdnum == PRO2EL293DMINI || brdnum == PRO2EL9110S)
        {
          paramval = (int)(paramval & 3);      // STEP1 - STEP2
        }
        else if (brdnum == PRO2ESP32ULN2003 || brdnum == PRO2ESP32L298N || brdnum == PRO2ESP32L293DMINI || brdnum == PRO2ESP32L9110S)
        {
          paramval = (int)(paramval & 3);      // STEP1 - STEP2
        }
        else if (brdnum == WEMOSDRV8825 || brdnum == PRO2EDRV8825 || brdnum == PRO2EDRV8825BIG)
        {
          paramval = (int) mySetupData->get_brdfixedstepmode();            // stepmopde set by jumpers
        }
        else if (brdnum == PRO2ESP32DRV8825 || brdnum == PRO2ESP32R3WEMOS)
        {
          paramval = (paramval < STEP1 ) ? STEP1 : paramval;
          paramval = (paramval > STEP32) ? STEP32 : paramval;
        }
        else if (brdnum == PRO2EL293DNEMA || brdnum == PRO2EL293D28BYJ48)
        {
          paramval = STEP1;
        }
        else if (brdnum == PRO2ESP32TMC2225 || brdnum == PRO2ESP32TMC2209 || brdnum == PRO2ESP32TMC2209P )
        {
          paramval = (paramval < STEP1 )  ? STEP1   : paramval;
          paramval = (paramval > STEP256) ? STEP256 : paramval;
        }
        else
        {
          Comms_DebugPrint("unknown DRVBRD: ");
          Comms_DebugPrintln(brdnum);
        }
      }
      driverboard->setstepmode((int)paramval);
      break;
    case 31: // set focuser position
      if ( isMoving == 0 )
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        {
          long tpos = (long)WorkString.toInt();
          tpos = (tpos < 0) ? 0 : tpos;
          unsigned long tmppos = ((unsigned long) tpos > mySetupData->get_maxstep()) ? mySetupData->get_maxstep() : (unsigned long) tpos;
          ftargetPosition = tmppos;
          driverboard->setposition(tmppos);
          mySetupData->set_fposition(tmppos);
        }
      }
      break;
    case 32: // get if stepsize is enabled
      SendPaket('U', mySetupData->get_stepsizeenabled());
      break;
    case 33: // get stepsize
      SendPaket('T', mySetupData->get_stepsize(), 3);       // ????????????? check format
      break;
    case 34: // get the time that an oled screen is displayed for
      SendPaket('X', mySetupData->get_oledpagetime());
      break;
    case 35: // set length of time an oledpage is displayed for in seconds
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = WorkString.toInt();
      if ( paramval < OLEDPAGETIMEMIN )
      {
        paramval = OLEDPAGETIMEMIN;
      }
      if ( paramval > OLEDPAGETIMEMAX )
      {
        paramval = OLEDPAGETIMEMAX;
      }
      mySetupData->set_oledpagetime((byte)paramval);
      break;
    case 36:
      // :360#    None    Disable Display
      // :361#    None    Enable Display
      if ( displaystate == true )
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        paramval = (byte) WorkString.toInt();
        mySetupData->set_displayenabled((byte) (paramval));
        if (paramval == 1)
        {
          if ( displayfound == true )
          {
            myoled->display_on();
          }
        }
        else
        {
          if ( displayfound == true )
          {
            myoled->display_off();
          }
        }
      }
      break;
    case 37: // get displaystatus
      SendPaket('D', mySetupData->get_displayenabled());
      break;
    case 38: // :38#   Dxx#      Get Temperature mode 1=Celsius, 0=Fahrenheight
      SendPaket('b', mySetupData->get_tempmode());
      break;
    case 39: // get the new motor position (target) XXXXXX
      SendPaket('N', ftargetPosition);
      break;
    case 40: // reset Arduino myFocuserPro2E controller
      software_Reboot(2000);      // reboot with 2s delay
      break;
    case 42: // reset focuser defaults
      if ( isMoving == 0 )
      {
        mySetupData->SetFocuserDefaults();
        ftargetPosition = mySetupData->get_fposition();
        driverboard->setposition(ftargetPosition);
        mySetupData->set_fposition(ftargetPosition);
      }
      break;
    case 43: // get motorspeed
      SendPaket('C', mySetupData->get_motorspeed());
      break;
    case 48: // save settings to FS
      mySetupData->set_fposition(driverboard->getposition());       // need to save setting
      mySetupData->SaveNow();                                       // save the focuser settings immediately
      break;
    case 49: // aXXXXX
      SendPaket('a', "b552efd");
      break;
    case 50: // Get if Home Position Switch enabled, 0 = no, 1 = yes
      if ( mySetupData->get_hpswitchenable() == 1)
      {
        SendPaket('l', 1);
      }
      else
      {
        SendPaket('l', 0);
      }
      break;
    case 51: // return ESP8266Wifi Controller IP Address
      SendPaket('d', ipStr);
      break;
    case 52: // return ESP32 Controller number of TCP packets sent
      SendPaket('e', packetssent);
      break;
    case 53: // return ESP32 Controller number of TCP packets received
      SendPaket('f', packetsreceived);
      break;
    case 54: // return ESP32 Controller SSID
#if (CONTROLLERMODE == LOCALSERIAL)
      SendPaket('g', "SERIAL");
#endif
#if (CONTROLLERMODE == BLUETOOTH)
      SendPaket('g', "BLUETOOTH");
#endif
#if ( (CONTROLLERMODE == ACCESSPOINT) || (CONTROLLERMODE == STATIONMODE) )
      SendPaket('g', mySSID);
#endif
      break;
    case 55: // get motorspeed delay for current speed setting
      SendPaket('0', mySetupData->get_brdmsdelay());
      break;
    case 56: // set motorspeed delay for current speed setting
      {
        int newdelay = 1000;
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        newdelay = WorkString.toInt();
        newdelay = (newdelay < 1000) ? 1000 : newdelay;   // ensure it is not too low
        mySetupData->set_brdmsdelay(newdelay);
      }
      break;
    case 61: // set update of position on oled when moving (0=disable, 1=enable)
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = (byte)WorkString.toInt();
      mySetupData->set_oledupdateonmove((byte) (paramval));
      break;
    case 62: // get update of position on oled when moving (00=disable, 01=enable)
      SendPaket('L', mySetupData->get_oledupdateonmove());
      break;
    case 63: // get status of home position switch (hpsw pin 1=open, 0=closed)
      if ( mySetupData->get_hpswitchenable() == 1)
      {
        SendPaket('H', !digitalRead(mySetupData->get_brdhpswpin()));    // return 1 if closed, 0 if open
      }
      else
      {
        SendPaket('H', "0");
      }
      break;
    case 64: // move a specified number of steps
      if ( isMoving == 0 )
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        long pos = (long)WorkString.toInt() + (long)driverboard->getposition();
        pos  = (pos < 0) ? 0 : pos;
        ftargetPosition = ( pos > (long)mySetupData->get_maxstep()) ? mySetupData->get_maxstep() : (unsigned long)pos;
      }
      break;
    case 71: // set DelayAfterMove in milliseconds
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      mySetupData->set_DelayAfterMove((byte)WorkString.toInt());
      break;
    case 72: // get DelayAfterMove
      SendPaket('3', mySetupData->get_DelayAfterMove());
      break;
    case 73: // Disable/enable backlash IN (going to lower focuser position)
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = (byte)WorkString.toInt();
      mySetupData->set_backlash_in_enabled((byte) (paramval));
      break;
    case 74: // get backlash in enabled status
      SendPaket('4', mySetupData->get_backlash_in_enabled());
      break;
    case 75: // Disable/enable backlash OUT (going to lower focuser position)
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = (byte)WorkString.toInt();
      mySetupData->set_backlash_out_enabled((byte) (paramval));
      break;
    case 76: // get backlash OUT enabled status
      SendPaket('5', mySetupData->get_backlash_out_enabled());
      break;
    case 77: // set backlash in steps
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      mySetupData->set_backlashsteps_in((byte)WorkString.toInt());
      break;
    case 78: // return number of backlash steps IN
      SendPaket('6', mySetupData->get_backlashsteps_in());
      break;
    case 79: // set backlash OUT steps
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      mySetupData->set_backlashsteps_out((byte)WorkString.toInt());
      break;
    case 80: // return number of backlash steps OUT
      SendPaket('7', mySetupData->get_backlashsteps_out());
      break;
    case 81: // get STALL_VALUE (for TMC2209 stepper modules)
      SendPaket('8', mySetupData->get_stallguard());
      break;
    case 82: // set STALL_VALUE (for TMC2209 stepper modules)
      WorkString = receiveString.substring(2, receiveString.length() );
      driverboard->setstallguard( (byte) WorkString.toInt() );
      break;
    case 83: // get if there is a temperature probe
      SendPaket('c', tprobe1);
      break;
    case 87: // get tc direction
      SendPaket('k', mySetupData->get_tcdirection());
      break;
    case 88: // set tc direction
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramval = (byte)WorkString.toInt();
      mySetupData->set_tcdirection((byte) (paramval));
      break;
    case 89:  // Get stepper power (reads from A7) - only valid if circuit is added (1=stepperpower ON)
      SendPaket('9', 1);
      break;
    case 90: // Set preset x [0-9] with position value yyyy [unsigned long]
      {
        byte preset = (byte) (receiveString[3] - '0');
        preset = (preset > 9) ? 9 : preset;
        WorkString = receiveString.substring(4, receiveString.length() - 1);
        unsigned long tmppos = (unsigned long) WorkString.toInt();
        mySetupData->set_focuserpreset( preset, tmppos );
      }
      break;
    case 91: // get focuserpreset [0-9]
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        paramval = (byte)WorkString.toInt();
        byte preset = (byte) (paramval);
        preset = (preset > 9) ? 9 : preset;
        SendPaket('h', mySetupData->get_focuserpreset(preset));
      }
      break;
    case 92: // Set OLED page display option
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        if ( WorkString == "" )                                     // check for null
        {
          WorkString = String(OLEDPGOPTIONALL, BIN);
        }
        if ( WorkString.length() != 3  )                            // check for 3 digits
        {
          WorkString = String(OLEDPGOPTIONALL, BIN);
        }
        for ( unsigned int i = 0; i < WorkString.length(); i++)     // check for 0 or 1
        {
          if ( (WorkString[i] != '0') && (WorkString[i] != '1') )
          {
            WorkString = String(OLEDPGOPTIONALL, BIN);
            break;
          }
        }
        Comms_DebugPrint("Set: Display Page Option: ");
        Comms_DebugPrintln(WorkString);
        // Convert binary string tp to integer
        int value = 0;
        for (unsigned int i = 0; i < WorkString.length(); i++) // for every character in the string  strlen(s) returns the length of a char array
        {
          value *= 2; // double the result so far
          if (WorkString[i] == '1')
          {
            value++;  // add 1 if needed
          }
        }
        Comms_DebugPrint("Display Page Option (byte): ");
        Comms_DebugPrintln(value);
        mySetupData->set_oledpageoption((byte) value);
      }
      break;
    case 93: // get OLED page display option
      {
        // return as string of 01's
        char buff[10];
        memset(buff, 0, 10);
        String answer = String( mySetupData->get_oledpageoption(), BIN );
        // assign leading 0's if necessary
        while( answer.length() < 3)
        {
          answer = "0" + answer;
        }
        unsigned int i;
        for ( i = 0; i < answer.length(); i++ )
        {
          buff[i] = answer[i];
        }
        buff[i] = 0;
        Comms_DebugPrint("Get: oled page option: ");
        Comms_DebugPrintln(buff);
        SendPaket('l', buff );
      }
      break;
    case 96: // Set management options
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        int option = WorkString.toInt();
        if ( (option & 1) == 1 )
        {
          // ascom server start if not already started
          if ( mySetupData->get_ascomserverstate() == 0)
          {
#if ( (CONTROLLERMODE == ACCESSPOINT) || (CONTROLLERMODE == STATIONMODE) )
            start_ascomremoteserver();
#endif
          }
        }
        else
        {
          // ascom server stop if running
          if ( mySetupData->get_ascomserverstate() == 1)
          {
#if ( (CONTROLLERMODE == ACCESSPOINT) || (CONTROLLERMODE == STATIONMODE) )
            stop_ascomremoteserver();
#endif
          }
        }
        if ( (option & 2) == 2)
        {
          // in out leds start
          if ( mySetupData->get_inoutledstate() == 0)
          {
            mySetupData->set_inoutledstate(1);
            // reinitialise pins
            if (drvbrd.equals("PRO2ESP32ULN2003") || drvbrd.equals("PRO2ESP32L298N") || drvbrd.equals("PRO2ESP32L293DMIN") || drvbrd.equals("PRO2ESP32L9110S") || drvbrd.equals("PRO2ESP32DRV8825") )
            {
              init_leds();
            }
            else
            {
              Comms_DebugPrintln("Leds not supported on this board type");
            }
          }
          else
          {
            Comms_DebugPrintln("Leds already enabled");
          }
        }
        else
        {
          // in out leds stop
          // if disabled then enable
          if ( mySetupData->get_inoutledstate() == 1)
          {
            mySetupData->set_inoutledstate(0);
          }
        }
        if ( (option & 4) == 4)
        {
          // temp probe start
          if ( mySetupData->get_brdtemppin() == -1)
          {
            Comms_DebugPrintln("Temp pin not set");
            mySetupData->set_temperatureprobestate(0);
          }
          else
          {
            if (mySetupData->get_temperatureprobestate() == 0)          // if temp probe disabled
            {
              mySetupData->set_temperatureprobestate(1);                // then enable probe
              if ( tprobe1 == 0 )                                       // if probe not started
              {
                myTempProbe = new TempProbe();                          // start a new probe
                Comms_DebugPrintln("Probe started");
              }
              else
              {
                Comms_DebugPrintln("Probe already statrted");
              }
            }
            else
            {
              Comms_DebugPrintln("Probe already enabled");
            }
          }
        }
        else
        {
          // temp probe stop
          if (mySetupData->get_temperatureprobestate() == 1)          // if probe currently enabled
          {
            mySetupData->set_temperatureprobestate(0);                // then disable it
            if ( tprobe1 != 0 )                                       // only call this if a probe was found
            {
              myTempProbe->stop_temp_probe();                         // else an exception will occur
            }
          }
        }
        if ( (option & 8) == 8 )
        {
          // set web server option
          if ( mySetupData->get_webserverstate() == 0)
          {
#if ( (CONTROLLERMODE == ACCESSPOINT) || (CONTROLLERMODE == STATIONMODE) )
            start_webserver();
#endif
          }
        }
        else
        {
          if ( mySetupData->get_webserverstate() == 1)
          {
#if ( (CONTROLLERMODE == ACCESSPOINT) || (CONTROLLERMODE == STATIONMODE) )
            stop_webserver();
#endif
          }
        }
      }
      break;
    case 97: // get management option
      {
        int option = 0;
        if ( mySetupData->get_ascomserverstate() == 1)
        {
          option += 1;
        }
        if ( mySetupData->get_inoutledstate() == 1 )
        {
          option += 2;
        }
        if (mySetupData->get_temperatureprobestate() == 1)
        {
          option += 4;
        }
        if ( mySetupData->get_webserverstate() == 1)
        {
          option += 8;
        }
        SendPaket('o', option);
      }
      break;
    case 98:  // get network strength dbm
      {
        long rssi = getrssi();
        SendPaket('s', rssi);
      }
      break;
    case 99:  // set homepositonswitch state, 0 or 1
      {
        int enablestate = 0;
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        enablestate = WorkString.toInt();
        if ( mySetupData->get_brdhpswpin() == -1)
        {
          Comms_DebugPrintln("hpswpin is -1");
        }
        else
        {
          mySetupData->set_hpswitchenable(enablestate);
          if ( enablestate == 1 )
          {
            Comms_DebugPrintln("hpsw state: enabled");
            if ( driverboard->init_homepositionswitch() == true)
            {
              Comms_DebugPrintln("hpsw init OK");
            }
            else
            {
              Comms_DebugPrintln("hpsw init NOK");
            }
          }
          else
          {
            Comms_DebugPrintln("hpsw state: disabled");
          }
        }
      }
      break;

    // new cases v214
    case 58:    // get coilpowertimeout value
      SendPaket('m', mySetupData->get_coilpower_timeout() );
      break;
    case 59:    // set coilpower timeout value (in milliseconds)
      {
        unsigned long cptime = 0;
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        cptime = WorkString.toInt();
        mySetupData->set_coilpower_timeout(cptime);
        Comms_DebugPrint("cp timeout=");
        Comms_DebugPrintln(cptime);
      }
      break;
    case 69:    // send boardconfig.jsn file
      send_boardconfig_file();
      break;

    // now cases for myFP2 compatibility
    case 9:
      break;
    case 41:
      break;
    case 44: // myFP2 set motorspeed threshold when moving - switches to slowspeed when nearing destination
      //ignore
      break;
    case 45: // myFP2 get motorspeedchange threshold value
      SendPaket('G', "200");
      break;
    case 46: // myFP2 enable/Disable motorspeed change when moving
      //ignore
      break;
    case 47: // get motorspeedchange enabled? on/off
      SendPaket('J', "0");
      break;
    case 57: // myFP2 set Super Slow Jogging Speed
      // ignore
      break;
    case 60:  //  myFP2 Set MotorSpeed when jogging
      // ignore
      break;
    case 65:  // myFP2 Set jogging state enable/disable
      // ignore
      break;
    case 66:  // myFP2 Get jogging state enabled/disabled
      SendPaket('K', 0);
      break;
    case 67: // myFP2 Set jogging direction, 0=IN, 1=OUT
      // ignore
      break;
    case 68:  // myFP2 Get jogging direction, 0=IN, 1=OUT
      SendPaket('V', 0);
      break;
    case 94:  // myFP2 Set DelayedDisplayUpdate (0=disabled, 1-enabled)
      break;
    case 95:  // myFP2 Get DelayedDisplayUpdate (0=disabled, 1-enabled)
      SendPaket('n', 0);
      break;
  }
}

void send_boardconfig_file(void)
{
  // send board configuration
  delay(10);
  // Open board_config.jsn file for reading
  File bfile = SPIFFS.open("/board_config.jsn", "r");
  if (!bfile)
  {
    Comms_DebugPrintln("err: no board config file. create defaults.");
    SendPaket('W', "Not found" );
  }
  else
  {
    delay(10);
    // Reading board_config.jsn
    board_data = bfile.readString();                                // read content of the text file
    Comms_DebugPrint("LoadConfiguration(): Board_data= ");
    Comms_DebugPrintln(board_data);                             // ... and print on serial
    bfile.close();
    board_data = board_data + EOFSTR;
    SendPaket('W', "null", true );
  }
}
#endif // #if defined(CONTROLLERMODE)

#if (CONTROLLERMODE == LOCALSERIAL)
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
#endif // #if (CONTROLLERMODE == LOCALSERIAL)

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


#endif
