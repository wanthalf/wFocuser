// ======================================================================
// FocuserSetupData.cpp : myFP2ESP FOCUSER DATA ROUTINES
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

#include <ArduinoJson.h>

#if defined(ESP8266)
#include "FS.h"
#else
#include "SPIFFS.h"
#endif

#include "FocuserSetupData.h"
//#include "generalDefinitions.h"       // should not be needed as FocuserSetupData.h includes this file

// delay(10) required in ESP8266 code around file handling

extern int DefaultBoardNumber;          // this was set to DRVBRD at compile time - used in LoadDefaultBoardData();
extern int brdfixedstepmode;            // set to FIXEDSTEPMODE for boards WEMOSDRV8825H, WEMOSDRV8825, PRO2EDRV8825BIG, PRO2EDRV8825
#define FIXEDSTEPMODE 1

SetupData::SetupData(void)
{
  DebugPrintln("Constructor Setupdata");

  this->SnapShotMillis      = millis();
  this->BoardSnapShotMillis = millis();
  this->ReqSaveData_var     = false;
  this->ReqSaveData_per     = false;
  this->ReqSaveBoard_var    = false;

  if (!SPIFFS.begin())
  {
    DebugPrintln("FS not mounted");
    DebugPrintln("Formatting, please wait...");
    SPIFFS.format();
    DebugPrintln("Format FS done");
  }
  else
  {
    DebugPrintln("FS mounted");
    this->ListDir("/", 0);
  }
  this->LoadConfiguration();
};

// Loads the configuration from a file
byte SetupData::LoadConfiguration()
{
  byte retval = 0;

  // Focuser variable data - Open data_per.jsn file for reading
  File dfile = SPIFFS.open(filename_persistant, "r");
  delay(10);
  if (!dfile)
  {
    DebugPrintln("Err: no Persistant data file. create defaults.");
    LoadDefaultPersistantData();
    delay(10);
  }
  else
  {
    delay(10);
    String fdata = dfile.readString();                 // read content of the text file
    DebugPrint("LoadConfiguration: Persistant SetupData= ");
    DebugPrintln(fdata);                               // ... and print on serial
    dfile.close();

    // Allocate a temporary JsonDocument
    DynamicJsonDocument doc_per(DEFAULTDOCSIZE);

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc_per, fdata);
    if (error)
    {
      DebugPrintln("Err: no persistant data file. create defaults.");
      LoadDefaultPersistantData();
    }
    else
    {
      this->maxstep               = doc_per["maxstep"];                   // max steps
      this->stepsize              = doc_per["stepsize"];                  // the step size in microns, ie 7.2 - value * 10, so real stepsize = stepsize / 10 (maxval = 25.6)
      this->DelayAfterMove        = doc_per["delayaftermove"];            // delay after movement is finished (maxval=256)
      this->backlashsteps_in      = doc_per["backlashsteps_in"];          // number of backlash steps to apply for IN moves
      this->backlashsteps_out     = doc_per["backlashsteps_out"];         // number of backlash steps to apply for OUT moves
      this->backlash_in_enabled   = doc_per["backlash_in_enabled"];
      this->backlash_out_enabled  = doc_per["backlash_out_enabled"];
      this->tempcoefficient       = doc_per["tempcoefficient"];           // steps per degree temperature coefficient value (maxval=256)
      this->tempresolution        = doc_per["tempresolution"];            // 9 -12
      this->coilpower             = doc_per["coilpwr"];
      this->reversedirection      = doc_per["rdirection"];
      this->stepsizeenabled       = doc_per["stepsizestate"];             // if 1, controller returns step size
      this->tempmode              = doc_per["tempmode"];                  // temperature display mode, Celcius=1, Fahrenheit=0
      this->lcdupdateonmove       = doc_per["lcdupdateonmove"];           // update position on lcd when moving
      this->lcdpagetime           = doc_per["lcdpagetime"];
      this->tempcompenabled       = doc_per["tempcompstate"];             // indicates if temperature compensation is enabled
      this->tcdirection           = doc_per["tcdir"];
      this->displayenabled        = doc_per["displaystate"];
      for (int i = 0; i < 10; i++)
      {
        this->preset[i]           = doc_per["preset"][i];
      }
      this->webserverport         = doc_per["wsport"];
      this->ascomalpacaport       = doc_per["ascomport"];
      this->webpagerefreshrate    = doc_per["wprefreshrate"];
      this->mdnsport              = doc_per["mdnsport"];
      this->tcpipport             = doc_per["tcpipport"];
      this->startscreen           = doc_per["startscrn"];
      this->backcolor             = doc_per["bcol"].as<char*>();
      this->textcolor             = doc_per["tcol"].as<char*>();
      this->headercolor           = doc_per["hcol"].as<char*>();
      this->titlecolor            = doc_per["ticol"].as<char*>();
      this->ascomserverstate      = doc_per["ason"];
      this->webserverstate        = doc_per["wson"];
      this->temperatureprobestate = doc_per["tprobe"];
      this->inoutledstate         = doc_per["leds"];
      this->showhpswmessages      = doc_per["hpswmsg"];
      this->forcedownload         = doc_per["fcdownld"];
      this->oledpageoption        = doc_per["oledpg"].as<char*>();
      this->motorspeed            = doc_per["mspeed"];                  // motorspeed slow, med, fast
      this->hpswitchenable        = doc_per["hpswen"];
      this->pbenable              = doc_per["pbenable"];
    }
    DebugPrintln("Config file persistant data loaded");
  }

  // process data_var settings
  delay(10);
  dfile = SPIFFS.open(filename_variable, "r");
  if (!dfile)
  {
    DebugPrintln("Err: no Variable data found. create defaults.");
    LoadDefaultVariableData();
    retval = 1;
  }
  else
  {
    String fdata = dfile.readString();               // read content of the text file
    DebugPrint("LoadConfiguration: Variable SetupData= ");
    DebugPrintln(fdata);                             // ... and print on serial
    dfile.close();

    // Allocate a temporary JsonDocument
    DynamicJsonDocument doc_var(DEFAULTVARDOCSIZE);

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc_var, fdata);
    if (error)
    {
      DebugPrintln("Err: no variable data file. create defaults.");
      LoadDefaultVariableData();
      retval = 2;
    }
    else
    {
      this->fposition = doc_var["fpos"];            // last focuser position
      this->focuserdirection = doc_var["fdir"];     // keeps track of last focuser move direction

      // round position to fullstep motor position
      this->fposition = (this->fposition + this->stepmode / 2) / this->stepmode * this->stepmode;
      retval = 3;
    }
  }
  dfile.close();
  DebugPrintln("variable data loaded");

  // Process board configuration
  delay(10);
  // Open board_config.jsn file for reading
  DebugPrint("Default board config file:");
  DebugPrintln(filename_boardconfig);
  File bfile = SPIFFS.open(filename_boardconfig, "r");
  if (!bfile)
  {
    DebugPrintln("err: no board config file. create defaults.");
    LoadDefaultBoardData();
    delay(10);
    retval = 4;
  }
  else
  {
    delay(10);
    // Reading board_config.jsn
    String board_data = bfile.readString();               // read content of the text file
    DebugPrint("LoadConfiguration(): Board_data= ");
    DebugPrintln(board_data);                             // ... and print on serial
    bfile.close();

    // Allocate a temporary JsonDocument
    DynamicJsonDocument doc_brd(DEFAULTBOARDSIZE);

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc_brd, board_data);
    if (error)
    {
      DebugPrintln("Err: deserialize board config file, create defaults.");
      LoadDefaultBoardData();
    }
    else
    {
      /*
        { "board":"PRO2ESP32DRV8825","maxstepmode":32,"stepmode":1,"sda":21,"sck":22,"enpin":14,"steppin":33,
        "dirpin":32,"temppin":13,"hpswpin":4,"inledpin":18,"outledpin":19,"pb1pin":34,"pb2pin":35,"irpin":15,
        "stepsrev":-1,"fixedsmode":-1,"brdpins":[27,26,25,-1],"msdelay":4000 }
      */
      this->board         = doc_brd["board"].as<char*>();
      this->maxstepmode   = doc_brd["maxstepmode"];
      this->stepmode      = doc_brd["stepmode"];
      this->sda           = doc_brd["sda"];
      this->sck           = doc_brd["sck"];
      this->enablepin     = doc_brd["enpin"];
      this->steppin       = doc_brd["steppin"];
      this->dirpin        = doc_brd["dirpin"];
      this->temppin       = doc_brd["temppin"];
      this->hpswpin       = doc_brd["hpswpin"];
      this->inledpin      = doc_brd["inledpin"];
      this->outledpin     = doc_brd["outledpin"];
      this->pb1pin        = doc_brd["pb1pin"];
      this->pb2pin        = doc_brd["pb2pin"];
      this->irpin         = doc_brd["irpin"];
      this->stepsperrev   = doc_brd["stepsrev"];
      this->fixedstepmode = doc_brd["fixedsmode"];
      for (int i = 0; i < 4; i++)
      {
        this->boardpins[i] = doc_brd["brdpins"][i];
      }
      this->msdelay        = doc_brd["msdelay"];                    // motor speed delay - do not confuse with motorspeed

      DebugPrintln("Board configuration file loaded");
    }
    retval = 5;
  }
  return retval;
}

