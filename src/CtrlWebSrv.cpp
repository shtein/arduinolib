#include "CtrlWebSrv.h"
#include "DbgTool.h"

#ifdef WIFI_ENABLED

DNSServer    dnsServer;
ESPWebServer webServer;
String       requestApi;

///////////////////////////////////
//Web request API handler
void _APIRequestHandler(const char *uri){
  requestApi = webServer.uri().substring(strlen(uri));

  //Ensure request processing
  if(requestApi.length() == 0){
    requestApi = " ";
  }
}

/////////////////////
// Handle all DNS requests in AP Mode
void enableCaptivePortalDNS(bool enable){
  if(enable){
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());
  }
  else{
    dnsServer.stop();
  }
}

bool handleCaptivePortal(const char *redirect){
  if(WiFi.softAPIP() == webServer.client().localIP()){  
    webServer.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + redirect, true); 
    webServer.send(302, "text/plain", "");       
    return true;
  }

  return false; 
}

/////////////////////
// WebApiInput
WebApiInput::WebApiInput(){  
}

void WebApiInput::read(){  
  //Handle client
  webServer.handleClient();
  //Handle DNS
  dnsServer.processNextRequest();
}

bool WebApiInput::isReady() const{
  return requestApi.length() != 0;
}

bool WebApiInput::getTokens(const char *tokens[], size_t maxTokens){
  //Make sure it is ready
  if(!isReady()){
    //Return true, handle 0 tokens as no command
    return true;
  }


  //Parse API URI
  if(!::getTokens((char *)requestApi.c_str(), tokens, maxTokens, '/', 0x00)){
    return false;
  }

  //Find the last token from URI
  size_t curTokens = 0;
  while(tokens[curTokens] && curTokens < maxTokens){
    curTokens ++;
  }

  //Check if there is enough slots to add parameter tokens
  if(curTokens + webServer.args() * 2 >= maxTokens){
    return false;
  }

  //Add command parameters
  for(int i = 0; i < webServer.args(); i++){
    tokens[curTokens + 2 * i]     = webServer.argName(i).c_str();
    tokens[curTokens + 2 * i + 1] = webServer.arg(i).c_str();
  }


  return true;
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

  //Nothing to process
  if(requestApi.length() == 0){
    return;
  }

  //Send response
  webServer.send(200, "application/json", _data);

  //Clear request data
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

void NtfWebApi::put(const char *key, bool v){
  char str[5];
  strcpy_P(str, v ? PSTR("true"): PSTR("false"));
  
  PRINT_KEY_VALUE(key, str);
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
