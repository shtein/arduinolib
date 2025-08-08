#include "JSONCfg.h"
#include "CtrlWiFi.h"
#include "CtrlWebSrv.h"
#include "Notification.h"
#include "DbgTool.h"


#ifdef WIFI_ENABLED

/////////////////////////////////////
// Helper functions

//String for IP address for parser
bool strToIPAddr(const char *str, uint32_t &u){
  if(!str)
    return false;
  
  u = ipaddr_addr(str);

  return true; 
}

String getHostDefaultName(const char* prefix, const uint8_t *mac){

  String str(prefix);
  for(size_t i = 3; i < 6; i++){
    str += String(mac[i], HEX);
  }
  
  return str;
}


//Defalt host name
String getAPDefaultName(const char* prefix){
//Configure name
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);  

  return getHostDefaultName(prefix, mac);
}

//Default AP name
String getSTAHostDefaultName(const char* prefix){
//Configure name
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);

  return getHostDefaultName(prefix, mac);
}


/////////////////////////////////////////
// Main control functions in manual mode

//Init
void initWiFi(const char *hostName, const char *apName){

  //Generic settings
  WiFi.setAutoReconnect(true);
  WiFi.setAutoConnect(false);
  WiFi.persistent(false);

  //Host name can be only set when mode is STA  
  if(hostName){
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(hostName);
  }
  //Turn wifi off after initialization
  WiFi.mode(WIFI_OFF); 

  //Set AP name
  if(apName){
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apName);    
  }
  //Turn wifi off after initialization
  WiFi.mode(WIFI_OFF); 

  WiFi.softAPdisconnect(true);
}


//Connect to Wifi
void connectWiFi(const WIFI_CONNECT &wcn){
  //Confiogure ip settings
  WiFi.config(wcn.ipaddress, wcn.gateway, wcn.subnetMask, wcn.dns1, wcn.dns2);

  //Enable wifi
  WiFi.begin(wcn.ssid, wcn.pwd[0] == 0 ? NULL : wcn.pwd);    
}

//Connect AP
void connectWiFiAP(const WIFI_AP_CONNECT &wcn){
  //Setup IP Address
  WiFi.softAPConfig( wcn.ipaddress, wcn.gateway, wcn.subnetMask);  

  //Enable AP SSID
  WiFi.softAP(wcn.ssid);
}




///////////////////////////
// Serialization for  notifications of wifi data structures

DEFINE_STR_PROGMEM(rs_MacAddr,        "macaddress");
DEFINE_STR_PROGMEM(rs_Host,           "host");
DEFINE_STR_PROGMEM(rs_WiFiStatus,     "wifistatus");
DEFINE_STR_PROGMEM(rs_SSID,           "ssid");
DEFINE_STR_PROGMEM(rs_Pwd,            "pwd");
DEFINE_STR_PROGMEM(rs_IPAddress,      "ipaddress");
DEFINE_STR_PROGMEM(rs_StaticIP,       "staticip");
DEFINE_STR_PROGMEM(rs_Gateway,        "gateway");
DEFINE_STR_PROGMEM(rs_NetMask,        "netmask");
DEFINE_STR_PROGMEM(rs_DNS1,           "dns1");
DEFINE_STR_PROGMEM(rs_DNS2,           "dns2");
DEFINE_STR_PROGMEM(rs_WiFiMode,       "wifimode");
DEFINE_STR_PROGMEM(rs_Station,        "station");
DEFINE_STR_PROGMEM(rs_Stations,       "stations");
DEFINE_STR_PROGMEM(rs_AP,             "ap");
DEFINE_STR_PROGMEM(rs_Network,        "network");
DEFINE_STR_PROGMEM(rs_Networks,       "networks");
DEFINE_STR_PROGMEM(rs_Result,         "result");
DEFINE_STR_PROGMEM(rs_Channel,        "channel");
DEFINE_STR_PROGMEM(rs_Encryption,     "encryption");
DEFINE_STR_PROGMEM(rs_RSSI,           "rssi");
DEFINE_STR_PROGMEM(rs_BSSID,          "bssid");

