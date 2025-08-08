#include "JSONCfg.h"
#include "DbgTool.h"
#include <LittleFS.h>

void writeJsonConfig(const char *filename, JsonDocument &jsDoc){
  //Open file for writing
  File file = LittleFS.open(filename, "w");

  if (!file) {
    DBG_OUTLN("Failed to open %s file for writing", filename);
    return;
  }

  //Parse the JSON document and write it to the file
  if (serializeJson(jsDoc, file) == 0) {
    DBG_OUTLN("Failed to write to %s file", filename);
  }

  file.close();
}

void readJsonConfig(const char *filename, JsonDocument &jsDoc){
  //Open file for reading
  File file = LittleFS.open(filename, "r");

  if (!file) {
    DBG_OUTLN("Failed to open %s file for reading", filename);
    return;
  }
  
  //Parse the JSON document from the file
  DeserializationError error = deserializeJson(jsDoc, file);
  if (error != DeserializationError::Ok) {
    DBG_OUTLN("Failed to parse %s file - %", filename, error.c_str());
  }

  file.close();
}
