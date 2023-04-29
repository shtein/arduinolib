#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "CtrlWiFi.h"

#include "DbgTool.h"



void convertFromJson(JsonVariantConst src, IPAddress& dst){  
  dst.fromString((const char *)src);
}

bool convertToJson(const IPAddress& src, JsonVariant dst){
  dst.set(IP_ADDRESS_STR(src));
  return true;
}


//
void convertFromJson(JsonVariantConst src, WIFI_AP_CONNECT& dst){  
}

bool convertToJson(const WIFI_AP_CONNECT& src, JsonVariant dst){
  JsonObject obj = dst.to<JsonObject>();

  return true;
}

//
void convertFromJson(JsonVariantConst src, WIFI_CONNECT& dst){  
  strncpy(dst.ssid, src["ssid"], sizeof(dst.ssid));
  strncpy(dst.pwd, src["pwd"], sizeof(dst.pwd));
  
  dst.ip         = src["ipaddress"];
  dst.gateway    = src["gateway"];
  dst.subnetMask = src["netmask"];
  dst.dns1       = src["dns1"];
  dst.dns2       = src["dns2"];
}

bool convertToJson(const WIFI_CONNECT& src, JsonVariant dst){
  JsonObject obj = dst.to<JsonObject>();

  obj["ssid"]      = src.ssid;
  obj["pwd"]       = src.pwd;
  obj["ipaddress"] = src.ip;
  obj["gateway"]   = src.gateway;
  obj["netmask"]   = src.subnetMask;
  obj["dns1"]      = src.dns1;
  obj["dns2"]      = src.dns2;

  return true;
}

//
void convertFromJson(JsonVariantConst src, WIFI_CONFIG_ALL& dst){  
  
  dst.wifiConnect = src["wifi"];
  dst.wifiAP      = src["ap"];

}

bool convertToJson(const WIFI_CONFIG_ALL& src, JsonVariant dst){
  JsonObject obj = dst.to<JsonObject>();
    
  obj["wifi"] = src.wifiConnect;
  obj["ap"]   = src.wifiAP;

  return true;
}


void writeJson(const char *filename, JsonDocument &jsDoc){
  File file = LittleFS.open(filename, "w");

  if (!file) {
    DBG_OUTLN("Failed to open file for writing");
    return;
  }

  if (serializeJson(jsDoc, file) == 0) {
    DBG_OUTLN("Failed to write to file");
  }

  file.close();
}

void readJson(const char *filename, JsonDocument &jsDoc){
  File file = LittleFS.open(filename, "r");
  if (!file) {
    DBG_OUTLN("Failed to open file for reading");
    return;
  }

  
  DeserializationError error = deserializeJson(jsDoc, file);
  if (error) {
    DBG_OUTLN("Failed to parse file");
  }

  file.close();
}

void setup(){
  DBG_INIT();

  LittleFS.begin();

  StaticJsonDocument<512> jsDoc;

  WIFI_CONFIG_ALL         cfg;
  memset(&cfg, 0, sizeof(cfg));

/*
  strcpy(cfg.wifiConnect.ssid, "my_ssid");
  strcpy(cfg.wifiConnect.pwd, "my_pwd");
  cfg.wifiConnect.ip = (uint32_t)IPAddress(192, 168, 100, 101);
  convertToJson(cfg, jsDoc);
  writeJson("/test.json", jsDoc);
*/

  readJson("/test.json", jsDoc);
  convertFromJson(jsDoc, cfg);


  DBG_OUTLN("Serialized:");
  serializeJsonPretty(jsDoc, Serial);

  DBG_OUTLN("ssid: %s, pwd: %s, ip: %s", cfg.wifiConnect.ssid, cfg.wifiConnect.pwd, IPAddress(cfg.wifiConnect.ip).toString().c_str())

  /*
  memset(&cfg, 0, sizeof(cfg));
  

  
  DBG_OUTLN("ssid: %s, pwd: %s, ip: %s", (const char *)jsDoc["wifi"]["ssid"], (const char *)jsDoc["wifi"]["pwd"], (const char *)jsDoc["wifi"]["ipaddress"]);
  */
}


void loop(){  
}