// called froms comms.h case 42: reset focuser to defaults
void SetupData::SetFocuserDefaults(void)
{
  LoadDefaultPersistantData();
  LoadDefaultVariableData();
  LoadDefaultBoardData();
  delay(10);
  if ( SPIFFS.exists(filename_persistant))
  {
    SPIFFS.remove(filename_persistant);
  }
  delay(10);
  if ( SPIFFS.exists(filename_variable))
  {
    SPIFFS.remove(filename_variable);
  }
  delay(10);
  if ( SPIFFS.exists(filename_boardconfig))
  {
    SPIFFS.remove(filename_boardconfig);
  }
}

// Saves Board configuration
boolean SetupData::SaveBoardConfiguration()
{
  byte cstatus = false;
  unsigned long x = millis();
  if ((BoardSnapShotMillis + DEFAULTSAVETIME) < x || BoardSnapShotMillis > x)    // 30s after snapshot
  {
    if (this->ReqSaveBoard_var == true)
    {
      if (this->WriteBoardConfiguration() == false)
      {
        DebugPrintln("Error save driver board configuration");
      }
      else
      {
        delay(10);
        DebugPrintln("++ driver board data saved");
      }
      cstatus = true;
      this->ReqSaveBoard_var = false;
    }
  }
  return cstatus;
}

// Saves the configuration to a file
boolean SetupData::SaveConfiguration(unsigned long currentPosition, byte DirOfTravel)
{
  //DebugPrintln("SaveConfiguration:");
  if (this->fposition != currentPosition || this->focuserdirection != DirOfTravel)  // last focuser position
  {
    this->fposition = currentPosition;
    this->focuserdirection = DirOfTravel;
    this->ReqSaveData_var = true;
    this->SnapShotMillis = millis();
    DebugPrintln("++ request for saving variable data");
    delay(10);
  }

  byte cstatus = false;
  unsigned long x = millis();

  if ((SnapShotMillis + DEFAULTSAVETIME) < x || SnapShotMillis > x)    // 30s after snapshot
  {
    if (this->ReqSaveData_per == true)
    {
      if (SavePersitantConfiguration() == false)
      {
        DebugPrintln("Error save persistant configuration");
      }
      else
      {
        delay(10);
        DebugPrintln("++ persistant data saved");
      }
      cstatus = true;
      this->ReqSaveData_per = false;
    }

    if (this->ReqSaveData_var == true)
    {
      if (SaveVariableConfiguration() == false)
      {
        DebugPrintln("Error save variable configuration");
      }
      else
      {
        delay(10);
        DebugPrintln("++ variable data saved");
      }
      cstatus = true;
      this->ReqSaveData_var = false;
    }
  }
  return cstatus;
}

boolean SetupData::SaveNow()                          // used by reboot to save settings
{
  return SavePersitantConfiguration();
}

