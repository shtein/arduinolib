#ifndef __CTRL_WIFI_H
#define __CTRL_WIFI_H

#include "arduinolib.h"

#ifdef WIFI_ENABLED

#include "Controls.h"
#include "Notification.h"
#include <ArduinoJson.h>

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
// Helpers
#define IP_ADDRESS_STR(ip) (ip.isSet() ? ip.toString().c_str() : "")
#define IP_ADDRESS_STR_U32(ip) IP_ADDRESS_STR(IPAddress(ip))

//IPAddress from string
bool strToIPAddr(const char *, uint32_t &);

//Dfault host names
String getAPDefaultName(const char *prefix);
String getSTAHostDefaultName(const char *prefix);

#define DEFAULT_HOST(prefix) getSTAHostDefaultName(prefix).c_str()
#define DEFAULT_AP(prefix) getAPDefaultName(prefix).c_str()

/////////////////////////////
// Wifi configs and status

class NtfBase;

#define SSID_LENGHT 33
#define PWD_LENGHT  65


//Connect to WiFi
struct WIFI_CONNECT{
  char ssid[SSID_LENGHT];
  char pwd[PWD_LENGHT];
  uint32_t ipaddress;
  uint32_t gateway;
  uint32_t subnetMask;
  uint32_t dns1;
  uint32_t dns2;
};

//AP 
struct WIFI_AP_CONNECT{
  char ssid[SSID_LENGHT];
  uint32_t ipaddress;
  uint32_t gateway;
  uint32_t subnetMask;
};

//Use to store wifi configuration
struct WIFI_CONFIG_ALL{
  WIFI_CONNECT    wifiConnect;
  WIFI_AP_CONNECT wifiAP;
};


///////////////////////////////////////
//Main standalone functions
void initWiFi(const char *hostName, const char *apName);
void connectWiFi(const WIFI_CONNECT &wcn);
void connectWiFiAP(const WIFI_AP_CONNECT &wcn);

////////////////////////////////
// Notifiations

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




//Wifi control
#define EEMC_WIFI                 0x40    //Wifi commands
#define EEMC_WIFI_STATUS          0x40    //WIFI status
#define EEMC_WIFI_STATUS_CHANGE   0x41    //WIFI status
#define EEMC_WIFI_SCAN            0x42    //WIFI scan networks
#define EEMC_WIFI_AP_ON           0x43    //Enable AP
#define EEMC_WIFI_AP_OFF          0x44    //Disable AP
#define EEMC_WIFI_CONNECT         0x45    //Connect WIFI
#define EEMC_WIFI_DISCONNECT      0x46    //Diconnect WIFI
#define EEMC_WIFI_CFG_GET         0x47    //Get WIFI configuration
#define EEMC_WIFI_CFG_CLEAR       0x48    //Clear config

DECLARE_PARSE_ROUTINE(parseWiFiCmd)


////////////////////////////////////////////////////////
// Wifi connection with config and auto functions
// Needs to be instanciated 

#define CFG_FILE_NAME_MAX 32

class WiFiConnection{
  public:
    WiFiConnection();

    void init(const char *cfgFileName, const char *hostName, const char *apName);
    bool onCmd(struct CtrlQueueItem &itm, NtfSet &ntf);

  protected:
    void readAPConfig(WIFI_AP_CONNECT &dst);
    void writeAPConfig(const WIFI_AP_CONNECT &src);

    void readWifiConfig(WIFI_CONNECT &dst);
    void writeWifiConfig(const WIFI_CONNECT &src);
    
  private:
    char _cfgFileName[CFG_FILE_NAME_MAX];  //Name of config file    
}; 


//Configuration in JSON file - and all nested structure
void convertFromJson(JsonVariantConst src, WIFI_CONNECT& dst);
bool convertToJson(const WIFI_CONNECT& src, JsonVariant dst);



#endif //WIFI_ENABLED

#endif //__CTRL_WIFI_H