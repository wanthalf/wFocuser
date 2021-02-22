// ======================================================================
// ascomserver.h : myFP2ESP ASCOM SERVER ROUTINES AND DEFINITIONS
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

#ifndef ascomserver_h
#define ascomserver_h

// ======================================================================
// EXTERNS
// ======================================================================

// ======================================================================
// DATA AND DEFINITIONS - ASCOM SETUP WEB PAGE
// ======================================================================
#define AS_COPYRIGHT              "<p>(c) R. Brown, Holger M, 2020. All rights reserved.</p>"
#define AS_TITLE                  "<h3>myFP2ESP ASCOM REMOTE focus controller</h3>"
#define AS_PAGETITLE              "<title>myFP2ESP ASCOM SERVER</title>"

#define AS_SM1CHECKED             "<input type=\"radio\" name=\"sm\" value=\"1\" Checked> Full"
#define AS_SM1UNCHECKED           "<input type=\"radio\" name=\"sm\" value=\"1\"> Full"
#define AS_SM2CHECKED             "<input type=\"radio\" name=\"sm\" value=\"2\" Checked> 1/2"
#define AS_SM2UNCHECKED           "<input type=\"radio\" name=\"sm\" value=\"2\"> 1/2"
#define AS_SM4CHECKED             "<input type=\"radio\" name=\"sm\" value=\"4\" Checked> 1/4"
#define AS_SM4UNCHECKED           "<input type=\"radio\" name=\"sm\" value=\"4\"> 1/4"
#define AS_SM8CHECKED             "<input type=\"radio\" name=\"sm\" value=\"8\" Checked> 1/8"
#define AS_SM8UNCHECKED           "<input type=\"radio\" name=\"sm\" value=\"8\"> 1/8"
#define AS_SM16CHECKED            "<input type=\"radio\" name=\"sm\" value=\"16\" Checked> 1/16"
#define AS_SM16UNCHECKED          "<input type=\"radio\" name=\"sm\" value=\"16\"> 1/16"
#define AS_SM32CHECKED            "<input type=\"radio\" name=\"sm\" value=\"32\" Checked> 1/32"
#define AS_SM32UNCHECKED          "<input type=\"radio\" name=\"sm\" value=\"32\"> 1/32"

#define AS_MSSLOWCHECKED          "<input type=\"radio\" name=\"ms\" value=\"0\" Checked> Slow"
#define AS_MSSLOWUNCHECKED        "<input type=\"radio\" name=\"ms\" value=\"0\"> Slow"
#define AS_MSMEDCHECKED           "<input type=\"radio\" name=\"ms\" value=\"1\" Checked> Medium"
#define AS_MSMEDUNCHECKED         "<input type=\"radio\" name=\"ms\" value=\"1\"> Medium"
#define AS_MSFASTCHECKED          "<input type=\"radio\" name=\"ms\" value=\"2\" Checked> Fast"
#define AS_MSFASTUNCHECKED        "<input type=\"radio\" name=\"ms\" value=\"2\"> Fast"

#define ASCOMDISCOVERYPORT        32227
#define ASCOMGUID                 "7e239e71-d304-4e7e-acda-3ff2e2b68515"
#define ASCOMMAXIMUMARGS          10
#define ASCOMSUCCESS              0
#define ASCOMNOTIMPLEMENTED       0x400
#define ASCOMINVALIDVALUE         0x401
#define ASCOMVALUENOTSET          0x402
#define ASCOMNOTCONNECTED         0x407
#define ASCOMINVALIDOPERATION     0x40B
#define ASCOMACTIONNOTIMPLEMENTED 0x40C
#define ASCOMERRORMSGERROR        "Error"
#define ASCOMERRORMSGNULL         ""
#define ASCOMERRORNOTIMPLEMENTED  "!implemented"
#define ASCOMERRORMSGINVALID      "Bad operation"
#define ASCOMNAME                 "\"myFP2ESPASCOMR\""
#define ASCOMDESCRIPTION          "\"ASCOM driver for myFP2ESP controllers\""
#define ASCOMDRIVERINFO           "\"myFP2ESP ASCOM Driver (c) R. Brown. 2020\""
#define ASCOMMANAGEMENTINFO       "{\"ServerName\":\"myFP2ESP\",\"Manufacturer\":\"R. Brown\",\"ManufacturerVersion\":\"v1.0\",\"Location\":\"New Zealand\"}"

#endif // ifndef ascomserver_h
