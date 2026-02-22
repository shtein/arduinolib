#ifndef __ARDUINOLIB_H


#ifdef ARDUINO
#include <Arduino.h>

#if (defined(ESP8266) || defined(ESP32) ) && !defined(NO_WIFI)
    #define WIFI_ENABLED
#endif

#endif






#endif //__ARDUINOLIB_H
