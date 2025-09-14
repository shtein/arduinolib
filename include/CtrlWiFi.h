#ifndef __CTRL_WIFI_H
#define __CTRL_WIFI_H

#include "arduinolib.h"

#ifdef WIFI_ENABLED

#include "Controls.h"
#include "Notification.h"

#if defined(ESP8266) 
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif


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

//Network configuration
#define MAX_HOST_NAME 33
#define MAX_DATA_SIZE 28

struct WIFI_NETWORK{
  char host[MAX_HOST_NAME];      //Host name
  uint16_t webPort;              //Web port 
  uint8_t spare[MAX_DATA_SIZE];  //Custom data
};


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
  char pwd[PWD_LENGHT];
  uint32_t ipaddress;
  uint32_t gateway;
  uint32_t subnetMask;
};



///////////////////////////////////////
//Main standalone functions
void initWiFi(const char *hostName = NULL, const char *apName = NULL );

void connectWiFi(const WIFI_CONNECT &wcn);
void disconnectWiFi();

void connectWiFiAP(const WIFI_AP_CONNECT &wcn);
void disconnectWiFiAP();

//Config read/write
bool readWiFiNetworkConfig(WIFI_NETWORK &dst);
bool writeWiFiNetworkConfig(const WIFI_NETWORK &src);
bool clearWiFiNetworkConfig();

bool readWiFiConfig(WIFI_CONNECT &dst);
bool writeWiFiConfig(const WIFI_CONNECT &src);
bool clearWiFiConfig();

bool readWiFiAPConfig(WIFI_AP_CONNECT &dst);
bool writeWiFiAPConfig(const WIFI_AP_CONNECT &src);
bool clearWiFiApConfig();





////////////////////////////////
// Notifiations

//Wifi station status
struct WIFI_STATUS_STATION{
};

//WIFI AP status
struct WIFI_STATUS_AP{  
};

//WiFi scan
struct WIFI_SCAN{
};


//Wifi control
#define EEMC_WIFI                 0x40    //Wifi commands

#define EEMC_WIFI_AP_STATUS       0x40    //WIFI status
#define EEMC_WIFI_AP_CONNECT      0x41    //Enable AP
#define EEMC_WIFI_AP_DISCONNECT   0x42    //Disable AP
#define EEMC_WIFI_AP_CFG_GET      0x43    //Get AP configuration
#define EEMC_WIFI_AP_CFG_SET      0x44    //Set AP configuration
#define EEMC_WIFI_AP_CFG_CLEAR    0x45    //Clear AP configuration

#define EEMC_WIFI_STATUS_CHANGE   0x46    //WIFI status change
#define EEMC_WIFI_STATUS          0x47    //WIFI status
#define EEMC_WIFI_SCAN            0x48    //WIFI scan networks
#define EEMC_WIFI_CONNECT         0x49    //Connect WIFI
#define EEMC_WIFI_DISCONNECT      0x4A    //Diconnect WIFI
#define EEMC_WIFI_CFG_GET         0x4B    //Get Station configuration
#define EEMC_WIFI_CFG_SET         0x4C    //Set Station configuration
#define EEMC_WIFI_CFG_CLEAR       0x4D    //Clear Station configuration


DECLARE_PARSE_ROUTINE(parseWiFiCmd)


////////////////////////////////////////////////////////
// Wifi connection with config and auto functions
// Needs to be instanciated 

bool onWiFiCmd(struct CtrlQueueItem &itm, NtfSet &ntf);



////////////////////////////////////
// CtrlWifiStatus - wifi status change
class CtrlWifiStatus: public CtrlItem{
  public:
    CtrlWifiStatus(): CtrlItem(EEMC_WIFI_STATUS_CHANGE, NULL) {
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


#endif //WIFI_ENABLED

#endif //__CTRL_WIFI_H