void putNtfObject(NtfBase &resp, const WIFI_STATUS_STATION &data){
  //Mac address  
  resp.put_F(rs_MacAddr, WiFi.macAddress().c_str()); 
  //Host
  resp.put_F(rs_Host, WiFi.getHostname());         
  //Status      
  resp.put_F(rs_WiFiStatus, (uint8_t)WiFi.status());  
  //SSID
  resp.put_F(rs_SSID, WiFi.SSID().c_str()); 
  //Password
  resp.put_F(rs_Pwd, WiFi.psk().c_str());                        
  //IP 
  resp.put_F(rs_IPAddress, IP_ADDRESS_STR( WiFi.localIP()));
  resp.put_F(rs_Gateway, IP_ADDRESS_STR(WiFi.gatewayIP()));
  resp.put_F(rs_NetMask, IP_ADDRESS_STR(WiFi.subnetMask()));
  //DNS
  resp.put_F(rs_DNS1, IP_ADDRESS_STR(WiFi.dnsIP(0)));
  resp.put_F(rs_DNS2, IP_ADDRESS_STR(WiFi.dnsIP(1)));    
}

void putNtfObject(NtfBase &resp, const WIFI_STATUS_AP &data){
  //Mac address
  resp.put_F(rs_MacAddr, WiFi.softAPmacAddress().c_str());       
  //SSID
  resp.put_F(rs_SSID, WiFi.softAPSSID().c_str());      
  //IP
  resp.put_F(rs_IPAddress, IP_ADDRESS_STR(WiFi.softAPIP())); 
  //Number of connected stations
  resp.put_F(rs_Stations, WiFi.softAPgetStationNum());  
}


//WiFi status
void putNtfObject(NtfBase &resp, const WIFI_STATUS &data){
  //Current mode
  resp.put_F(rs_WiFiMode, (uint8_t)WiFi.getMode()); 
  resp.put_F(rs_Station, data.station);
  resp.put_F(rs_AP, data.ap);    
}


//WiFi scan
void putNtfObject(NtfBase &resp, const WIFI_SCAN &data){
  //Scan result
  int8_t result = WiFi.scanComplete();

  resp.put_F(rs_Result, result >= 0 ? 0 : result);

  //Networks
  if(result > 0){
    resp.beginArray_F(rs_Networks);

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
        resp.begin_F(rs_Network);

          resp.put_F(rs_SSID, WiFi.SSID(i).c_str());
          resp.put_F(rs_Channel, WiFi.channel(i));
          resp.put_F(rs_Encryption, (uint8_t)WiFi.encryptionType(i));
          resp.put_F(rs_RSSI, WiFi.RSSI(i));
          resp.put_F(rs_BSSID, WiFi.BSSIDstr(i).c_str());

        resp.end();
      }
    }
    resp.endArray();
  }
}

void putNtfObject(NtfBase &resp, const WIFI_CONNECT &data){
  resp.put_F(rs_SSID, data.ssid);
  resp.put_F(rs_StaticIP, data.ipaddress == 0 ? false : true);
  resp.put_F(rs_IPAddress, IP_ADDRESS_STR(IPAddress(data.ipaddress)));
  resp.put_F(rs_Gateway, IP_ADDRESS_STR(IPAddress(data.gateway)));
  resp.put_F(rs_NetMask, IP_ADDRESS_STR(IPAddress(data.subnetMask)));  
  resp.put_F(rs_DNS1, IP_ADDRESS_STR(IPAddress(data.dns1)));
  resp.put_F(rs_DNS2, IP_ADDRESS_STR(IPAddress(data.dns2)));
}

void putNtfObject(NtfBase &resp, const WIFI_AP_CONNECT &data){
  resp.put_F(rs_SSID, data.ssid);
}


void putNtfObject(NtfBase &resp, const WIFI_CONFIG_ALL &data){
  resp.put_F(rs_Station, data.wifiConnect);
  resp.put_F(rs_AP, data.wifiAP);
}

