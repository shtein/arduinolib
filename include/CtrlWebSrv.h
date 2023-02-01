#ifndef __CTRL_WEB_SRV_H
#define __CTRL_WEB_SRV_H

#if defined(ESP8266) || defined(ESP32)

#include "AnalogInput.h"
#include "Controls.h"
#include "Notification.h"

#if defined(ESP8266) 
  #include <ESP8266WebServer.h>
  #include <uri/UriRegex.h>
  
  typedef ESP8266WebServer ESPWebServer;
#elif defined(ESP32)
  #include <WebServer.h>
  #include <uri/UriRegex.h>

  typedef WebServer ESPWebServer;
#endif

extern ESPWebServer webServer;

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
void _APIRequestHandler(const char *uri);

#define ADD_API_REQUEST_HANDLER(method, uri) \
  webServer.on(UriRegex(uri".*"), method, [](){ \
      _APIRequestHandler(uri); \
    });

//////////////////////////////////////
// CtrlWebSrv - multi command interface
template <uint8_t (*PARSER) (char *cmdLine, CtrlQueueData &data)>
using CtrlItemWebApi = CtrlItemMultiCommand<WebApiInput, NtfWebApi, PARSER>;

////////////////////////////////////
// CtrlWifiStatus - wifi status change
class CtrlWifiStatus: public CtrlItem{
  public:
    CtrlWifiStatus(uint8_t cmd): CtrlItem(cmd, NULL) {
      _status = WiFi.status();        
    }

  protected:  
    bool triggered() const{     
      return _status != WiFi.status();
    }

    void getData(CtrlQueueData &data){                       
      _status = WiFi.status();
      data.setValue(_status);            
    }    

  private:
    wl_status_t  _status;    
};

/////////////////////////////
// Wifi configs and status

//Connect to WiFi
struct WIFI_CONNECT{
  char ssid[32];
  char pwd[32];
};


//Wifi status - for notification 
struct WIFI_STATUS{
};

//WiFi scan
struct WIFI_SCAN{
};


void putNtfObject(NtfBase &resp, const ip_info &info);
void putNtfObject(NtfBase &resp, const WIFI_STATUS &data);
void putNtfObject(NtfBase &resp, const WIFI_SCAN &data);

//Wifi config
struct WIFI_CONFIG{
  IPAddress ip;
  IPAddress gateway;
  IPAddress subnetMask;
  IPAddress dns1;
  IPAddress dns2;
};



#endif

#endif //__CTRL_WEB_SRV_H