boolean SetupData::SaveBoardConfigNow(void)
{
  return WriteBoardConfiguration();
}

// ======================================================================
// Focuser Variable Data - Position and Direction
// ======================================================================
void SetupData::LoadDefaultVariableData()
{
  this->fposition = DEFAULTPOSITION;                  // last focuser position
  this->focuserdirection = moving_in;                 // keeps track of last focuser move direction
}

byte SetupData::SaveVariableConfiguration()
{
  // Delete existing file
  if ( SPIFFS.exists(filename_variable))
  {
    SPIFFS.remove(filename_variable);
  }
  delay(10);
  SPIFFS.remove(filename_variable);

  delay(10);
  // Open file for writing
  File file = SPIFFS.open(this->filename_variable, "w");
  if (!file)
  {
    TRACE();
    DebugPrintln(CREATEFILEFAILSTR);
    return false;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<DEFAULTVARDOCSIZE> doc;

  // Set the values in the document
  doc["fpos"] =  this->fposition;                  // last focuser position
  doc["fdir"] =  this->focuserdirection;           // keeps track of last focuser move direction

  if (serializeJson(doc, file) == 0)                // Serialize JSON to file
  {
    TRACE();
    DebugPrintln(WRITEFILEFAILSTR);
    file.close();                                   // Close the file
    return false;
  }
  else
  {
    DebugPrintln(WRITEFILESUCCESSSTR);
    file.close();     // Close the file
    return true;
  }
}

// ======================================================================
// Focuser Persistent Data
// ======================================================================
void SetupData::LoadDefaultPersistantData()
{
  this->maxstep               = DEFAULTMAXSTEPS;
  this->coilpower             = DEFAULTOFF;
  this->reversedirection      = DEFAULTOFF;
  this->stepsizeenabled       = DEFAULTOFF;
  this->stepsize              = DEFAULTSTEPSIZE;
  this->DelayAfterMove        = DEFAULTOFF;
  this->backlashsteps_in      = DEFAULTOFF;
  this->backlashsteps_out     = DEFAULTOFF;
  this->backlash_in_enabled   = DEFAULTON;
  this->backlash_out_enabled  = DEFAULTON;
  this->tempcoefficient       = DEFAULTOFF;
  this->tempresolution        = TEMPRESOLUTION;       // 0.25 degrees
  this->tcdirection           = DEFAULTOFF;           // temperature compensation direction 1
  this->tempmode              = DEFAULTCELSIUS;       // default is celsius
  this->tempcompenabled       = DEFAULTOFF;           // temperature compensation disabled
  this->lcdupdateonmove       = DEFAULTON;
  this->lcdpagetime           = LCDPAGETIMEMIN;       // 2, 3 -- 10
  this->motorspeed            = FAST;
  this->displayenabled        = DEFAULTON;
  for (int i = 0; i < 10; i++)
  {
    this->preset[i]           = 0;
  }
  this->webserverport         = WEBSERVERPORT;        // 80
  this->ascomalpacaport       = ALPACAPORT;           // 4040
  this->webpagerefreshrate    = WS_REFRESHRATE;       // 30s
  this->mdnsport              = MDNSSERVERPORT;       // 7070
  this->tcpipport             = SERVERPORT;           // 2020
  this->startscreen           = DEFAULTON;
  this->backcolor             = "333333";
  this->textcolor             = "5d6d7e";
  this->headercolor           = "3399ff";
  this->titlecolor            = "8e44ad";
  this->ascomserverstate      = DEFAULTOFF;           // this should be default OFF
  this->webserverstate        = DEFAULTON;            // this should be default OFF
  this->temperatureprobestate = DEFAULTON;            // this should be default OFF - if HW not fitted could crash
  this->inoutledstate         = DEFAULTOFF;           // this should be default OFF - if HW not fitted could crash
  this->showhpswmessages      = DEFAULTOFF;           // this should be default OFF
  this->forcedownload         = DEFAULTOFF;           // this should be default OFF, MANAGEMENT Server only
  this->oledpageoption        = OLEDPGOPTIONALL;
  this->hpswitchenable        = DEFAULTOFF;
  this->pbenable              = DEFAULTOFF;

  this->SavePersitantConfiguration();                 // write default values to SPIFFS
}

byte SetupData::SavePersitantConfiguration()
{
  delay(10);
  if ( SPIFFS.exists(filename_persistant))
  {
    SPIFFS.remove(filename_persistant);
  }
  delay(10);
  File file = SPIFFS.open(filename_persistant, "w");         // Open file for writing
  if (!file)
  {
    TRACE();
    DebugPrintln(CREATEFILEFAILSTR);
    return false;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<DEFAULTDOCSIZE> doc;

  // Set the values in the document
  doc["maxstep"]            = this->maxstep;                    // max steps
  doc["stepsize"]           = this->stepsize;                   // the step size in microns, ie 7.2 - value * 10, so real stepsize = stepsize / 10 (maxval = 25.6)
  doc["delayaftermove"]     = this->DelayAfterMove;             // delay after movement is finished (maxval=256)
  doc["backlashsteps_in"]   = this->backlashsteps_in;           // number of backlash steps to apply for IN moves
  doc["backlashsteps_out"]  = this->backlashsteps_out;          // number of backlash steps to apply for OUT moves
  doc["backlash_in_enabled"] = this->backlash_in_enabled;
  doc["backlash_out_enabled"] = this->backlash_out_enabled;
  doc["tempcoefficient"]    = this->tempcoefficient;            // steps per degree temperature coefficient value (maxval=256)
  doc["tempresolution"]     = this->tempresolution;
  doc["coilpwr"]            = this->coilpower;
  doc["rdirection"]         = this->reversedirection;
  doc["stepsizestate"]      = this->stepsizeenabled;            // if 1, controller returns step size
  doc["tempmode"]           = this->tempmode;                   // temperature display mode, Celcius=1, Fahrenheit=0
  doc["lcdupdateonmove"]    = this->lcdupdateonmove;            // update position on lcd when moving
  doc["lcdpagetime"]        = this->lcdpagetime;                // *100 to give interval between lcd pages display time
  doc["tempcompstate"]      = this->tempcompenabled;            // indicates if temperature compensation is enabled
  doc["tcdir"]              = this->tcdirection;
  doc["motorspeed"]         = this->motorspeed;
  doc["displaystate"]       = this->displayenabled;
  for (int i = 0; i < 10; i++)
  {
    doc["preset"][i]        = this->preset[i];                  // Json array for presets
  };
  doc["wsport"]             = this->webserverport;
  doc["ascomport"]          = this->ascomalpacaport;
  doc["mdnsport"]           = this->mdnsport;
  doc["wprefreshrate"]      = this->webpagerefreshrate;
  doc["tcpipport"]          = this->tcpipport;
  doc["startscrn"]          = this->startscreen;
  doc["bcol"]               = this->backcolor;
  doc["tcol"]               = this->textcolor;
  doc["hcol"]               = this->headercolor;
  doc["ticol"]              = this->titlecolor;
  doc["ason"]               = this->ascomserverstate;
  doc["wson"]               = this->webserverstate;
  doc["tprobe"]             = this->temperatureprobestate;
  doc["leds"]               = this->inoutledstate;
  doc["hpswmsg"]            = this->showhpswmessages;
  doc["fcdownld"]           = this->forcedownload;
  doc["oledpg"]             = this->oledpageoption;
  doc["mspeed"]             = this->motorspeed;
  doc["hpswen"]             = this->hpswitchenable;
  doc["pbenable"]           = this->pbenable;

  // Serialize JSON to file
  DebugPrintln("Writing to file");
  if (serializeJson(doc, file) == 0)
  {
    TRACE();
    DebugPrintln(WRITEFILEFAILSTR);
    file.close();                                     // Close the file
    return false;
  }
  else
  {
    DebugPrintln(WRITEFILESUCCESSSTR);
    file.close();                                     // Close the file
    return true;
  }
}

//__getter
unsigned long SetupData::get_fposition()
{
  return this->fposition;             // last focuser position
}

byte SetupData::get_focuserdirection()
{
  return this->focuserdirection;      // keeps track of last focuser move direction
}

unsigned long SetupData::get_maxstep()
{
  return this->maxstep;               // max steps
}

float SetupData::get_stepsize()
{
  return this->stepsize;              // the step size in microns
  // this is the actual measured focuser stepsize in microns amd is reported to ASCOM, so must be valid
  // the amount in microns that the focuser tube moves in one step of the motor
}

byte SetupData::get_DelayAfterMove()
{
  return this->DelayAfterMove;        // delay after movement is finished (maxval=256)
}

byte SetupData::get_backlashsteps_in()
{
  return this->backlashsteps_in;      // number of backlash steps to apply for IN moves
}

byte SetupData::get_backlashsteps_out()
{
  return this->backlashsteps_out;     // number of backlash steps to apply for OUT moves
}

byte SetupData::get_backlash_in_enabled()
{
  return this->backlash_in_enabled;   // apply backlash when moving in [0=!enabled, 1=enabled]
}

byte SetupData::get_backlash_out_enabled()
{
  return this->backlash_out_enabled;  // apply backlash when moving out [0=!enabled, 1=enabled]
}

byte SetupData::get_tempcoefficient()
{
  return this->tempcoefficient;       // steps per degree temperature coefficient value (maxval=256)
}

byte SetupData::get_tempresolution()
{
  return this->tempresolution;        // resolution of temperature measurement 9-12
}

byte SetupData::get_coilpower()
{
  return this->coilpower;             // state of coil power, 0 = !enabled, 1= enabled
}

byte SetupData::get_reversedirection()
{
  return this->reversedirection;      // state for reverse direction, 0 = !enabled, 1= enabled
}

byte SetupData::get_stepsizeenabled()
{
  return this->stepsizeenabled;       // if 1, controller returns step size
}

byte SetupData::get_tempmode()
{
  return this->tempmode;              // temperature display mode, Celcius=1, Fahrenheit=0
}

byte SetupData::get_lcdupdateonmove()
{
  return this->lcdupdateonmove;       // update position on lcd when moving
}

byte SetupData::get_lcdpagetime()
{
  return this->lcdpagetime;           // the length of time the page is displayed for
}

byte SetupData::get_tempcompenabled()
{
  return this->tempcompenabled;       // indicates if temperature compensation is enabled
}

byte SetupData::get_tcdirection()
{
  return this->tcdirection;           // indicates the direction in which temperature compensation is applied
}

byte SetupData::get_motorspeed()
{
  return this->motorspeed;            // the stepper motor speed, slow, medium, fast
}

byte SetupData::get_displayenabled()
{
  return this->displayenabled;        // the state of the oled display, enabled or !enabled
}

unsigned long SetupData::get_focuserpreset(byte idx)
{
  return this->preset[idx % 10];      // the focuser position for each preset
}

unsigned long SetupData::get_webserverport(void)
{
  return this->webserverport;         // the port number of the webserver
}

unsigned long SetupData::get_ascomalpacaport(void)
{
  return this->ascomalpacaport;       // the port number used by the ALPACA ASCOM Remote server
}

int SetupData::get_webpagerefreshrate(void)
{
  return this->webpagerefreshrate;    // the webpage refresh rate
}

unsigned long SetupData::get_mdnsport(void)
{
  return this->mdnsport;              // the mdns port number
}

unsigned long SetupData::get_tcpipport(void)
{
  return this->tcpipport;             // the tcp/ip port used by the tcp/server
}

byte SetupData::get_showstartscreen(void)
{
  return this->startscreen;           //  the state of startscreen, enabled or !enabled
  // if enabled, show startup messages on the TEXT OLED display
}

String SetupData::get_wp_backcolor()
{
  return this->backcolor;
}

String SetupData::get_wp_textcolor()
{
  return this->textcolor;
}

String SetupData::get_wp_headercolor()
{
  return this->headercolor;
}

String SetupData::get_wp_titlecolor()
{
  return this->titlecolor;
}

byte SetupData::get_ascomserverstate()
{
  return this->ascomserverstate;
}

byte SetupData::get_webserverstate()
{
  return this->webserverstate;
}

byte SetupData::get_temperatureprobestate()
{
  return this->temperatureprobestate;
}

byte SetupData::get_inoutledstate()
{
  return this->inoutledstate;
}

byte SetupData::get_showhpswmsg()
{
  return this->showhpswmessages;
}

byte SetupData::get_forcedownload()
{
  return this->forcedownload;
}

String SetupData::get_oledpageoption()
{
  return this->oledpageoption;
}

byte SetupData::get_hpswitchenable()
{
  return this->hpswitchenable;
}

byte SetupData::get_pbenable()
{
  return this->pbenable;
}

//__Setter

void SetupData::set_fposition(unsigned long fposition)
{
  this->fposition = fposition; // last focuser position
}

void SetupData::set_focuserdirection(byte focuserdirection)
{
  this->focuserdirection = focuserdirection; // keeps track of last focuser move direction
}

void SetupData::set_maxstep(unsigned long maxstep)
{
  this->StartDelayedUpdate(this->maxstep, maxstep); // max steps
}

void SetupData::set_stepsize(float stepsize)
{
  this->StartDelayedUpdate(this->stepsize, stepsize); // the step size in microns, ie 7.2 - value * 10, so real stepsize = stepsize / 10 (maxval = 25.6)
}

void SetupData::set_DelayAfterMove(byte DelayAfterMove)
{
  this->StartDelayedUpdate(this->DelayAfterMove, DelayAfterMove); // delay after movement is finished (maxval=256)
}

void SetupData::set_backlashsteps_in(byte backlashsteps)
{
  this->StartDelayedUpdate(this->backlashsteps_in, backlashsteps); // number of backlash steps to apply for IN moves
}

void SetupData::set_backlashsteps_out(byte backlashsteps_out)
{
  this->StartDelayedUpdate(this->backlashsteps_out, backlashsteps_out); // number of backlash steps to apply for OUT moves
}

void SetupData::set_backlash_in_enabled(byte backlash_in_enabled)
{
  this->StartDelayedUpdate(this->backlash_in_enabled, backlash_in_enabled);
}

void SetupData::set_backlash_out_enabled(byte backlash_out_enabled)
{
  this->StartDelayedUpdate(this->backlash_out_enabled, backlash_out_enabled);
}

void SetupData::set_tempcoefficient(byte tempcoefficient)
{
  this->StartDelayedUpdate(this->tempcoefficient, tempcoefficient); // steps per degree temperature coefficient value (maxval=256)
}

void SetupData::set_tempresolution(byte tempresolution)
{
  this->StartDelayedUpdate(this->tempresolution, tempresolution);
}

void SetupData::set_coilpower(byte coilpower)
{
  this->StartDelayedUpdate(this->coilpower, coilpower);
}

void SetupData::set_reversedirection(byte reversedirection)
{
  this->StartDelayedUpdate(this->reversedirection, reversedirection);
}

void SetupData::set_stepsizeenabled(byte stepsizeenabled)
{
  this->StartDelayedUpdate(this->stepsizeenabled, stepsizeenabled); // if 1, controller returns step size
}

void SetupData::set_tempmode(byte tempmode)
{
  this->StartDelayedUpdate(this->tempmode, tempmode); // temperature display mode, Celcius=1, Fahrenheit=0
}

void SetupData::set_lcdupdateonmove(byte lcdupdateonmove)
{
  this->StartDelayedUpdate(this->lcdupdateonmove, lcdupdateonmove); // update position on lcd when moving
}

void SetupData::set_lcdpagetime(byte lcdpagetime)
{
  this->StartDelayedUpdate(this->lcdpagetime, lcdpagetime);
}

void SetupData::set_tempcompenabled(byte tempcompenabled)
{
  this->StartDelayedUpdate(this->tempcompenabled, tempcompenabled); // indicates if temperature compensation is enabled
}

void SetupData::set_tcdirection(byte tcdirection)
{
  this->StartDelayedUpdate(this->tcdirection, tcdirection);
}

void SetupData::set_motorspeed(byte motorspeed)
{
  this->StartDelayedUpdate(this->motorspeed, motorspeed);
}

void SetupData::set_displayenabled(byte displaystate)
{
  this->StartDelayedUpdate(this->displayenabled, displaystate);
}

void SetupData::set_focuserpreset(byte idx, unsigned long pos)
{
  this->StartDelayedUpdate(this->preset[idx % 10], pos);
}

void SetupData::set_webserverport(unsigned long wsp)
{
  this->StartDelayedUpdate(this->webserverport, wsp);
}

void SetupData::set_ascomalpacaport(unsigned long asp)
{
  this->StartDelayedUpdate(this->ascomalpacaport, asp);
}

void SetupData::set_webpagerefreshrate(int rr)
{
  this->StartDelayedUpdate(this->webpagerefreshrate, rr);
}

void SetupData::set_mdnsport(unsigned long port)
{
  this->StartDelayedUpdate(this->mdnsport, port);
}

void SetupData::set_tcpipport(unsigned long port)
{
  this->StartDelayedUpdate(this->tcpipport, port);
}

void SetupData::set_showstartscreen(byte newval)
{
  this->StartDelayedUpdate(this->startscreen, newval);
}

void SetupData::set_wp_backcolor(String newstr)
{
  this->StartDelayedUpdate(this->backcolor, newstr);
}

void SetupData::set_wp_textcolor(String newstr)
{
  this->StartDelayedUpdate(this->textcolor, newstr);
}

void SetupData::set_wp_headercolor(String newstr)
{
  this->StartDelayedUpdate(this->headercolor, newstr);
}

void SetupData::set_wp_titlecolor(String newstr)
{
  this->StartDelayedUpdate(this->titlecolor, newstr);
}

void SetupData::set_ascomserverstate(byte newval)
{
  this->StartDelayedUpdate(this->ascomserverstate, newval);
}

void SetupData::set_webserverstate(byte newval)
{
  this->StartDelayedUpdate(this->webserverstate, newval);
}

void SetupData::set_temperatureprobestate(byte newval)
{
  this->StartDelayedUpdate(this->temperatureprobestate, newval);
}

void SetupData::set_inoutledstate(byte newval)
{
  this->StartDelayedUpdate(this->inoutledstate, newval);
}

void SetupData::set_showhpswmsg(byte newval)
{
  this->StartDelayedUpdate(this->showhpswmessages, newval);
}

void SetupData::set_forcedownload(byte newval)
{
  this->StartDelayedUpdate(this->forcedownload, newval);
}

void SetupData::set_oledpageoption(String newval)
{
  this->StartDelayedUpdate(this->oledpageoption, newval);
}

void SetupData::set_hpswitchenable(byte newval)
{
  this->StartDelayedUpdate(this->hpswitchenable, newval);
}

void SetupData::set_pbenable(byte newval)
{
  this->StartDelayedUpdate(this->pbenable, newval);
}

void SetupData::StartDelayedUpdate(int & org_data, int new_data)
{
  if (org_data != new_data)
  {
    this->ReqSaveData_per = true;
    this->SnapShotMillis = millis();
    org_data = new_data;
    DebugPrintln("++ request for saving persitant data");
  }
}

void SetupData::StartDelayedUpdate(unsigned long & org_data, unsigned long new_data)
{
  if (org_data != new_data)
  {
    this->ReqSaveData_per = true;
    this->SnapShotMillis = millis();
    org_data = new_data;
    DebugPrintln("++ request for saving persitant data");
  }
}

void SetupData::StartDelayedUpdate(float & org_data, float new_data)
{
  if (org_data != new_data)
  {
    this->ReqSaveData_per = true;
    this->SnapShotMillis = millis();
    org_data = new_data;
    DebugPrintln("++ request for saving persitant data");
  }
}

void SetupData::StartDelayedUpdate(byte & org_data, byte new_data)
{
  if (org_data != new_data)
  {
    this->ReqSaveData_per = true;
    this->SnapShotMillis = millis();
    org_data = new_data;
    DebugPrintln("++ request for saving persitant data");
  }
}

void SetupData::StartDelayedUpdate(String & org_data, String new_data)
{
  if (org_data != new_data)
  {
    this->ReqSaveData_per = true;
    this->SnapShotMillis = millis();
    org_data = new_data;
    DebugPrintln("Save request for data_per.jsn");
  }
}

// ======================================================================
// Focuser Persistent Data for Driver Board
// ======================================================================

boolean SetupData:: LoadBrdConfigStart(String brdfile)
{
  delay(10);
  File bfile = SPIFFS.open(brdfile, "r");               // Open file for writing
  DebugPrint("LoadBrdConfigStart: ");
  DebugPrintln(brdfile);
  if (!bfile)
  {
    TRACE();
    DebugPrintln("File not found");
    return false;
  }
  else
  {
    // read file and deserialize
    String fdata = bfile.readString();                  // read content of the text file
    DebugPrint("LoadBrdConfigStart: Data= ");
    DebugPrintln(fdata);                              // ... and print on serial
    bfile.close();

    // Allocate a temporary JsonDocument
    DynamicJsonDocument doc_brd(DEFAULTBOARDSIZE);

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc_brd, fdata);
    if (error)
    {
      DebugPrintln("Failed to read board config file");
      return false;
    }
    else
    {
      // save the brd_data just read from board config file (brdfile) into board_config.jsn
      // Set the board values from doc_brd
      this->board         = doc_brd["board"].as<char*>();
      this->maxstepmode   = doc_brd["maxstepmode"];
      this->stepmode      = doc_brd["stepmode"];
      this->sda           = doc_brd["sda"];
      this->sck           = doc_brd["sck"];
      this->enablepin     = doc_brd["enpin"];
      this->steppin       = doc_brd["steppin"];
      this->dirpin        = doc_brd["dirpin"];
      this->temppin       = doc_brd["temppin"];
      this->hpswpin       = doc_brd["hpswpin"];
      this->inledpin      = doc_brd["inledpin"];
      this->outledpin     = doc_brd["outledpin"];
      this->pb1pin        = doc_brd["pb1pin"];
      this->pb2pin        = doc_brd["pb2pin"];
      this->irpin         = doc_brd["irpin"];
      this->stepsperrev   = doc_brd["stepsrev"];
      switch ( DefaultBoardNumber )                                 // apply fixedstepmode to specific boards
      {
        case WEMOSDRV8825H:
        case WEMOSDRV8825:
        case PRO2EDRV8825BIG:
        case PRO2EDRV8825:
          this->fixedstepmode = brdfixedstepmode;                   // set fixedstepmode from focuserconfig.h FIXEDSTEPMODE
          break;
        default:
          // ignore                                                 // do not apply to boards which do not use this value
          break;
      }
      for (int i = 0; i < 4; i++)
      {
        this->boardpins[i] = doc_brd["brdpins"][i];
      }
      this->msdelay        = doc_brd["msdelay"];                    // motor speed delay - do not confuse with motorspeed
      WriteBoardConfiguration();
      return true;
    }
  }
  return false;
}

void SetupData::LoadDefaultBoardData()
{
  // we are here because board_config.jsn not found
  // we can load the default board configuration from DRVBRD defined - DefaultBoardName in .ino file
  // Focuser driver board data - Open specific board config .jsn file for reading

  String brdfile = "/boards/" + String(DefaultBoardNumber) + ".jsn";
  DebugPrint("brdfile: " );
  DebugPrintln(brdfile);

  if ( LoadBrdConfigStart(brdfile) == true )
  {
    DebugPrintln("LoadDefaultBoardData(): Loaded default DRV board: OK");
  }
  else
  {
    DebugPrintln("LoadDefaultBoardData(): Loaded default DRV board: Fail");
    DebugPrintln("LoadDefaultBoardData(): Create Unknown Board Config File");
    this->board         = "Unknown";
    this->maxstepmode   = -1;
    this->stepmode      =  1;              // full step
    this->sda           = -1;
    this->sck           = -1;
    this->enablepin     = -1;
    this->steppin       = -1;
    this->dirpin        = -1;
    this->temppin       = -1;
    this->hpswpin       = -1;
    this->inledpin      = -1;
    this->outledpin     = -1;
    this->pb1pin        = -1;
    this->pb1pin        = -1;
    this->irpin         = -1;
    this->stepsperrev   = -1;
      switch ( DefaultBoardNumber )
      {
        case WEMOSDRV8825H:
        case WEMOSDRV8825:
        case PRO2EDRV8825BIG:
        case PRO2EDRV8825:
          this->fixedstepmode = brdfixedstepmode;                   // set fixedstepmode from focuserconfig.h FIXEDSTEPMODE
          break;
        default:
          this->fixedstepmode = -1;
          break;
      }
    for (int i = 0; i < 4; i++)
    {
      this->boardpins[i] = -1;
    }
    this->msdelay        = 8000;
  }
  WriteBoardConfiguration();
}

boolean SetupData::WriteBoardConfiguration()
{
  delay(10);
  if ( SPIFFS.exists(filename_boardconfig))
  {
    SPIFFS.remove(filename_boardconfig);
  }
  delay(10);
  File bfile = SPIFFS.open(filename_boardconfig, "w");         // Open file for writing
  if (!bfile)
  {
    TRACE();
    DebugPrintln(CREATEFILEFAILSTR);
    return false;
  }
  else
  {
    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/assistant to compute the capacity.
    StaticJsonDocument<DEFAULTBOARDSIZE> doc_brd;

    // Set the values in the document
    doc_brd["board"]        = this->board;
    doc_brd["maxstepmode"]  = this->maxstepmode;
    doc_brd["stepmode"]     = this->stepmode;
    doc_brd["sda"]          = this->sda;
    doc_brd["sck"]          = this->sck;
    doc_brd["enpin"]        = this->enablepin;
    doc_brd["steppin"]      = this->steppin;
    doc_brd["dirpin"]       = this->dirpin;
    doc_brd["temppin"]      = this->temppin;
    doc_brd["hpswpin"]      = this->hpswpin;
    doc_brd["inledpin"]     = this->inledpin;
    doc_brd["outledpin"]    = this->outledpin;
    doc_brd["pb1pin"]       = this->pb1pin;
    doc_brd["pb2pin"]       = this->pb2pin;
    doc_brd["irpin"]        = this->irpin;
    doc_brd["stepsrev"]     = this->stepsperrev;
    doc_brd["fixedsmode"]   = this->fixedstepmode;
    for (int i = 0; i < 4; i++)
    {
      doc_brd["brdpins"][i] = this->boardpins[i];
    }
    doc_brd["msdelay"]      = this->msdelay;

    // Serialize JSON to file
    DebugPrintln("Writing to board file");
    if (serializeJson(doc_brd, bfile) == 0)
    {
      TRACE();
      DebugPrintln(WRITEFILEFAILSTR);
      bfile.close();                                     // Close the file
      return false;
    }
    else
    {
      DebugPrintln(WRITEFILESUCCESSSTR);
      bfile.close();                                     // Close the file
    }
  }
  return true;
}

boolean SetupData::CreateBoardConfigfromjson(String jsonstr)
{
  // generate board configuration from json string
  delay(10);

  DebugPrint("CreateBoardConfigfromjson. Board_data= ");
  DebugPrintln(jsonstr);                             // ... and print on serial
  // Allocate a temporary Json Document
  DynamicJsonDocument doc_brd(DEFAULTBOARDSIZE + 100);

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc_brd, jsonstr);
  if (error)
  {
    DebugPrintln("Deserializae board : Fail");
    LoadDefaultBoardData();
    return false;
  }
  else
  {
    /*
      { "board":"PRO2ESP32DRV8825","maxstepmode":32,"stepmode":1,"sda":21,"sck":22,"enpin":14,"steppin":33,
      "dirpin":32,"temppin":13,"hpswpin":4,"inledpin":18,"outledpin":19,"pb1pin":34,"pb2pin":35,"irpin":15,
      "stepsrev":-1,"fixedsmode":-1,"brdpins":[27,26,25,-1],"msdelay":4000 }
    */
    DebugPrintln("Deserializa board : OK");
    this->board         = doc_brd["board"].as<char*>();
    this->maxstepmode   = doc_brd["maxstepmode"];
    this->stepmode      = doc_brd["stepmode"];
    this->sda           = doc_brd["sda"];
    this->sck           = doc_brd["sck"];
    this->enablepin     = doc_brd["enpin"];
    this->steppin       = doc_brd["steppin"];
    this->dirpin        = doc_brd["dirpin"];
    this->temppin       = doc_brd["temppin"];
    this->hpswpin       = doc_brd["hpswpin"];
    this->inledpin      = doc_brd["inledpin"];
    this->outledpin     = doc_brd["outledpin"];
    this->pb1pin        = doc_brd["pb1pin"];
    this->pb2pin        = doc_brd["pb2pin"];
    this->irpin         = doc_brd["irpin"];
    this->stepsperrev   = doc_brd["stepsrev"];
    this->fixedstepmode = doc_brd["fixedsmode"];
    for (int i = 0; i < 4; i++)
    {
      this->boardpins[i] = doc_brd["brdpins"][i];
    }
    this->msdelay        = doc_brd["msdelay"];                    // motor speed delay - do not confuse with motorspeed

    DebugPrintln("Board configuration file loaded");

    SaveBoardConfigNow();
    DebugPrintln("board_config.jsn created");
    return true;
  }
}

