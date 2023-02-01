#include "arduinolib.h"

#include <Arduino.h>
#include "AnalogInput.h"
#include "CtrlWebSrv.h"
#include "DbgTool.h"

//Support for ESP8266, ESP32 etc
#if defined(ESP8266) || defined(ESP32)

ESPWebServer webServer;
String       requestApi;

///////////////////////////////////
//Web request API handler
void _APIRequestHandler(const char *uri){
  requestApi = webServer.uri().substring(strlen(uri));

  //Remove /, &, ?
  requestApi.replace('&', ' ');
  requestApi.replace('?', ' ');
  requestApi.replace('/', ' ');

  if(requestApi.length() == 0){
    requestApi = " ";
  }
}

/////////////////////
// WebApiInput
WebApiInput::WebApiInput(){  
}

void WebApiInput::read(){  
  //Handle client
  webServer.handleClient();
}

bool WebApiInput::isReady() const{
  return requestApi.length() != 0;
}

char *WebApiInput::getCommandLine(){
  return requestApi.length() == 0 ? NULL : (char *)requestApi.c_str(); 
}


/////////////////////////////////  
//NtfWebApi - json serialization
const char OPB  = '{';  //opening bracket
const char CLB  = '}';  //opening bracket
const char SEP  = ',';  //separator - comma
const char AOPB = '[';  //array opening bracket
const char ACLB = ']';  //array opening bracket
const char QTM  = '\"'; //quota
const char CLN  = ':';  //colon

#define PRINT_KEY(key) \
  if(key){ \
    _data += QTM; \
    _data += key; \
    _data += QTM; \
    _data += CLN; \
  }

#define PRINT(s) _data += s;

#define PRINT_KEY_VALUE(key, value) \
  PRINT_KEY(key); \
  PRINT(value); \
  PRINT(SEP);

#define PRINT_KEY_VALUE_STRING(key, value) \
  PRINT_KEY(key); \
  PRINT(QTM); \
  PRINT(v); \
  PRINT(QTM); \
  PRINT(SEP);


#define CHECK_TRAILING_COMMA() \
  if(_data[_data.length() - 1] == SEP){ \
    _data.remove(_data.length() - 1); \
  }

void NtfWebApi::reset(){
  _data = emptyString;
}

void NtfWebApi::send(){
  CHECK_TRAILING_COMMA();

  //Send response:
  webServer.send(200, "application/json", _data);

  //Clear request string
  requestApi = emptyString;
}


void NtfWebApi::begin(const char *key){
  PRINT_KEY(key);
  PRINT(OPB);
}

void NtfWebApi::end(const char *){    
  CHECK_TRAILING_COMMA();
  PRINT(CLB);
  PRINT(SEP);
}

void NtfWebApi::beginArray(const char *key){
  PRINT_KEY(key);
  PRINT(AOPB);
}

void NtfWebApi::endArray(const char *){  
  CHECK_TRAILING_COMMA();
  PRINT(ACLB);
}

void NtfWebApi::put(const char *key, uint8_t v){
  PRINT_KEY_VALUE(key, v);
}

void NtfWebApi::put(const char *key, uint16_t v){
  PRINT_KEY_VALUE(key, v);
}

void NtfWebApi::put(const char *key, uint32_t v){
  PRINT_KEY_VALUE(key, v);
}

void NtfWebApi::put(const char *key, int8_t v){
  PRINT_KEY_VALUE(key, v);
}

void NtfWebApi::put(const char *key, int16_t v){  
  PRINT_KEY_VALUE(key, v);
}

void NtfWebApi::put(const char *key, int32_t v){
  PRINT_KEY_VALUE(key, v);
}

void NtfWebApi::put(const char *key, const char *v){  
  PRINT_KEY_VALUE_STRING(key, value);
}


///////////////////////////
// Serialization for notifications

//IP Info
void putNtfObject(NtfBase &resp, const ip_info &info){
  resp.put_F(F("ipaddress"), IPAddress(info.ip).toString().c_str());
  resp.put_F(F("gateway"), IPAddress(info.gw).toString().c_str());
  resp.put_F(F("subnetmask"), IPAddress(info.netmask).toString().c_str());    
}

//WiFi status
void putNtfObject(NtfBase &resp, const WIFI_STATUS &data){
  resp.put_F(F("macaddress"), WiFi.macAddress().c_str());   
  resp.put_F(F("wifimode"), (uint8_t)WiFi.getMode());   

  
  if(WiFi.getMode() & WIFI_STA){
    resp.begin_F(F("sta"));
      //Station mode
      resp.put_F(F("wifistatus"), (uint8_t)WiFi.status());  
      
      if(WiFi.status() == WL_CONNECTED){    
        //SSID
        resp.put_F(F("ssid"), WiFi.SSID().c_str());
        
        //IP 
        struct ip_info ip;
        wifi_get_ip_info(STATION_IF, &ip);
        resp.put_F(F("ipconfig"), ip);

        //DNS
        resp.put_F(F("dns1"), WiFi.dnsIP(0).toString().c_str());
        resp.put_F(F("dns2"), WiFi.dnsIP(1).toString().c_str());
      } 
    resp.end_F(F("sta"));
  }

  if(WiFi.getMode() & WIFI_AP){    
    resp.begin_F(F("AP"));

      //SSID
      resp.put_F(F("ssid"), WiFi.softAPSSID().c_str());

      //IP
      struct ip_info ip;
      wifi_get_ip_info(SOFTAP_IF, &ip);
      resp.put_F(F("ipconfig"), ip);        

    resp.end_F(F("AP"));
  }
  
}

//WiFi scan
void putNtfObject(NtfBase &resp, const WIFI_SCAN &data){
  //Scan result
  int8_t result = WiFi.scanComplete();

  resp.put_F(F("result"), result >= 0 ? 0 : result);    

  //Networks
  if(result > 0){
    resp.beginArray_F(F("networks"));

    for(int i = 0; i < result; i++){
     
      int index = i;

      //Remove duplicates and select a better network
      for(int j = 0; j < result; j++){

        //Same network and better signal
        if( i != j &&
            WiFi.SSID(i) == WiFi.SSID(j) && 
            WiFi.RSSI(i) < WiFi.RSSI(j) ){
                                    
          index = j;
          break;
        }
      }

      //The current network is the best
      if(index == i) {
        resp.begin(NULL);

          resp.put_F(F("ssid"), WiFi.SSID(i).c_str());
          resp.put_F(F("channel"), WiFi.channel(i));
          resp.put_F(F("encryption"), WiFi.encryptionType(i));
          resp.put_F(F("rssi"), WiFi.RSSI(i));
          resp.put_F(F("bssid"), WiFi.BSSIDstr(i).c_str());                    
          resp.put_F(F("hidden"), WiFi.isHidden(i) ? 1 : 0);

        resp.end_F(NULL);
      }
    }
    resp.endArray_F(F("networks"));
  }
}


#endif
