#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>

#if defined(ESP8266)
 #include <FS.h>
#else
 #include "SPIFFS.h"
#endif

#include "FocuserSetupData.h"
#include "generalDefinitions.h"

SetupData::SetupData(byte set_mode)
{
  DebugPrintln("Constructor Setupdata");

  this->mode = set_mode;
  if (mode == Mode_EEPROM)
  {
    DebugPrintln(F("Mode: EEPROM"));
  }
  else
  {
    DebugPrintln(F("Mode: SPIFFS"));
  }

  this->DataAssign = 0;
  this->SnapShotMillis = millis();

  // mount SPIFFS
  if (!SPIFFS.begin()) {
    DebugPrintln("An Error has occurred while mounting SPIFFS");
    return;
  }
  else
  {
    DebugPrintln("SPIFFS mounted");
  }

  this->LoadConfiguration();
};

// Loads the configuration from a file
byte SetupData::LoadConfiguration() 
{
  byte retval = 0;

  // Open file for reading
  File file = SPIFFS.open(filename_persistant, "r");
  LoadDefaultPersistantData();

  file = SPIFFS.open(filename_vaiable, "r");
  if (!file) {
    DebugPrintln("no config file variable data found, load default values");
    LoadDefaultVariableData();
    retval = 1;
  }
  else {
    // Allocate a temporary JsonDocument
    StaticJsonDocument<64> doc_var;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc_var, file);
    if (error) {
      DebugPrintln(F("Failed to read variable data file, using default configuration"));
      LoadDefaultVariableData();
      retval = 2;
    }
    else {
      this->fposition = doc_var["fpos"];         // last focuser position
      this->focuserdirection = doc_var["fdir"];  // keeps track of last focuser move direction
      retval = 3;
    }
  }
  file.close();
  DebugPrintln(F("config file variable data loaded"));
  return retval;
}

void SetupData::SetFocuserDefaults(void)
{
  LoadDefaultPersistantData();
  LoadDefaultVariableData();

  SPIFFS.remove(filename_persistant);
  SPIFFS.remove(filename_vaiable);
}

void SetupData::LoadDefaultPersistantData()
{
  this->maxstep = 80000L;
  this->coilpower = 0;
  this->reversedirection = 0;
  this->stepsizeenabled = 0;                  // default state is step size OFF
  this->stepsize = DEFAULTSTEPSIZE;
  this->DelayAfterMove = 0;
  this->backlashsteps_in = 88;
  this->backlashsteps_out = 88;
  this->backlash_in_enabled = 1;
  this->backlash_out_enabled = 1;
  this->tempcoefficient = 0;
  this->tempprecision = TEMP_PRECISION;
  this->stepmode = 1;                         // step mode 1=full, 2, 4, 8, 16, 32
  this->tcdirection = 0;                      // temperature compensation direction 1
  this->tempmode = 1;                         // default is celsius
  this->tempcompenabled = 0;                  // temperature compensation disabled
  this->lcdupdateonmove = 1;
  this->lcdpagetime = 20;                     // 20, 25, 30, 35, 40
  this->motorSpeed = FAST;
  this->displayenabled = 0;
}

void SetupData::LoadDefaultVariableData()
{
  this->fposition = 5000L;                    // last focuser position
  this->focuserdirection = moving_in;         // keeps track of last focuser move direction
}

// Saves the configuration to a file
byte SetupData::SaveConfiguration(unsigned long currentPosition, byte DirOfTravel)
{
  if (this->fposition != currentPosition || this->focuserdirection != DirOfTravel)  // last focuser position
  {
    this->fposition = currentPosition;
    this->focuserdirection = DirOfTravel;
    DataAssign |= 2;    // set bit1
    SnapShotMillis = millis();
  }

  byte status = false;
  unsigned long x = millis();

  if ((SnapShotMillis + 30000) < x || SnapShotMillis > x)    // 30s after snapshot
  {
    if (DataAssign & 1)
    {
      if (SavePersitantConfiguration() == false)
      {
        DebugPrintln(F("Error save persitant configuration"));
      }
      status |= 1;
      DataAssign &= (byte) ~1;   // clear bit0
    }

    if (DataAssign & 2)
    {
      if (SaveVariableConfiguration() == false)
      {
        DebugPrintln(F("Error save variable configuration"));
      }
      status |= 2;
      DataAssign &= (byte) ~2;   // clear bit1
    }
  }
  return status;
}

