//Support for ESP8266, ESP32 etc
#if defined(ESP8266) || defined(ESP32)


#include "arduinolib.h"

#include <Arduino.h>
#include "CtrlWifi.h"
#include "Notification.h"
#include "DbgTool.h"


///////////////////////////
// Serialization for notifications

//WiFi status
void putNtfObject(NtfBase &resp, const WIFI_STATUS &data){
  //Current mode
  resp.put_F(F("wifimode"), (uint8_t)WiFi.getMode());   
  
  //Station
  resp.begin_F(F("station"));
    //Mac address
    resp.put_F(F("macaddress"), WiFi.macAddress().c_str());   
    //SSID
    resp.put_F(F("ssid"), WiFi.SSID().c_str());

    //Active 
    if(WiFi.getMode() & WIFI_STA){
        
      //Statis      
      resp.put_F(F("wifistatus"), (uint8_t)WiFi.status());  
      
      if(WiFi.status() == WL_CONNECTED){                    
        //IP 
        resp.put_F(F("ipaddress"), WiFi.localIP().toString().c_str());
        resp.put_F(F("gateway"), WiFi.gatewayIP().toString().c_str());
        resp.put_F(F("netmask"), WiFi.subnetMask().toString().c_str());
        //DNS
        resp.put_F(F("dns1"), WiFi.dnsIP(0).toString().c_str());
        resp.put_F(F("dns2"), WiFi.dnsIP(1).toString().c_str());
      }     
    }
  resp.end_F(F("station"));

  resp.begin_F(F("ap"));
    //Mac address
    resp.put_F(F("macaddress"), WiFi.softAPmacAddress().c_str());   
    //SSID
    resp.put_F(F("ssid"), WiFi.softAPSSID().c_str());
    
    if(WiFi.getMode() & WIFI_AP){    
      //IP
      resp.put_F(F("ipdadress"), WiFi.softAPIP().toString().c_str());  
      resp.put_F(F("stations"), WiFi.softAPgetStationNum());  
    }
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

void putNtfObject(NtfBase &resp, const WIFI_CONFIG &data){
  //resp.put_F(F("staticip"), data.ip.isSet());

  if(data.ip.isSet()){
    resp.put_F(F("ipaddress"), data.ip.toString().c_str());
    resp.put_F(F("gateway"), data.gateway.toString().c_str());
    resp.put_F(F("netmask"), data.subnetMask.toString().c_str());  
  }
  if(data.dns1.isSet()){
    resp.put_F(F("dns1"), data.dns1.toString().c_str());
  }
  if(data.dns2.isSet()){
    resp.put_F(F("dns2"), data.dns2.toString().c_str());
  }
}


#endif