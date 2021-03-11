// ======================================================================
// FocuserSetupData.h : myFP2ESP FOCUSER DATA DEFINITIONS
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

#include <Arduino.h>

//#include "generalDefinitions.h"
//#include "boarddefs.h"

#define DEFAULTOFF              0
#define DEFAULTON               1
#define DEFAULTCELSIUS          1
#define DEFAULTFAHREN           0
#define DEFAULTDOCSIZE          2048      // board configuration - about 832 - https://arduinojson.org/v6/assistant/ deserialize
#define DEFAULTVARDOCSIZE       64
#define DEFAULTBOARDSIZE        1024      // board configuration - about 300 - https://arduinojson.org/v6/assistant/ deserialize

class SetupData
{
  public:
    SetupData(void);
    byte LoadConfiguration(void);
    boolean SaveConfiguration(unsigned long, byte);
    boolean SaveBoardConfiguration(void);               // delayed save board_config.jsn if changed
    boolean SaveNow(void);
    boolean SaveBoardConfigNow(void);                   // immediate save borad_config.jsn
    void    SetFocuserDefaults(void);
    boolean CreateBoardConfigfromjson(String);          // create a board config frm a json string - used by Management Server
    boolean LoadBrdConfigStart(String);                 // attempt to load a board config file [DRVBRD] immediately after a firmware reprogram
    
    //  getter data_per
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
    byte get_tempresolution();
    byte get_coilpower();
    byte get_reversedirection();
    byte get_stepsizeenabled();
    byte get_tempmode();
    byte get_lcdupdateonmove();
    byte get_lcdpagetime();
    byte get_tempcompenabled();
    byte get_tcdirection();
    byte get_motorspeed();
    byte get_displayenabled();
    unsigned long get_focuserpreset(byte);
    unsigned long get_webserverport();
    unsigned long get_ascomalpacaport();
    int get_webpagerefreshrate();
    unsigned long get_mdnsport();
    unsigned long get_tcpipport();
    byte get_showstartscreen();
    String  get_wp_backcolor();
    String  get_wp_textcolor();
    String  get_wp_headercolor();
    String  get_wp_titlecolor();
    byte get_ascomserverstate();
    byte get_webserverstate();
    byte get_inoutledstate();
    byte get_temperatureprobestate();
    byte get_showhpswmsg();
    byte get_forcedownload();
    String  get_oledpageoption();
    byte get_hpswitchenable();
    byte get_pbenable();
    byte get_indi();

    //__setter data_per
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
    void set_tempresolution(byte);
    void set_coilpower(byte);
    void set_reversedirection(byte);
    void set_stepsizeenabled(byte);
    void set_tempmode(byte);
    void set_lcdupdateonmove(byte);
    void set_lcdpagetime(byte);
    void set_tempcompenabled(byte);
    void set_tcdirection(byte);
    void set_motorspeed(byte);
    void set_displayenabled(byte);
    void set_focuserpreset(byte, unsigned long);
    void set_webserverport(unsigned long);
    void set_ascomalpacaport(unsigned long);
    void set_webpagerefreshrate(int);
    void set_mdnsport(unsigned long);
    void set_tcpipport(unsigned long);
    void set_showstartscreen(byte);
    void set_wp_backcolor(String);
    void set_wp_textcolor(String);
    void set_wp_headercolor(String);
    void set_wp_titlecolor(String);
    void set_ascomserverstate(byte);
    void set_webserverstate(byte);
    void set_temperatureprobestate(byte);
    void set_inoutledstate(byte);
    void set_showhpswmsg(byte);
    void set_forcedownload(byte);
    void set_oledpageoption(String);
    void set_hpswitchenable(byte);
    void set_pbenable(byte);
    void set_irremoteenable(byte);
    void set_indi(byte);
    
    //__getter boardconfig
    String get_brdname(void);
    int get_brdmaxstepmode(void);
    int get_brdstepmode(void);
    int get_brdsda(void);
    int get_brdsck(void);
    int get_brdenablepin(void);
    int get_brdsteppin(void);
    int get_brddirpin(void);
    int get_brdtemppin(void);
    int get_brdhpswpin(void);
    int get_brdinledpin(void);
    int get_brdoutledpin(void);
    int get_brdirpin(void);
    int get_brdboardpins(int);
    int get_brdstepsperrev(void);
    int get_brdfixedstepmode(void);
    int get_brdpb1pin(void);
    int get_brdpb2pin(void);
    unsigned long get_brdmsdelay(void);

    //__setter boardconfig
    void set_brdname(String);
    void set_brdmaxstepmode(int);
    void set_brdstepmode(int);
    void set_brdsda(int);
    void set_brdsck(int);
    void set_brdenablepin(int);
    void set_brdsteppin(int);
    void set_brddirpin(int);
    void set_brdtemppin(int);
    void set_brdhpswpin(int);
    void set_brdinledpin(int);
    void set_brdoutledpin(int);
    void set_brdirpin(int);
    void set_brdboardpins(int);
    void set_brdstepsperrev(int);
    void set_brdfixedstepmode(int);
    void set_brdpb1pin(int);
    void set_brdpb2pin(int);
    void set_brdmsdelay(unsigned long);

