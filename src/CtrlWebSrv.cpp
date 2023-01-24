#include "arduinolib.h"

#include <Arduino.h>
#include "AnalogInput.h"
#include "CtrlWebSrv.h"
#include "DbgTool.h"

//Support for ESP8266, ESP32 etc
#if defined(ESP8266) || defined(ESP32)


/////////////////////////////////
// API Response
class AsyncResponseAPI: public AsyncAbstractResponse{
public:
  AsyncResponseAPI(){
    _code              = 0;
    _contentType       = "application/json";
    _contentReady      = false;
    //_chunked           = true;
    _sendContentLength = false;
    _filledLength      = 0;
  }

  ~AsyncResponseAPI(){
  }

  bool _sourceValid() const { 
    return true; 
  }

  size_t _fillBuffer(uint8_t *buf, size_t maxLen){
    if(!_contentReady){
      return RESPONSE_TRY_AGAIN;
    }

    size_t len = _content.length()  - _filledLength;
    if(len > maxLen)
      len = maxLen;

    memcpy(buf, _content.c_str(), len);

    _filledLength += len;

    return len;
  }

  void setContent(const String &content){
    _content      = content;
    _contentReady = true;
  }

private:  
  String  _content;
  bool    _contentReady;
  size_t  _filledLength;
};

struct ApiRequestResponse{
  String             requestStr;          //Request string, i.e. part of URL
  bool               requestReady;        //free slot
  AsyncResponseAPI  *response;            //Response to be called setReady
};

#define MAX_API_CALLS 2

class ApiCalls{
public:
  ApiCalls(){
    _lastSelected = MAX_API_CALLS;

    for(int i = 0; i < MAX_API_CALLS; i++){
      _apiCalls[i].response     = NULL;
      _apiCalls[i].requestReady = false;
    }
  }

  ApiRequestResponse *findFreeSlot(){        
    for(size_t i = 0; i < MAX_API_CALLS; i++){
      //Start from last selected            
      ApiRequestResponse *p = &_apiCalls[i];
      //Free slot found
      if( p->requestReady == false){      
        //ok        
        return p;
      }
    }
    //Fre slot not found
    
    return NULL;
  }

  ApiRequestResponse *findAndLockNextReadySlot(){
    //Check if last selected slot is not processed yet
    ApiRequestResponse *p = getLockedSlot();
    if(p && p->requestReady){
      return p;
    }

    size_t start = _lastSelected == MAX_API_CALLS ? 0 : _lastSelected;

    for(size_t i = 0; i < MAX_API_CALLS; i++){
      p = &_apiCalls[(start + i) % MAX_API_CALLS];
      //Free slot found
      if( p->requestReady == true){      
        //ok
        _lastSelected = (start + i) % MAX_API_CALLS;
        return p;
      }
    }
    //Fre slot not found
    _lastSelected = MAX_API_CALLS;

    return NULL;
  }

  ApiRequestResponse *getLockedSlot(){
    //If current slot is set and it is ready then return it
    return _lastSelected == MAX_API_CALLS ? NULL : &_apiCalls[_lastSelected];
  }
  
private:
  ApiRequestResponse _apiCalls[MAX_API_CALLS]; //Pool of requests/responses
  size_t             _lastSelected;
};


ApiCalls apiCalls;

///////////////////////////////////
//Web request API handler
void APIRequestHandler(AsyncWebServerRequest *request){
  //Find free slot for response  
  ApiRequestResponse *p = apiCalls.findFreeSlot();
  if(p){
    p->requestStr = request->url();
    p->response = new AsyncResponseAPI;
    p->requestReady = true;    

    request->send(p->response);
  }
  else{
    request->send(500);
  }
}

/////////////////////
// WebApiInput
WebApiInput::WebApiInput(){  
}

void WebApiInput::read(){  
  //Find next ready request
  apiCalls.findAndLockNextReadySlot();
}

bool WebApiInput::isReady() const{
  return apiCalls.getLockedSlot() ? true : false;
}

char *WebApiInput::getCommandLine(){
  ApiRequestResponse *p = apiCalls.getLockedSlot();  
  
  return p ? (char *)p->requestStr.c_str() : NULL;
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

  ApiRequestResponse *p = apiCalls.getLockedSlot();  

  if(p){
    //Send data out
    p->response->setContent(_data);
    
    //Release slot
    p->requestReady = false;
    p->response     = NULL;
    p->requestStr   = emptyString;
  }
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