////////////////////////////////////////
//Wifi commands and parser

DEFINE_STR_PROGMEM(rs_CmdParam_Wifi,       "wifi|w")
DEFINE_STR_PROGMEM(rs_CmdParam_Status,     "status|")
DEFINE_STR_PROGMEM(rs_CmdParam_AP,         "ap")
DEFINE_STR_PROGMEM(rs_CmdParam_AP_On,      "on")
DEFINE_STR_PROGMEM(rs_CmdParam_AP_Off,     "off")
DEFINE_STR_PROGMEM(rs_CmdParam_Scan,       "scan|s")
DEFINE_STR_PROGMEM(rs_CmdParam_Connect,    "connect|c")
DEFINE_STR_PROGMEM(rs_CmdParam_Host,       "host|h")
DEFINE_STR_PROGMEM(rs_CmdParam_SSID,       "ssid")
DEFINE_STR_PROGMEM(rs_CmdParam_Password,   "pwd")
DEFINE_STR_PROGMEM(rs_CmdParam_Gateway,    "gateway|gw")
DEFINE_STR_PROGMEM(rs_CmdParam_SubnetMask, "subnetmask|sm")
DEFINE_STR_PROGMEM(rs_CmdParam_DNS1,       "dns1")
DEFINE_STR_PROGMEM(rs_CmdParam_DNS2,       "dns2")
DEFINE_STR_PROGMEM(rs_CmdParam_Disconnect, "disconnect|d")
DEFINE_STR_PROGMEM(rs_CmdParam_Config,     "config|cfg")
DEFINE_STR_PROGMEM(rs_CmdParam_Get,        "get|")
DEFINE_STR_PROGMEM(rs_CmdParam_Clear,      "clear")


BEGIN_PARSE_ROUTINE(parseWiFiCmd)
  BEGIN_GROUP_TOKEN(rs_CmdParam_Wifi) 
    VALUE_IS_TOKEN(rs_CmdParam_Status, EEMC_WIFI_STATUS)          //WiFi status

    BEGIN_GROUP_TOKEN(rs_CmdParam_AP)                             //AP control
      VALUE_IS_TOKEN(rs_CmdParam_AP_On, EEMC_WIFI_AP_ON)          //AP off               
      VALUE_IS_TOKEN(rs_CmdParam_AP_Off, EEMC_WIFI_AP_OFF)        //AP off         
    END_GROUP_TOKEN() 

    VALUE_IS_TOKEN(rs_CmdParam_Scan, EEMC_WIFI_SCAN)              //Scan networks

    BEGIN_OBJECT(rs_CmdParam_Connect, WIFI_CONNECT, EEMC_WIFI_CONNECT)  //Connect
      DATA_MEMBER(rs_CmdParam_SSID, ssid)                                  //SSID
      DATA_MEMBER(rs_CmdParam_Password, pwd, "")                           //Password
      DATA_MEMBER_AS_IP(rs_CmdParam_Gateway, gateway, 0)                   //Gateway
      DATA_MEMBER_AS_IP(rs_CmdParam_SubnetMask, subnetMask, 0)             //Subnet mask
      DATA_MEMBER_AS_IP(rs_CmdParam_DNS1, dns1, 0)                         //DNS 1
      DATA_MEMBER_AS_IP(rs_CmdParam_DNS2, dns2, 0)                         //DNS 2
    END_OBJECT()
    VALUE_IS_TOKEN(rs_CmdParam_Disconnect, EEMC_WIFI_DISCONNECT)         //Disconnect

    BEGIN_GROUP_TOKEN(rs_CmdParam_Config)
      VALUE_IS_TOKEN(rs_CmdParam_Get, EEMC_WIFI_CFG_GET)                 //Get config saved after last succesfull connect
      VALUE_IS_TOKEN(rs_CmdParam_Clear, EEMC_WIFI_CFG_CLEAR)             //Clear config for debugging purposes
    END_GROUP_TOKEN()
  
  END_GROUP_TOKEN()
