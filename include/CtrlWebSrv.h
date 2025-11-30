#ifndef __CTRL_WEB_SRV_H
#define __CTRL_WEB_SRV_H

#include "arduinolib.h"

#ifdef WIFI_ENABLED

#include "AnalogInput.h"
#include "Controls.h"
#include "Notification.h"
#include <LittleFS.h>

#if defined(ESP8266) 
  #include <ESP8266WebServer.h>
  #include <uri/UriRegex.h>
  
  typedef ESP8266WebServer ESPWebServer;
#elif defined(ESP32)
  #include <WebServer.h>
  #include <uri/UriRegex.h>

  typedef WebServer ESPWebServer;
#endif

#include <DNSServer.h> 

extern ESPWebServer webServer;
extern DNSServer dnsServer;

////////////////////////////////////////
// Web Server Input
class WebApiInput: public BaseInput{
public:
  WebApiInput();

  void read();
  bool isReady() const;
  bool getTokens(const char *tokens[], size_t maxTokens);
};


////////////////////////////////////////
// Json serialization for web api responses
class NtfWebApi: public NtfBase{
public:  
  void reset();
  void send(); 
  
  void begin(const char *key = NULL);
  void end();
  void beginArray(const char *key = NULL);
  void endArray();
  
  void put(const char *key, bool v);
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
void _APIRequestHandler(const char *uri);

#define ADD_API_REQUEST_HANDLER(method, uri) \
  webServer.on(UriRegex("^(" uri ").*"), method, [](){ \
      _APIRequestHandler(uri); \
    });

//////////////////////////////////////
// Captive portal functions

//Enable on DNS
void enableCaptivePortalDNS(bool enable);
//Handle captive portal in not found request
bool handleCaptivePortal(const char *redirect);

//////////////////////////////////////
// CtrlWebSrv - multi command interface
template <uint8_t (*PARSER) (const char *tokens[], CtrlQueueData &data)>
using CtrlItemWebApi = CtrlItemMultiCommand<WebApiInput, NtfWebApi, PARSER>;

#endif

#endif //__CTRL_WEB_SRV_H