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
#include <DallasTemperature.h>

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
  DebugPrintln("start_temp_probe()");
  tprobe1 = false;
  // check if valid pin is defined for board
  if ( mySetupData->get_brdtemppin() == -1 )
  {
    DebugPrintln("temp pin not defined.");
  }
  else
  { 
    oneWirech1.begin( mySetupData->get_brdtemppin() );    // start onewire;
    DebugPrintln("begin");
    begin();                                              // start dallas temp probe sensor1
    DebugPrintln("get device count");
    tprobe1 = getDeviceCount();                           // should return 1 if probe connected
    DebugPrint("Probes:");
    DebugPrintln(tprobe1);
    if ( tprobe1 != 0 )
    {
      if (getAddress(tpAddress, 0) == true)               // get the address so we can set the probe resolution
      {
        tprobe1 = 1;                                      // address was found so there was a probe
        setResolution(tpAddress, mySetupData->get_tempresolution());   // set probe resolution
        DebugPrint("set resolution:");
        switch (mySetupData->get_tempresolution())
        {
          case 9: DebugPrintln("0.5");
            break;
          case 10: DebugPrint("0.25");
            break;
          case 11: DebugPrintln("0.125");
            break;
          case 12: DebugPrintln("0.0625");
            break;
          default:
            DebugPrintln("Unknown");
            break;
        }
        requestTemperatures();                            // request the sensor to begin a temperature reading
      }
    }
    else
    {
      DebugPrintln("Probe not found");
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
    DebugPrintln("err: No probe found.");
  }
}

float TempProbe::read_temp(byte new_measurement)
{
  if (!new_measurement || ( tprobe1 == 0 ))
  {
    return lasttemp;                                      // return previous measurement
  }

  float result = sensor1.getTempCByIndex(0);              // get temperature, always in celsius
  DebugPrint("temp:");
  DebugPrintln(result);
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
  static byte tcchanged = mySetupData->get_tempcompenabled();  // keeps track if tempcompenabled changes

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
        static float starttemp;                             // start temperature to use when temperature compensation is enabled

        if ( tcchanged != mySetupData->get_tempcompenabled() )
        {
          tcchanged = mySetupData->get_tempcompenabled();
          if ( tcchanged == 1 )
          {
            starttemp = read_temp(1);
          }
        }

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

        if (mySetupData->get_tempcompenabled() == 1)    // check for temperature compensation
        {
          if ((abs)(starttemp - tempval) >= 1)          // calculate if temp has moved by more than 1 degree
          {
            unsigned long newPos;
            byte temperaturedirection;                  // did temperature fall (1) or rise (0)?
            temperaturedirection = (tempval < starttemp) ? 1 : 0;
            if (mySetupData->get_tcdirection() == 0)    // check if tc direction for compensation is inwards
            {
              // temperature compensation direction is in, if a fall then move in else move out
              if ( temperaturedirection == 1 )          // check if temperature is falling
              {
                newPos = ftargetPosition - mySetupData->get_tempcoefficient();    // then move inwards
              }
              else
              {
                newPos = ftargetPosition + mySetupData->get_tempcoefficient();    // else move outwards
              }
            }
            else
            {
              // temperature compensation direction is out, if a fall then move out else move in
              if ( temperaturedirection == 1 )
              {
                newPos = ftargetPosition + mySetupData->get_tempcoefficient();
              }
              else
              {
                newPos = ftargetPosition - mySetupData->get_tempcoefficient();
              }
            }
            newPos = (newPos > mySetupData->get_maxstep()) ? mySetupData->get_maxstep() : newPos;
            // newPos should be checked for < 0 but cannot due to unsigned
            // newPos = (newPos < 0 ) ? 0 : newPos;
            ftargetPosition = newPos;
            starttemp = tempval;                        // save this current temp point for future reference
          } // end of check for tempchange >=1
        } // end of check for tempcomp enabled
      } // end of check for temperature needs updating
    } // end of if tprobe
    else
    {
      DebugPrintln("err: No probe found.");
    }
  }
  else
  {
    DebugPrintln("err: Probe disabled.");
  }
}
