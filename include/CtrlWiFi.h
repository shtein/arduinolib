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

#define SSID_LENGHT 32
#define PWD_LENGHT  64

//Connect to WiFi
struct WIFI_CONNECT{
  char ssid[SSID_LENGHT];
  char pwd[PWD_LENGHT];
  uint32_t ip;
  uint32_t gateway;
  uint32_t subnetMask;
  uint32_t dns1;
  uint32_t dns2;
};

//AP config
struct WIFI_AP_CONNECT{
  char ssid[SSID_LENGHT];
};

//Read wifi config from current wifi
#define IP_CONFIG_STATIC_IP  0x01
#define IP_CONFIG_DNS1_SET   0x02
#define IP_CONFIG_DNS2_SET   0x04

void getWiFiConnect(WIFI_CONNECT &wcn, uint8_t flags);
void setWiFiConnect(const WIFI_CONNECT &wcn, uint8_t &flags);


//Wifi station status
struct WIFI_STATUS_STATION{
};

//WIFI AP status
struct WIFI_STATUS_AP{  
};

//Wifi status - for notification 
struct WIFI_STATUS{
  WIFI_STATUS_STATION station;
  WIFI_STATUS_AP      ap;
};

//WiFi scan
struct WIFI_SCAN{
};

//Use to store wifi configuration
struct WIFI_CONFIG_ALL{
  WIFI_CONNECT    wifiConnect;
  WIFI_AP_CONNECT wifiAP;
};




////////////////////
//Serialization
void putNtfObject(NtfBase &resp, const WIFI_STATUS_STATION &data);
void putNtfObject(NtfBase &resp, const WIFI_STATUS_AP &data);
void putNtfObject(NtfBase &resp, const WIFI_STATUS &data);
void putNtfObject(NtfBase &resp, const WIFI_SCAN &data);
void putNtfObject(NtfBase &resp, const WIFI_CONNECT &data);
void putNtfObject(NtfBase &resp, const WIFI_AP_CONNECT &data);
void putNtfObject(NtfBase &resp, const WIFI_CONFIG_ALL &data);

//IPAddress from string
bool strToIPAddr(const char *, uint32_t &);


#endif

#endif //#ifndef __CTRL_WIFI_H