// getters
String SetupData::get_brdname()
{
  return this->board;
}

int SetupData::get_brdmaxstepmode()
{
  return this->maxstepmode;
}

int SetupData::get_brdstepmode()
{
  return this->stepmode;
}

int SetupData::get_brdsda()
{
  return this->sda;
}

int SetupData::get_brdsck()
{
  return this->sck;
}

int SetupData::get_brdenablepin()
{
  return this->enablepin;
}

int SetupData::get_brdsteppin()
{
  return this->steppin;
}

int SetupData::get_brddirpin()
{
  return this->dirpin;
}

int SetupData::get_brdtemppin()
{
  return this->temppin;
}

int SetupData::get_brdhpswpin()
{
  return this->hpswpin;
}

int SetupData::get_brdinledpin()
{
  return this->inledpin;
}

int SetupData::get_brdoutledpin()
{
  return this->outledpin;
}

int SetupData::get_brdirpin()
{
  return this->irpin;
}

int SetupData::get_brdboardpins(int pinnum)
{
  return this->boardpins[pinnum];
}

int SetupData::get_brdstepsperrev()
{
  return this->stepsperrev;
}

int SetupData::get_brdfixedstepmode()
{
  return this->fixedstepmode;
}

int SetupData::get_brdpb1pin()
{
  return this->pb1pin;
}

