#include <Arduino.h>

#include "generalDefinitions.h"

#define Mode_EEPROM true
#define Mode_SPIFFS !Mode_EEPROM

class SetupData
{
  public:
    SetupData(byte);
    byte LoadConfiguration(void);
    byte SaveConfiguration(unsigned long, byte);
    void SetFocuserDefaults(void);

    //  getter
    byte get_mode(void);
    unsigned long get_fposition();
    byte get_focuserdirection();
    unsigned long get_maxstep();
    float get_stepsize();
    byte get_DelayAfterMove();
    byte get_backlashsteps_in();
    byte get_backlashsteps_out();
    byte get_backlash_in_enabled();
    byte get_backlash_out_enabled();
    byte get_tempcoefficient();
    byte get_tempprecision();
    byte get_stepmode();
    byte get_coilpower();
    byte get_reversedirection();
    byte get_stepsizeenabled();
    byte get_tempmode();
    byte get_lcdupdateonmove();
    byte get_lcdpagetime();
    byte get_tempcompenabled();
    byte get_tcdirection();
    byte get_motorSpeed();
    byte get_displayenabled();

    //__setter
    void set_fposition(unsigned long);
    void set_focuserdirection(byte);
    void set_maxstep(unsigned long);
    void set_stepsize(float);
    void set_DelayAfterMove(byte);
    void set_backlashsteps_in(byte);
    void set_backlashsteps_out(byte);
    void set_backlash_in_enabled(byte);
    void set_backlash_out_enabled(byte);
    void set_tempcoefficient(byte);
    void set_tempprecision(byte);
    void set_stepmode(byte);
    void set_coilpower(byte);
    void set_reversedirection(byte);
    void set_stepsizeenabled(byte);
    void set_tempmode(byte);
    void set_lcdupdateonmove(byte);
    void set_lcdpagetime(byte);
    void set_tempcompenabled(byte);
    void set_tcdirection(byte);
    void set_motorSpeed(byte);
    void set_displayenabled(byte);

  private:
    byte SavePersitantConfiguration();
    byte SaveVariableConfiguration();

    void LoadDefaultPersistantData(void);
    void LoadDefaultVariableData(void);

    void StartDelayedUpdate(unsigned long &, unsigned long);
    void StartDelayedUpdate(float &, float);
    void StartDelayedUpdate(byte &, byte);

    byte mode;                                          // store to EEPROM or SPIFFS
    byte DataAssign;
    const String filename_persistant = "/data_per.jsn"; // persistant JSON setup data
    const String filename_vaiable = "/data_var.jsn";    // variable  JSON setup data

    unsigned long fposition;        // last focuser position
    byte focuserdirection;          // keeps track of last focuser move direction
    unsigned long fposition_org;    // last focuser position
    byte focuserdirection_org;      // keeps track of last focuser move direction
    unsigned long SnapShotMillis;

    //dataset_persistant
    unsigned long maxstep;          // max steps
    float stepsize;                 // the step size in microns, ie 7.2 - value * 10, so real stepsize = stepsize / 10 (maxval = 25.6)
    byte DelayAfterMove;            // delay after movement is finished (maxval=256)
    byte backlashsteps_in;          // number of backlash steps to apply for IN moves
    byte backlashsteps_out;         // number of backlash steps to apply for OUT moves
    byte backlash_in_enabled;
    byte backlash_out_enabled;
    byte tempcoefficient;           // steps per degree temperature coefficient value (maxval=255)
    byte tempprecision;             // 9 -12
    byte stepmode;
    byte coilpower;
    byte reversedirection;
    byte stepsizeenabled;           // if 1, controller returns step size
    byte tempmode;                  // temperature display mode, Celcius=1, Fahrenheit=0
    byte lcdupdateonmove;           // update position on lcd when moving
    byte lcdpagetime;
    byte tempcompenabled;           // indicates if temperature compensation is enabled
    byte tcdirection;
    byte motorSpeed;
    byte displayenabled;
};