byte SetupData::SavePersitantConfiguration()
{
  SPIFFS.remove(filename_persistant);                // Delete existing file

  File file = SPIFFS.open(filename_persistant, "w"); // Open file for writing
  if (!file) {
    DebugPrintln(F("Failed to create file for persitant data"));
    return false;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Set the values in the document
  doc["maxstep"] = this->maxstep;                           // max steps
  doc["stepsize"] = this->stepsize;                         // the step size in microns, ie 7.2 - value * 10, so real stepsize = stepsize / 10 (maxval = 25.6)
  doc["DelayAfterMove"] = this->DelayAfterMove;             // delay after movement is finished (maxval=256)
  doc["backlashsteps_in"] = this->backlashsteps_in;         // number of backlash steps to apply for IN moves
  doc["backlashsteps_out"] = this->backlashsteps_out;       // number of backlash steps to apply for OUT moves
  doc["backlash_in_enabled"] = this->backlash_in_enabled;
  doc["backlash_out_enabled"] = this->backlash_out_enabled;
  doc["tempcoefficient"] = this->tempcoefficient;           // steps per degree temperature coefficient value (maxval=256)
  doc["tempprecision"] = this->tempprecision;
  doc["stepmode"] = this->stepmode;
  doc["coilpower"] = this->coilpower;
  doc["reversedirection"] = this->reversedirection;
  doc["stepsizeenabled"] = this->stepsizeenabled;           // if 1, controller returns step size
  doc["tempmode"] = this->tempmode;                         // temperature display mode, Celcius=1, Fahrenheit=0
  doc["lcdupdateonmove"] = this->lcdupdateonmove;           // update position on lcd when moving
  doc["lcdpagetime"] = this->lcdpagetime;                   // *100 to give interval between lcd pages display time
  doc["tempcompenabled"] = this->tempcompenabled;           // indicates if temperature compensation is enabled
  doc["tcdirection"] = this->tcdirection;
  doc["motorSpeed"] = this->motorSpeed;
  doc["displayenabled"] = this->displayenabled;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    DebugPrintln("Failed to write to file");
    file.close();     // Close the file
    return false;
  }
  else {
    file.close();     // Close the file
    return true;
  }
}

byte SetupData::SaveVariableConfiguration()
{

  // Delete existing file
  SPIFFS.remove(filename_vaiable);

  // Open file for writing
  File file = SPIFFS.open(this->filename_vaiable, "w");
  if (!file) {
    DebugPrintln(F("Failed to create file for variable data"));
    return false;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<64> doc;

  // Set the values in the document
  doc["fpos"] =  this->fposition;                  // last focuser position
  doc["fdir"] =  this->focuserdirection;           // keeps track of last focuser move direction

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    DebugPrintln("Failed to write to file");
    file.close();     // Close the file
    return false;
  }
  else {
    file.close();     // Close the file
    return true;
  }
}


//__getter

byte SetupData::get_mode()                {
  return this->mode;
}
unsigned long SetupData::get_fposition()  {
  return this->fposition; // last focuser position
}
byte SetupData::get_focuserdirection()    {
  return this->focuserdirection; // keeps track of last focuser move direction
}
unsigned long  SetupData::get_maxstep()   {
  return this->maxstep; // max steps
}
float SetupData::get_stepsize()           {
  return this->stepsize; // the step size in microns, ie 7.2 - value * 10, so real stepsize = stepsize / 10 (maxval = 25.6)
}
byte SetupData::get_DelayAfterMove()      {
  return this->DelayAfterMove; // delay after movement is finished (maxval=256)
}
byte SetupData::get_backlashsteps_in()    {
  return this->backlashsteps_in; // number of backlash steps to apply for IN moves
}
byte SetupData::get_backlashsteps_out()   {
  return this->backlashsteps_out; // number of backlash steps to apply for OUT moves
}
byte SetupData::get_backlash_in_enabled() {
  return this->backlash_in_enabled;
}
byte SetupData::get_backlash_out_enabled() {
  return this->backlash_out_enabled;
}
byte SetupData::get_tempcoefficient()     {
  return this->tempcoefficient; // steps per degree temperature coefficient value (maxval=256)
}
byte SetupData::get_tempprecision()       {
  return this->tempprecision;
}
byte SetupData::get_stepmode()            {
  return this->stepmode;
}
byte SetupData::get_coilpower()           {
  return this->coilpower;
}
byte SetupData::get_reversedirection()    {
  return this->reversedirection;
}
byte SetupData::get_stepsizeenabled()     {
  return this->stepsizeenabled; // if 1, controller returns step size
}
byte SetupData::get_tempmode()            {
  return this->tempmode; // temperature display mode, Celcius=1, Fahrenheit=0
}
byte SetupData::get_lcdupdateonmove()     {
  return this->lcdupdateonmove; // update position on lcd when moving
}
byte SetupData::get_lcdpagetime()         {
  return this->lcdpagetime;
}
byte SetupData::get_tempcompenabled()     {
  return this->tempcompenabled; // indicates if temperature compensation is enabled
}
byte SetupData::get_tcdirection()         {
  return this->tcdirection;
}
byte SetupData::get_motorSpeed()          {
  return this->motorSpeed;
}
byte SetupData::get_displayenabled()      {
  return this->displayenabled;
}