int SetupData::get_brdpb2pin()
{
  return this->pb2pin;
}

unsigned long SetupData::get_brdmsdelay()
{
  return this->msdelay;
}

// setter
void SetupData::set_brdname(String newstr)
{
  this->StartBoardDelayedUpdate(this->board, newstr);
}

void SetupData::set_brdmaxstepmode(int newval)
{
  this->StartBoardDelayedUpdate(this->maxstepmode, newval);
}

void SetupData::set_brdstepmode(int newval)
{
  this->StartBoardDelayedUpdate(this->stepmode, newval);
}

void SetupData::set_brdsda(int pinnum)
{
  this->StartBoardDelayedUpdate(this->sda, pinnum);
}

void SetupData::set_brdsck(int pinnum)
{
  this->StartBoardDelayedUpdate(this->sck, pinnum);
}

void SetupData::set_brdenablepin(int pinnum)
{
  this->StartBoardDelayedUpdate(this->enablepin, pinnum);
}

void SetupData::set_brdsteppin(int pinnum)
{
  this->StartBoardDelayedUpdate(this->steppin, pinnum);
}

void SetupData::set_brddirpin(int pinnum)
{
  this->StartBoardDelayedUpdate(this->dirpin, pinnum);
}

void SetupData::set_brdtemppin(int pinnum)
{
  this->StartBoardDelayedUpdate(this->temppin, pinnum);
}

