#ifndef __JSON_CFG_H
#define __JSON_CFG_H

#include "arduinolib.h"

#ifdef WIFI_ENABLED
#include <ArduinoJson.h>


///////////////////////////////////////////
// Unlike EEPROMCfg, no class is defined here. Config is in JsonDocument
// Only functions to read/write JSON config file are needed

void readJsonConfig(const char *filename, JsonDocument &jsDoc);
void writeJsonConfig(const char *filename, JsonDocument &jsDoc);

#endif //WIFI_ENABLED

#endif //__JSON_CFG_H