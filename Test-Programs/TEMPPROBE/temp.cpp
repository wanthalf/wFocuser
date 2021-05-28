// ======================================================================
// temp.cpp : myFP2ESP TEMPERATURE PROBE
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger Manz, 2020-2021. All Rights Reserved.
// ======================================================================

// ======================================================================
// EXTERNALS
// ======================================================================

#include <Arduino.h>
#include "FocuserSetupData.h"
#include "generalDefinitions.h"
#include "myBoards.h"
#include <OneWire.h>                          // https://github.com/PaulStoffregen/OneWire
#include <myDallasTemperature.h>

#include "temp.h"

// ======================================================================
// EXTERNALS
// ======================================================================
extern int   tprobe1;
extern float lasttemp;

extern SetupData *mySetupData;
extern bool TimeCheck(unsigned long, unsigned long);
extern unsigned long ftargetPosition;         // target position

// ======================================================================
// DATA
// ======================================================================

OneWire oneWirech1(0);                              // dummy pin, will change when tempprobe is instantiated
DallasTemperature sensor1(&oneWirech1);
DeviceAddress tpAddress;                            // holds address of the temperature probe

// ======================================================================
// CLASS
// ======================================================================
TempProbe::TempProbe()  :  DallasTemperature (&oneWirech1)
{
  start_temp_probe();
}

void TempProbe::start_temp_probe()
{
  Temp_DebugPrintln("start_temp_probe()");
  tprobe1 = false;
  // check if valid pin is defined for board
  if ( mySetupData->get_brdtemppin() == -1 )
  {
    Temp_DebugPrintln("temp pin not defined.");
  }
  else
  { 
    oneWirech1.begin( mySetupData->get_brdtemppin() );    // start onewire;
    Temp_DebugPrintln("begin");
    begin();                                              // start dallas temp probe sensor1
    Temp_DebugPrintln("get device count");
    tprobe1 = getDeviceCount();                           // should return 1 if probe connected
    Temp_DebugPrint("Probes:");
    Temp_DebugPrintln(tprobe1);
    if ( tprobe1 != 0 )
    {
      if (getAddress(tpAddress, 0) == true)               // get the address so we can set the probe resolution
      {
        tprobe1 = 1;                                      // address was found so there was a probe
        setResolution(tpAddress, mySetupData->get_tempresolution());   // set probe resolution
        Temp_DebugPrint("set resolution:");
        switch (mySetupData->get_tempresolution())
        {
          case 9: Temp_DebugPrintln("0.5");
            break;
          case 10: Temp_DebugPrintln("0.25");
            break;
          case 11: Temp_DebugPrintln("0.125");
            break;
          case 12: Temp_DebugPrintln("0.0625");
            break;
          default:
            Temp_DebugPrintln("Unknown");
            break;
        }
        requestTemperatures();                            // request the sensor to begin a temperature reading
      }
    }
    else
    {
      Temp_DebugPrintln("Probe not found");
    }
  }
}

void TempProbe::stop_temp_probe()
{
  tprobe1 = 0;
}

void TempProbe::temp_setresolution(byte rval)
{
  if ( tprobe1 != 0 )
  {
    sensor1.setResolution(tpAddress, rval);               // set the probe resolution (9=0.25, 12=0.00125)
  }
  else
  {
    Temp_DebugPrintln("err: No probe found.");
  }
}

float TempProbe::read_temp(byte new_measurement)
{
  if (!new_measurement || ( tprobe1 == 0 ))
  {
    return lasttemp;                                      // return previous measurement
  }

  float result = sensor1.getTempCByIndex(0);              // get temperature, always in celsius
  Temp_DebugPrint("Temp: ");
  Temp_DebugPrintln(result);
  if (result > -40.0 && result < 80.0)                    // avoid erronous readings
  {
    lasttemp = result;
  }
  else
  {
    result = lasttemp;
  }
  return result;
}

void TempProbe::update_temp(void)
{
  if ( mySetupData->get_temperatureprobestate() == 1)
  {
    if (tprobe1 == 1)
    {
      static unsigned long lasttempconversion = 0;
      static byte requesttempflag = 0;                      // start with request
      unsigned long tempnow = millis();

      // see if the temperature needs updating - done automatically every 1.5s
      if (TimeCheck(lasttempconversion, TEMPREFRESHRATE))   // see if the temperature needs updating
      {
        static float tempval;

        lasttempconversion = tempnow;                       // update time stamp

        if (requesttempflag)
        {
          tempval = read_temp(1);
        }
        else
        {
          sensor1.requestTemperatures();
        }

        requesttempflag ^= 1; // toggle flag

      } // end of check for temperature needs updating
    } // end of if tprobe
    else
    {
      Serial.println("err: No probe found.");
    }
  }
  else
  {
    Serial.println("err: Probe disabled.");
  }
}
