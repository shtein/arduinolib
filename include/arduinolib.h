#ifndef __ARDUINOLIB_H

#include <Arduino.h>

#if (defined(ESP8266) || defined(ESP32) ) && !defined(NO_WIFI)
    #define WIFI_ENABLED
#endif


//Change commands
#define EEMC_NONE        0x00   //Nothing changed
#define EEMC_ERROR       0xFF   //Input error

//Errors
#define EEER_SUCCESS    0x00 //No error
#define EEER_INVALID    0x01 //Invalid input
#define EEER_UNHANDLED  0x02 //Unhandled command
#define EEER_DELAYED    0x03 //Delayed response


///////////////////////////////////
// Control flags
#define CTF_NONE        0x00  //Nothing
#define CTF_VAL_ABS     0x01  //Absolute number
#define CTF_VAL_BOOL    0x02  //Value is bool
#define CTF_VAL_DELTA   0x03  //Value is delta
#define CTF_VAL_NEXT    0x04  //Go next - in cycles
#define CTF_VAL_PREV    0x05  //Go previous - in cycles
#define CTF_VAL_STRING  0x06  //Value is string
#define CTF_VAL_OBJECT  0x07  //Value is object

#ifdef WIFI_ENABLED
  #define MAX_STR_VALUE 256
#else
  #define MAX_STR_VALUE 16
#endif



#endif //__ARDUINOLIB_H