void SetupData::set_brdhpswpin(int pinnum)
{
  this->StartBoardDelayedUpdate(this->hpswpin, pinnum);
}

void SetupData::set_brdinledpin(int pinnum)
{
  this->StartBoardDelayedUpdate(this->inledpin, pinnum);
}

void SetupData::set_brdoutledpin(int pinnum)
{
  this->StartBoardDelayedUpdate(this->outledpin, pinnum);
}

void SetupData::set_brdirpin(int pinnum)
{
  this->StartBoardDelayedUpdate(this->irpin, pinnum);
}

void SetupData::set_brdboardpins(int pinnum)
{
  this->StartBoardDelayedUpdate(this->boardpins[pinnum], pinnum);
}

void SetupData::set_brdstepsperrev(int stepsrev)
{
  this->StartBoardDelayedUpdate(this->stepsperrev, stepsrev);
}

void SetupData::set_brdfixedstepmode(int newval)
{
  this->StartBoardDelayedUpdate(this->fixedstepmode, newval);
}

void SetupData::set_brdpb1pin(int newpin)
{
  this->StartBoardDelayedUpdate(this->pb1pin, newpin);
}

void SetupData::set_brdpb2pin(int newpin)
{
  this->StartBoardDelayedUpdate(this->pb2pin, newpin);
}