//__Setter

void SetupData::set_fposition(unsigned long fposition)      {
  this->fposition = fposition; // last focuser position
}
void SetupData::set_focuserdirection(byte focuserdirection) {
  this->focuserdirection = focuserdirection; // keeps track of last focuser move direction
}
void SetupData::set_maxstep(unsigned long maxstep)          {
  this->StartDelayedUpdate(this->maxstep, maxstep); // max steps
}
void SetupData::set_stepsize(float stepsize)                {
  this->StartDelayedUpdate(this->stepsize, stepsize); // the step size in microns, ie 7.2 - value * 10, so real stepsize = stepsize / 10 (maxval = 25.6)
}
void SetupData::set_DelayAfterMove(byte DelayAfterMove)     {
  this->StartDelayedUpdate(this->DelayAfterMove, DelayAfterMove); // delay after movement is finished (maxval=256)
}
void SetupData::set_backlashsteps_in(byte backlashsteps)    {
  this->StartDelayedUpdate(this->backlashsteps_in, backlashsteps); // number of backlash steps to apply for IN moves
}
void SetupData::set_backlashsteps_out(byte backlashsteps_out) {
  this->StartDelayedUpdate(this->backlashsteps_out, backlashsteps_out); // number of backlash steps to apply for OUT moves
}
void SetupData::set_backlash_in_enabled(byte backlash_in_enabled) {
  this->StartDelayedUpdate(this->backlash_in_enabled, backlash_in_enabled);
}
void SetupData::set_backlash_out_enabled(byte backlash_out_enabled) {
  this->StartDelayedUpdate(this->backlash_out_enabled, backlash_out_enabled);
}
void SetupData::set_tempcoefficient(byte tempcoefficient)   {
  this->StartDelayedUpdate(this->tempcoefficient, tempcoefficient); // steps per degree temperature coefficient value (maxval=256)
}
void SetupData::set_tempprecision(byte tempprecision)       {
  this->StartDelayedUpdate(this->tempprecision, tempprecision);
}
void SetupData::set_stepmode(byte stepmode)                 {
  this->StartDelayedUpdate(this->stepmode, stepmode);
}
void SetupData::set_coilpower(byte coilpower)               {
  this->StartDelayedUpdate(this->coilpower, coilpower);
}
void SetupData::set_reversedirection(byte reversedirection) {
  this->StartDelayedUpdate(this->reversedirection, reversedirection);
}
void SetupData::set_stepsizeenabled(byte stepsizeenabled)   {
  this->StartDelayedUpdate(this->stepsizeenabled, stepsizeenabled); // if 1, controller returns step size
}
void SetupData::set_tempmode(byte tempmode)                 {
  this->StartDelayedUpdate(this->tempmode, tempmode); // temperature display mode, Celcius=1, Fahrenheit=0
}
void SetupData::set_lcdupdateonmove(byte lcdupdateonmove)   {
  this->StartDelayedUpdate(this->lcdupdateonmove, lcdupdateonmove); // update position on lcd when moving
}
void SetupData::set_lcdpagetime(byte lcdpagetime)           {
  this->StartDelayedUpdate(this->lcdpagetime, lcdpagetime);
}
void SetupData::set_tempcompenabled(byte tempcompenabled)   {
  this->StartDelayedUpdate(this->tempcompenabled, tempcompenabled); // indicates if temperature compensation is enabled
}
void SetupData::set_tcdirection(byte tcdirection)           {
  this->StartDelayedUpdate(this->tcdirection, tcdirection);
}
void SetupData::set_motorSpeed(byte motorSpeed)             {
  this->StartDelayedUpdate(this->motorSpeed, motorSpeed);
}
void SetupData::set_displayenabled(byte displaystate)       {
  this->StartDelayedUpdate(this->displayenabled, displaystate);
}

void SetupData::StartDelayedUpdate(unsigned long & org_data, unsigned long new_data)
{
  if (org_data != new_data) {
    DataAssign |= 1;    // set bit0
    SnapShotMillis = millis();
    org_data = new_data;
  }
}

void SetupData::StartDelayedUpdate(float & org_data, float new_data)
{
  if (org_data != new_data) {
    DataAssign |= 1;    // set bit0
    SnapShotMillis = millis();
    org_data = new_data;
  }
}

void SetupData::StartDelayedUpdate(byte & org_data, byte new_data)
{
  if (org_data != new_data) {
    DataAssign |= 1;    // set bit0
    SnapShotMillis = millis();
    org_data = new_data;
  }
}
