// ======================================================================
// temp.h : myFP2ESP support for DS18B20 Temperature Sensor
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger Manz, 2020-2021. All Rights Reserved.
// ======================================================================

// ======================================================================
// Includes
// ======================================================================
#include <myDallasTemperature.h>

// ======================================================================
// TEMPERATURE Class
// ======================================================================
//class TempProbe
class TempProbe : public DallasTemperature
{
  public:
    TempProbe();
    void  init_temp(void);
    void  temp_setresolution(byte rval);
    float read_temp(byte new_measurement);
    void  update_temp(void);
    void  start_temp_probe(void);
    void  stop_temp_probe(void);

    // we cannot place tprobe1 here - because if the object is disposed then we cannot access it and generates an exception
    // other functions reference tprobe1 to see if a probe was found - so it cannot be in this class
};
