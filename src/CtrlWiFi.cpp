//Support for ESP8266, ESP32 etc
#if defined(ESP8266) || defined(ESP32)


#include "arduinolib.h"

#include <Arduino.h>
#include "CtrlWifi.h"
#include "Notification.h"
#include "DbgTool.h"


///////////////////////////
// Serialization for notifications

#define IP_ADDRESS_STR(ip) (ip.isSet() ? ip.toString().c_str() : "")

void putNtfObject(NtfBase &resp, const WIFI_STATUS_STATION &data){
  //Mac address
  resp.put_F(F("macaddress"), WiFi.macAddress().c_str());         
  //Status      
  resp.put_F(F("wifistatus"), (uint8_t)WiFi.status());  
  //SSID
  resp.put_F(F("ssid"), WiFi.SSID().c_str());                             
  //IP 
  resp.put_F(F("ipaddress"), IP_ADDRESS_STR( WiFi.localIP()));
  resp.put_F(F("gateway"), IP_ADDRESS_STR(WiFi.gatewayIP()));
  resp.put_F(F("netmask"), IP_ADDRESS_STR(WiFi.subnetMask()));
  //DNS
  resp.put_F(F("dns1"), IP_ADDRESS_STR(WiFi.dnsIP(0)));
  resp.put_F(F("dns2"), IP_ADDRESS_STR(WiFi.dnsIP(1)));    
}

void putNtfObject(NtfBase &resp, const WIFI_STATUS_AP &data){
  //Mac address
  resp.put_F(F("macaddress"), WiFi.softAPmacAddress().c_str());       
  //SSID
  resp.put_F(F("ssid"), WiFi.softAPSSID().c_str());      
  //IP
  resp.put_F(F("ipaddress"), IP_ADDRESS_STR(WiFi.softAPIP()));  
  //Number of connected stations
  resp.put_F(F("stations"), WiFi.softAPgetStationNum());  
}


//WiFi status
void putNtfObject(NtfBase &resp, const WIFI_STATUS &data){
  //Current mode
  resp.put_F(F("wifimode"), (uint8_t)WiFi.getMode()); 
  resp.put_F(F("station"), data.station);
  resp.put_F(F("ap"), data.ap);    
}


//WiFi scan
void putNtfObject(NtfBase &resp, const WIFI_SCAN &data){
  //Scan result
  int8_t result = WiFi.scanComplete();

  resp.put_F(F("result"), result >= 0 ? 0 : result);    

  //Networks
  if(result > 0){
    resp.beginArray_F(F("networks"));

    for(int i = 0; i < result; i++){
     
      int index = i;

      //Remove duplicates and select a better network
      for(int j = 0; j < result; j++){

        //Same network and better signal
        if( i != j &&
            WiFi.SSID(i) == WiFi.SSID(j) && 
            WiFi.RSSI(i) < WiFi.RSSI(j) ){
                                    
          index = j;
          break;
        }
      }

      //The current network is the best
      if(index == i) {
        resp.begin(NULL);

          resp.put_F(F("ssid"), WiFi.SSID(i).c_str());
          resp.put_F(F("channel"), WiFi.channel(i));
          resp.put_F(F("encryption"), (uint8_t)WiFi.encryptionType(i));
          resp.put_F(F("rssi"), WiFi.RSSI(i));
          resp.put_F(F("bssid"), WiFi.BSSIDstr(i).c_str()); 

        resp.end_F(NULL);
      }
    }
    resp.endArray_F(F("networks"));
  }
}

void putNtfObject(NtfBase &resp, const WIFI_CONNECT &data){
  resp.put_F(F("ssid"), data.ssid);
  resp.put_F(F("staticip"), data.ip == 0 ? false : true);
  resp.put_F(F("ipaddress"), IP_ADDRESS_STR(IPAddress(data.ip)));
  resp.put_F(F("gateway"), IP_ADDRESS_STR(IPAddress(data.gateway)));
  resp.put_F(F("netmask"), IP_ADDRESS_STR(IPAddress(data.subnetMask)));  
  resp.put_F(F("dns1"), IP_ADDRESS_STR(IPAddress(data.dns1)));
  resp.put_F(F("dns2"), IP_ADDRESS_STR(IPAddress(data.dns2)));
}

void putNtfObject(NtfBase &resp, const WIFI_AP_CONNECT &wap){
  resp.put_F(F("ssid"), wap.ssid);
}


void putNtfObject(NtfBase &resp, const WIFI_CONFIG_ALL &wcn){
  resp.put_F(F("station"), wcn.wifiConnect);
  resp.put_F(F("ap"), wcn.wifiAP);
}


//String for IP address for parser
bool strToIPAddr(const char *str, uint32_t &u){
  if(!str)
    return false;
  
  u = ipaddr_addr(str);

  return true; 
}

//Retrive information abut WiFi
void getWiFiConnect(WIFI_CONNECT &wcn, uint8_t flags){
  memset(&wcn, 0, sizeof(wcn));

  //Remeber ssid and password
  strncpy(wcn.ssid, WiFi.SSID().c_str(), sizeof(wcn.ssid) - 1);
  strncpy(wcn.pwd, WiFi.psk().c_str(), sizeof(wcn.pwd) - 1);
  
  //Remember ip config
  if(flags & IP_CONFIG_STATIC_IP){      
    wcn.ip         = WiFi.localIP();
    wcn.gateway    = WiFi.gatewayIP();
    wcn.subnetMask = WiFi.subnetMask();
    if(flags & IP_CONFIG_DNS1_SET){
      wcn.dns1       =  WiFi.dnsIP(0);
    }
    if(flags & IP_CONFIG_DNS2_SET){
      wcn.dns2       =  WiFi.dnsIP(1);
    }
  }
}

//Connect to Wifi
void setWiFiConnect(const WIFI_CONNECT &wcn, uint8_t &flags){
  //Configure wifi settings
  WiFi.setAutoReconnect(true);
  WiFi.setAutoConnect(true);
  WiFi.persistent(false);
  
  //Confiogure ip settings
  WiFi.config(wcn.ip, wcn.gateway, wcn.subnetMask, wcn.dns1, wcn.dns2);

  //Enable wifi
  WiFi.begin(wcn.ssid, wcn.pwd[0] == 0 ? NULL : wcn.pwd);  

  //Configuration flags
  flags = 0;

  if(wcn.ip != 0){
    flags |= IP_CONFIG_STATIC_IP;

    if(wcn.dns1 != 0){
      flags |= IP_CONFIG_DNS1_SET;
    }

    if(wcn.dns2 != 0){
      flags |= IP_CONFIG_DNS2_SET;
    }
  }
}

void getWiFiAPConnect(WIFI_AP_CONNECT &wcn){
  //Copy SSID
  strcpy(wcn.ssid, WiFi.softAPSSID().c_str());
}

void setWiFiAPConnect(const WIFI_AP_CONNECT &wcn){
  //Setup IP Address
  WiFi.softAPConfig( IPAddress(172, 16, 25, 25),
                     IPAddress(172, 16, 25, 25), 
                     IPAddress(255, 255, 255, 0) 
                    );   
  //Enable AP SSID
  WiFi.softAP(wcn.ssid);
}



  

#endif