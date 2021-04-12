// ======================================================================
// webserver.h : myFP2ESP WEB SERVER DEFINITIONS
// (c) Copyright Robert Brown 2014-2021. All Rights Reserved.
// (c) Copyright Holger M, 2019-2021. All Rights Reserved.
// ======================================================================

#ifndef webserver_h
#define webserver_h

// ======================================================================
// DATA
// ======================================================================
// stepmode html to create radio buttons on web page
#define WS_SM1CHECKED             "<input type=\"radio\" name=\"sm\" value=\"1\" Checked> 1"
#define WS_SM1UNCHECKED           "<input type=\"radio\" name=\"sm\" value=\"1\"> 1"
#define WS_SM2CHECKED             "<input type=\"radio\" name=\"sm\" value=\"2\" Checked> 2"
#define WS_SM2UNCHECKED           "<input type=\"radio\" name=\"sm\" value=\"2\"> 2"
#define WS_SM4CHECKED             "<input type=\"radio\" name=\"sm\" value=\"4\" Checked> 4"
#define WS_SM4UNCHECKED           "<input type=\"radio\" name=\"sm\" value=\"4\"> 4"
#define WS_SM8CHECKED             "<input type=\"radio\" name=\"sm\" value=\"8\" Checked> 8"
#define WS_SM8UNCHECKED           "<input type=\"radio\" name=\"sm\" value=\"8\"> 8"
#define WS_SM16CHECKED            "<input type=\"radio\" name=\"sm\" value=\"16\" Checked> 16"
#define WS_SM16UNCHECKED          "<input type=\"radio\" name=\"sm\" value=\"16\"> 16"
#define WS_SM32CHECKED            "<input type=\"radio\" name=\"sm\" value=\"32\" Checked> 32"
#define WS_SM32UNCHECKED          "<input type=\"radio\" name=\"sm\" value=\"32\"> 32"

// Motor speed html to create radio buttons on web page
#define WS_MSSLOWCHECKED          "<input type=\"radio\" name=\"ms\" value=\"0\" Checked> S"
#define WS_MSSLOWUNCHECKED        "<input type=\"radio\" name=\"ms\" value=\"0\"> S"
#define WS_MSMEDCHECKED           "<input type=\"radio\" name=\"ms\" value=\"1\" Checked> M"
#define WS_MSMEDUNCHECKED         "<input type=\"radio\" name=\"ms\" value=\"1\"> M"
#define WS_MSFASTCHECKED          "<input type=\"radio\" name=\"ms\" value=\"2\" Checked> F"
#define WS_MSFASTUNCHECKED        "<input type=\"radio\" name=\"ms\" value=\"2\"> F"

#endif // ifndef webserver_h
