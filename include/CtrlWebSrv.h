#ifndef __CTRL_WEB_SRV_H
#define __CTRL_WEB_SRV_H

#if defined(ESP8266) || defined(ESP32)

#include "AnalogInput.h"
#include "Controls.h"
#include "Notification.h"

#include <ESPAsyncWebServer.h>

#define MAX_API_RESPONSE_SIZE 256

////////////////////////////////////////
// Web Server Input
class WebApiInput: public BaseInput{
public:
  WebApiInput();

  void read();
  bool isReady() const;
  char *getCommandLine();
};


////////////////////////////////////////
// Json serialization for web api responses


class NtfWebApi: public NtfBase{
public:  
  void reset();
  void send(); 
  
  void begin(const char *key = NULL);
  void end(const char *key = NULL);
  void beginArray(const char *key = NULL);
  void endArray(const char *key = NULL);
  
  void put(const char *key, uint8_t v);
  void put(const char *key, uint16_t v);
  void put(const char *key, uint32_t v);
  void put(const char *key, int8_t v);
  void put(const char *key, int16_t v);  
  void put(const char *key, int32_t v);  
  void put(const char *key, const char *v);

  
private:
  String _data;
};

//////////////////////////////////////
// APIRequestHandler
void APIRequestHandler(AsyncWebServerRequest *request);

//////////////////////////////////////
// CtrlWebSrv - multi command interface
template <uint8_t (*PARSER) (char *cmdLine, CtrlQueueData &data)>
using CtrlItemWebApi = CtrlItemMultiCommand<WebApiInput, NtfWebApi, PARSER>;


#endif

#endif //__CTRL_WEB_SRV_H