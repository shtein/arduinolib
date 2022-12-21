#include "arduinolib.h"

#include <string.h>
#include "esp_s.h"

//Support for ESP8266, ESP32 etc
#if defined(ESP8266) || defined(ESP32)

const char * 	strchr_P (const char *p, int __val){
  char b[2];
  b[0] = (char)__val;
  b[1] = 0;

  return strstr_P(p, b);
}

#endif