  private:
    byte SavePersitantConfiguration();
    byte SaveVariableConfiguration();
    bool WriteBoardConfiguration();
    void LoadDefaultPersistantData(void);
    void LoadDefaultVariableData(void);
    void LoadDefaultBoardData(void);
    void LoadBoardConfiguration(void);
    void SetDefaultBoardData(void);
    
    void StartDelayedUpdate(unsigned long &, unsigned long);
    void StartDelayedUpdate(float &, float);
    void StartDelayedUpdate(byte &, byte);
    void StartDelayedUpdate(int &, int);
    void StartDelayedUpdate(String &, String);

    void StartBoardDelayedUpdate(unsigned long &, unsigned long);
    void StartBoardDelayedUpdate(float &, float);
    void StartBoardDelayedUpdate(byte &, byte);
    void StartBoardDelayedUpdate(int &, int);
    void StartBoardDelayedUpdate(String &, String);

    void ListDir(const char*, uint8_t);

    boolean ReqSaveData_var;        // Flag for request save variable data
    boolean ReqSaveData_per;        // Flag for request save persitant data
    boolean ReqSaveBoard_var;       // Flag for request save board configuration data

    const String filename_persistant  = "/data_per.jsn";      // persistant JSON setup data
    const String filename_variable    = "/data_var.jsn";      // variable  JSON setup data
    const String filename_boardconfig = "/board_config.jsn";  // board configuration

    unsigned long fposition;        // last focuser position
    byte          focuserdirection; // keeps track of last focuser move direction
    unsigned long SnapShotMillis;
    unsigned long BoardSnapShotMillis;

    // Focuser dataset_persistant
    unsigned long maxstep;          // max steps
    float stepsize;                 // the step size in microns, ie 7.2 - value * 10, so real stepsize = stepsize / 10 (maxval = 25.6)
    byte DelayAfterMove;            // delay after movement is finished (maxval=256)
    byte backlashsteps_in;          // number of backlash steps to apply for IN moves
    byte backlashsteps_out;         // number of backlash steps to apply for OUT moves
    byte backlash_in_enabled;       // if 1, backlash is enabled for IN movements (lower or -ve moves)
    byte backlash_out_enabled;      // if 1, backlash is enabled for OUT movements (higher or +ve moves)
    byte tempcoefficient;           // steps per degree temperature coefficient value (maxval=255)
    byte tempresolution;            // 9 -12
    byte coilpower;                 // if 1, coil power is enabled
    byte reversedirection;          // if 1, motor direction is reversed
    byte stepsizeenabled;           // if 1, controller returns step size
    byte tempmode;                  // temperature display mode, Celcius=1, Fahrenheit=0
    byte lcdupdateonmove;           // update position on lcd when moving
    byte lcdpagetime;               // length of time a display page is shown for
    byte tempcompenabled;           // indicates if temperature compensation is enabled
    byte tcdirection;               // direction in which to apply temperature compensation
    byte motorspeed;                // speed of motor, slow, medium or fast
    byte displayenabled;            // if 1, display is enabled
    unsigned long preset[10];       // focuser presets can be used with software or ir-remote controller
    unsigned long webserverport;
    unsigned long ascomalpacaport;
    int webpagerefreshrate;
    unsigned long mdnsport;
    unsigned long tcpipport;
    byte    startscreen;               // if 1, display shows startscreen messages on bootup
    String  backcolor;
    String  textcolor;
    String  headercolor;
    String  titlecolor;
    byte    ascomserverstate;          // if 1, then ascomserver is enabled
    byte    webserverstate;            // if 1, then webserver is enabled
    byte    temperatureprobestate;     // if 1, then temperature probe is enabled
    byte    inoutledstate;             // if 1, in out leds are enabled [only if board supports them]
    byte    showhpswmessages;          // if 1, home position switch msg's show on display if enabled
    byte    forcedownload;             // if 1, in the MANAGEMENT SERVER, a file is downloaded instead of being displayed is web browser window
    String  oledpageoption;
    byte    hpswitchenable;
    byte    pbenable;
    byte    inoutledenable;
    byte    indi;

    // dataset board configuration
    String board;
    int    maxstepmode;
    int    stepmode;
    int    sda;
    int    sck;
    int    enablepin;
    int    steppin;
    int    dirpin;
    int    temppin;
    int    hpswpin;
    int    inledpin;
    int    outledpin;
    int    pb1pin;
    int    pb2pin;
    int    irpin;
    int    boardpins[4];
    int    stepsperrev;
    int    fixedstepmode;
    unsigned long msdelay;
    /*
      { "board":"PRO2ESP32DRV8825","maxstepmode":32,"stepmode":1,"sda":21,"sck":22,"enpin":14,"steppin":33,"dirpin":32,
      "temppin":13,"hpswpin":4,"inledpin":18,"outledpin":19,"pb1pin":34,"pb2pin":35,"irpin":15,"stepsrev":-1,
      "fixedsmode":-1,"brdpins":[27,26,25,-1],"mspeed":4000 }
     */
};
