#ifndef __CTRL_WIFI_H
#define __CTRL_WIFI_H

#if defined(ESP8266) || defined(ESP32)

#include "Controls.h"

#if defined(ESP8266) 
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif


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
class NtfBase;

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

//Wifi config
struct WIFI_CONFIG{
  IPAddress ip;
  IPAddress gateway;
  IPAddress subnetMask;
  IPAddress dns1;
  IPAddress dns2;
};

////////////////////
//Serialization
void putNtfObject(NtfBase &resp, const WIFI_STATUS &data);
void putNtfObject(NtfBase &resp, const WIFI_SCAN &data);
void putNtfObject(NtfBase &resp, const WIFI_CONFIG &data);


#endif

#endif //#ifndef __CTRL_WIFI_H