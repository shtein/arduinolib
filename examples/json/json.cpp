#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "CtrlWiFi.h"

#include "DbgTool.h"
#include "JSONCfg.h"

void setup(){
  DBG_INIT();

  LittleFS.begin();

  DynamicJsonDocument jsDoc(512);

  WIFI_CONNECT             wc;
  memset(&wc, 0, sizeof(wc));
/*
  strcpy(cfg.ssid, "my_ssid");
  strcpy(cfg.pwd, "my_pwd");
  cfg.ipaddress = (uint32_t)IPAddress(192, 168, 100, 101);
  jsDoc["wifi"] = cfg;
  writeJsonConfig("/test.json", jsDoc);
*/
  
  readJsonConfig("/test.json", jsDoc);


  DBG_OUTLN("Serialized:");
  serializeJsonPretty(jsDoc, Serial);

  //DBG_OUTLN("ssid: %s, pwd: %s, ip: %s", cfg.wifiConnect.ssid, cfg.wifiConnect.pwd, IPAddress(cfg.wifiConnect.ip).toString().c_str())

  /*
  memset(&cfg, 0, sizeof(cfg));
  

  
  DBG_OUTLN("ssid: %s, pwd: %s, ip: %s", (const char *)jsDoc["wifi"]["ssid"], (const char *)jsDoc["wifi"]["pwd"], (const char *)jsDoc["wifi"]["ipaddress"]);
  */
}


void loop(){  
}