void SetupData::set_brdmsdelay(unsigned long newval)
{
  this->StartBoardDelayedUpdate(this->msdelay, newval);
}

void SetupData::StartBoardDelayedUpdate(int & org_data, int new_data)
{
  if (org_data != new_data)
  {
    this->ReqSaveBoard_var = true;
    this->BoardSnapShotMillis = millis();
    org_data = new_data;
    DebugPrintln("++ request for saving board data");
  }
}

void SetupData::StartBoardDelayedUpdate(unsigned long & org_data, unsigned long new_data)
{
  if (org_data != new_data)
  {
    this->ReqSaveBoard_var = true;
    this->BoardSnapShotMillis = millis();
    org_data = new_data;
    DebugPrintln("++ request for saving board data");
  }
}

void SetupData::StartBoardDelayedUpdate(byte & org_data, byte new_data)
{
  if (org_data != new_data)
  {
    this->ReqSaveBoard_var = true;
    this->BoardSnapShotMillis = millis();
    org_data = new_data;
    DebugPrintln("++ request for saving board data");
  }
}

void SetupData::StartBoardDelayedUpdate(String & org_data, String new_data)
{
  if (org_data != new_data)
  {
    this->ReqSaveBoard_var = true;
    this->BoardSnapShotMillis = millis();
    org_data = new_data;
    DebugPrintln("++ request for saving board data");
  }
}

// ======================================================================
// Misc
// ======================================================================

void SetupData::ListDir(const char * dirname, uint8_t levels)
{
  // TODO
  // THIS DOES NOT WORK ON ESP8266!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#if defined(ESP8266)
  DebugPrintln("SetupData::ListDir() does not work on ESP8266");
#else
  File root = SPIFFS.open(dirname);
  delay(10);
  DebugPrint("Listing directory: {");

  if (!root)
  {
    DebugPrintln(" - failed to open directory");
  }
  else
  {
    if (!root.isDirectory())
    {
      DebugPrintln(" - not a directory");
    }
    else
    {
      File file = root.openNextFile();
      delay(10);
      int i = 0;
      while (file)
      {
        if (file.isDirectory())
        {
          DebugPrint("  DIR : ");
          DebugPrintln(file.name());
          if (levels)
          {
            this->ListDir(file.name(), levels - 1);
          }
        }
        else
        {
          DebugPrint(file.name());
          DebugPrint(":");
          DebugPrint(file.size());
          if ((++i % 6) == 0)
          {
            DebugPrintln("");
          }
          else
          {
            DebugPrint("  ");
          }
        }
        delay(10);
        file = root.openNextFile();
      }
      DebugPrintln("}");
    }
  }
#endif
}
