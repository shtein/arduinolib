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

//WiFi status
void putNtfObject(NtfBase &resp, const WIFI_STATUS &data){
  //Current mode
  resp.put_F(F("wifimode"), (uint8_t)WiFi.getMode());   
  
  //Station
  resp.begin_F(F("station"));
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
  resp.end_F(F("station"));

  resp.begin_F(F("ap"));
    //Mac address
    resp.put_F(F("macaddress"), WiFi.softAPmacAddress().c_str());       
    //SSID
    resp.put_F(F("ssid"), WiFi.softAPSSID().c_str());      
    //IP
    resp.put_F(F("ipaddress"), IP_ADDRESS_STR(WiFi.softAPIP()));  
    //Number of connected stations
    resp.put_F(F("stations"), WiFi.softAPgetStationNum());  
  resp.end_F(F("ap"));
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
  resp.put_F(F("staticip"), data.ip == 0);
  resp.put_F(F("ipaddress"), IP_ADDRESS_STR(IPAddress(data.ip)));
  resp.put_F(F("gateway"), IP_ADDRESS_STR(IPAddress(data.gateway)));
  resp.put_F(F("netmask"), IP_ADDRESS_STR(IPAddress(data.subnetMask)));  
  resp.put_F(F("dns1"), IP_ADDRESS_STR(IPAddress(data.dns1)));
  resp.put_F(F("dns2"), IP_ADDRESS_STR(IPAddress(data.dns2)));
}

//String for IP address for parser
bool strToIPAddr(const char *str, uint32_t &u){
  if(!str)
    return false;
  
  u = ipaddr_addr(str);

  return true; 
}


#endif