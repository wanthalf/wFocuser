// ----------------------------------------------------------------------------------------------
// TITLE: myFP2ESP FIRMWARE OFFICIAL RELEASE 
// TEST HOMEPOSITIONSWITCH-1
// ----------------------------------------------------------------------------------------------

// Output is shown in serial port window
// This program checks for the HPSW - you must close and open the switch by hand
// If the switch is open it will be shown as open
// If the switch is closed it will be shown as closed

// ----------------------------------------------------------------------------------------------
// COPYRIGHT
// ----------------------------------------------------------------------------------------------
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.

// ----------------------------------------------------------------------------------------------
// 16: FIRMWARE CODE START - CHANGE AT YOUR OWN PERIL
// ----------------------------------------------------------------------------------------------

#define HPSWPIN       4


void setup()
{
  Serial.begin(115200);
  delay(500);

  Serial.println("Init HPSW");
  pinMode(HPSWPIN, INPUT_PULLUP);
}

void loop()
{
  if( digitalRead(HPSWPIN == 0 ))
  {
    Serial.println("HPSW State: 0 or open");
  }
  else
  {
    Serial.println("HPSW State: 1 or closed");
  }
  delay(2000);
} // end Loop()
