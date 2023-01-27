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
}

void NtfWebApi::beginArray(const char *key){
  PRINT_KEY(key);
  PRINT(AOPB);
}

void NtfWebApi::endArray(const char *key){  
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


#endif