END_PARSE_ROUTINE()


///////////////////////////////////////
// WiFiConnection
WiFiConnection::WiFiConnection(){
  //_cfgFileName[0] = 0;  
}

void WiFiConnection::init(const char *cfgFileName, const char *hostName, const char *apName){
  //Copy config
  strncpy(_cfgFileName, cfgFileName, sizeof(_cfgFileName));

  //Basic initialization
  initWiFi(hostName, apName);

  //Read AP config
  WIFI_AP_CONNECT wcAp;
  readAPConfig(wcAp);

  //Set AP name
  if(wcAp.ssid[0] == 0){
    //Set default AP name
    strncpy(wcAp.ssid, apName, sizeof(wcAp.ssid));
  }

  //Set AP IP address
  if(wcAp.ipaddress == 0){
    //Set default IP address
    wcAp.ipaddress  = IPAddress(172, 16, 25, 25);
    wcAp.gateway    = IPAddress(172, 16, 25, 25);
    wcAp.subnetMask = IPAddress(255, 255, 255, 0);  
  }
  
  //Save ap config
  writeAPConfig(wcAp);
}

bool WiFiConnection::onCmd(struct CtrlQueueItem &itm, NtfSet &ntf){

  switch(itm.cmd){
    case EEMC_WIFI_STATUS: // Retreieve current wifi status
      ntf.put(CmdResponse<WIFI_STATUS>{itm.cmd});      
    break;

    case EEMC_WIFI_STATUS_CHANGE: { //Change of WiFi status 
      if(WiFi.status() == WL_CONNECTED){
        //Save last known configutation
        //writeConfig();
      }

      ntf.put(CmdResponse<WIFI_STATUS_STATION>{itm.cmd});  
    }
    break;

    case EEMC_WIFI_CONNECT:{ //Connect to WiFi network
                      
      WIFI_CONNECT *wc = (WIFI_CONNECT *)itm.data.str;          
      connectWiFi(*wc);
      ntf.put(CmdResponse<>{itm.cmd, EEER_DELAYED});     
    }    
    break;

    case EEMC_WIFI_DISCONNECT: //Disconnect 
      WiFi.disconnect(true);    
      ntf.put(CmdResponse<>{itm.cmd, EEER_DELAYED});        
    break;
    
    case EEMC_WIFI_AP_ON:{ //Connect AP     
      WIFI_AP_CONNECT wcAp;
      readAPConfig(wcAp);
      connectWiFiAP(wcAp);
      
      enableCaptivePortalDNS(true);      

      ntf.put(CmdResponse<WIFI_STATUS_AP>{itm.cmd}); 
    }
    break;

    case EEMC_WIFI_AP_OFF: //Disconnect AP 
      enableCaptivePortalDNS(false);
      WiFi.softAPdisconnect(true);
      ntf.put(CmdResponse<WIFI_STATUS_AP>{itm.cmd});       
    break;
    
    case EEMC_WIFI_SCAN:{
      switch(WiFi.scanComplete()){
        case WIFI_SCAN_FAILED:
          WiFi.scanNetworks(true, true);
          ntf.put(CmdResponse<>{itm.cmd, EEER_DELAYED});    
        break;
        case WIFI_SCAN_RUNNING:
          ntf.put(CmdResponse<>{itm.cmd, EEER_DELAYED});    
        break;
        default:
          ntf.put(CmdResponse<WIFI_SCAN>{itm.cmd});        
          WiFi.scanDelete();
        break;
      }
    }
    break;

    case EEMC_WIFI_CFG_GET: {      
      //Send reponse      
      //CmdResponse<const WIFI_CONFIG_ALL &> resp{itm.cmd, _wcn};
      //ntf.put(resp);              
    }
    break;      

    case EEMC_WIFI_CFG_CLEAR:{
      //memset(&_wcn, 0, sizeof(_wcn));
      //writeConfig();
      //readConfig();

      //Send reponse      
      //CmdResponse<const WIFI_CONFIG_ALL &> resp{itm.cmd, _wcn};
      //ntf.put(resp);              
    }
    break;

    default: 
    return false;
  }

  return true;
}

//Converion from/to JSON
void convertFromJson(JsonVariantConst src, WIFI_AP_CONNECT& dst){
  strncpy(dst.ssid, src[PROGMEM_TO_STR(rs_SSID)] | "", sizeof(dst.ssid));
  dst.ipaddress  = src[PROGMEM_TO_STR(rs_IPAddress)];
  dst.gateway    = src[PROGMEM_TO_STR(rs_Gateway)];
  dst.subnetMask = src[PROGMEM_TO_STR(rs_NetMask)];
}

bool convertToJson(const WIFI_AP_CONNECT& src, JsonVariant dst){
  JsonObject obj = dst.to<JsonObject>();

  obj[PROGMEM_TO_STR(rs_SSID)]      = src.ssid; 
  obj[PROGMEM_TO_STR(rs_IPAddress)] = IP_ADDRESS_STR_U32(src.ipaddress);
  obj[PROGMEM_TO_STR(rs_Gateway)]   = IP_ADDRESS_STR_U32(src.gateway);
  obj[PROGMEM_TO_STR(rs_NetMask)]   = IP_ADDRESS_STR_U32(src.subnetMask);

  return true;
}

void WiFiConnection::readAPConfig(WIFI_AP_CONNECT &dst){  
  //Read from file
  StaticJsonDocument<512> doc;  
  readJsonConfig(_cfgFileName, doc);

  JsonVariantConst obj = doc[PROGMEM_TO_STR(rs_AP)];
  dst = obj.as<WIFI_AP_CONNECT>();

  serializeJsonPretty(doc, Serial);

  
}

void WiFiConnection::writeAPConfig(const WIFI_AP_CONNECT &src){
  StaticJsonDocument<512> doc;  
  readJsonConfig(_cfgFileName, doc);

  doc[PROGMEM_TO_STR(rs_AP)] = src;
  writeJsonConfig(_cfgFileName, doc);
}


void convertFromJson(JsonVariantConst src, WIFI_CONNECT& dst){
  strncpy(dst.ssid, src[PROGMEM_TO_STR(rs_SSID)], sizeof(dst.ssid));
  strncpy(dst.pwd, src[PROGMEM_TO_STR(rs_Pwd)], sizeof(dst.pwd));
  
  dst.ipaddress  = src[PROGMEM_TO_STR(rs_IPAddress)];
  dst.gateway    = src[PROGMEM_TO_STR(rs_Gateway)];
  dst.subnetMask = src[PROGMEM_TO_STR(rs_NetMask)];
  dst.dns1       = src[PROGMEM_TO_STR(rs_DNS1)];
  dst.dns2       = src[PROGMEM_TO_STR(rs_DNS2)];
}


bool convertToJson(const WIFI_CONNECT& src, JsonVariant dst){
  JsonObject obj = dst.to<JsonObject>();

  obj[PROGMEM_TO_STR(rs_SSID)]      = src.ssid;
  obj[PROGMEM_TO_STR(rs_Pwd)]       = src.pwd;
  obj[PROGMEM_TO_STR(rs_IPAddress)] = IP_ADDRESS_STR_U32(src.ipaddress);
  obj[PROGMEM_TO_STR(rs_Gateway)]   = IP_ADDRESS_STR_U32(src.gateway);
  obj[PROGMEM_TO_STR(rs_NetMask)]   = IP_ADDRESS_STR_U32(src.subnetMask);
  obj[PROGMEM_TO_STR(rs_DNS1)]      = IP_ADDRESS_STR_U32(src.dns1);
  obj[PROGMEM_TO_STR(rs_DNS2)]      = IP_ADDRESS_STR_U32(src.dns2);

  return true;
}


void readWifiConfig(WIFI_CONNECT &dst){
}

void writeWifiConfig(const WIFI_CONNECT &src){
}




#endif