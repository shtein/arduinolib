#include "CtrlWiFi.h"
#include "CtrlWebSrv.h"
#include "Notification.h"
#include "DbgTool.h"
#include "EEPROMCfg.h"


#ifdef WIFI_ENABLED


//////////////////////////////////
// WiFi configuration 

// Stored configuration:
//  - WIFI_CONFIG_VERSION
//  - WIFI_NETWORK 
//  - WIFI_CONNECT
//  - WIFI_AP_CONNECT
//  - WIFI_CUSOM_CONNIG

// WiFi configuration version
#define WIFI_CFG_VERSION_HIGH 0x01
#define WIFI_CFG_UNSET        0xFF

struct WIFI_CONFIG_VERSION{
  uint8_t high; //High version
  uint8_t low;  //Low version
};


//Read WiFi configuration
#define WIFI_CONFIZG_SIZE          (sizeof(WIFI_CONFIG_VERSION) + sizeof(WIFI_NETWORK) + sizeof(WIFI_CONNECT) + sizeof(WIFI_AP_CONNECT)) 
#define WIFI_CONFIG_OFFSET         1024
#define WIFI_CONFIG_VERSION_OFFSET (WIFI_CONFIG_OFFSET)
#define WIFI_NETWORK_OFFSET        (WIFI_CONFIG_VERSION_OFFSET + sizeof(WIFI_CONFIG_VERSION))
#define WIFI_CONNECT_OFFSET        (WIFI_NETWORK_OFFSET + sizeof(WIFI_NETWORK))
#define WIFI_AP_CONNECT_OFFSET     (WIFI_CONNECT_OFFSET + sizeof(WIFI_CONNECT))


bool readWifiConfigVersion(WIFI_CONFIG_VERSION &ver){
  //Read WiFi configuration version
  EEPROMCfg ee(WIFI_CONFIG_VERSION_OFFSET);
  ee >> ver;

  return true;
}



bool clearWifiConfigVersion(){
  //Clear WiFi configuration version
  EEPROMCfg ee(WIFI_CONFIG_VERSION_OFFSET);
  
  WIFI_CONFIG_VERSION ver = {WIFI_CFG_UNSET, 0};
  ee << ver;

  return true;
} 

bool readWiFiNetworkConfig(WIFI_NETWORK &dst){
  //Read network configuration
  EEPROMCfg ee(WIFI_NETWORK_OFFSET);  
  ee >> dst;

  return true;
}

bool writeWiFiNetworkConfig(const WIFI_NETWORK &src){
  //Write network configuration
  EEPROMCfg ee(WIFI_NETWORK_OFFSET);  
  ee << src;

  return true;
}

bool clearWiFiNetworkConfig(){
  //Clear network configuration
  EEPROMCfg ee(WIFI_NETWORK_OFFSET);  

  WIFI_NETWORK wcn; 
  memset(&wcn, 0, sizeof(wcn)); 
  ee << wcn;

  return true;
} 


bool readWiFiConfig(WIFI_CONNECT &dst){
  //Read connect configuration
  EEPROMCfg ee(WIFI_CONNECT_OFFSET);  
  ee >> dst;

  return true;
}

bool writeWiFiConfig(const WIFI_CONNECT &src){
  //Write connect configuration
  EEPROMCfg ee(WIFI_CONNECT_OFFSET);  
  ee << src;  

  return true;
}

bool clearWiFiConfig(){
  //Clear connect configuration
  EEPROMCfg ee(WIFI_CONNECT_OFFSET);  

  WIFI_CONNECT wc;
  memset(&wc, 0, sizeof(wc));
  ee << wc;

  return true;
} 

bool readWiFiAPConfig(WIFI_AP_CONNECT &dst){
  //Read AP configuration
  EEPROMCfg ee(WIFI_AP_CONNECT_OFFSET);  
  ee >> dst;


  //Default ip address
  if(dst.ipaddress == 0){
    dst.ipaddress  = IPAddress(172, 16, 25, 25);
    dst.gateway    = IPAddress(172, 16, 25, 25);
    dst.subnetMask = IPAddress(255, 255, 255, 0);
  }

  return true;
}

bool writeWiFiAPConfig(const WIFI_AP_CONNECT &src){
  //Write AP configuration

  EEPROMCfg ee(WIFI_AP_CONNECT_OFFSET);  
  ee << src;


  return true;
}

bool clearWiFiApConfig(){
  //Clear AP configuration
  EEPROMCfg ee(WIFI_AP_CONNECT_OFFSET);  

  WIFI_AP_CONNECT wca;
  memset(&wca, 0, sizeof(wca));
  ee << wca;

  return true;
} 



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

  str.toUpperCase();
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

void checkWifiConfigVersion(){
  //Check if EEPROM is initialized
  WIFI_CONFIG_VERSION ver;
  readWifiConfigVersion(ver);

  if(ver.high == WIFI_CFG_UNSET || ver.high < WIFI_CFG_VERSION_HIGH){
    clearWifiConfigVersion(); //Clear old config
    clearWiFiNetworkConfig(); //Clear network config
    clearWiFiConfig();        //Clear connect config
    clearWiFiApConfig();      //Clear AP config  
  }
}

void checkWifiConfigAP(const char *apName){
  //WiFi AP
  WIFI_AP_CONNECT wca;
  readWiFiAPConfig(wca);

  if(wca.ssid[0] == 0 && apName){
    //Set default AP name
    strncpy(wca.ssid, apName, SSID_LENGHT);
    writeWiFiAPConfig(wca);
  }

  //Set AP name  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(wca.ssid[0] ? wca.ssid : NULL);    
  //Turn wifi off after initialization
  WiFi.mode(WIFI_OFF); 
  WiFi.softAPdisconnect(true);
}

void checkWifiConfig(const char *hostName){
  //Wifi Network 
  WIFI_NETWORK wn;
  readWiFiNetworkConfig(wn);

  if(wn.host[0] == 0 && hostName){      
    //Set default host name from parameter
    strncpy(wn.host, hostName, MAX_HOST_NAME);
    writeWiFiNetworkConfig(wn);
  }

  WiFi.mode(WIFI_STA);
  WiFi.setHostname(wn.host[0] == 0 ? NULL : wn.host); //Set host name

  WiFi.setAutoReconnect(true);
  WiFi.setAutoConnect(false);
  WiFi.persistent(false);

  //Turn wifi off after initialization
  WiFi.mode(WIFI_OFF); 


  //Connect to WiFi if configured
  WIFI_CONNECT wcn;
  readWiFiConfig(wcn);
  connectWiFi(wcn);
}

//Init all
void initWiFi(const char *hostName, const char *apName){
  checkWifiConfigVersion();
  checkWifiConfigAP(apName ? apName : WiFi.softAPSSID().c_str());
  checkWifiConfig(hostName ? hostName : apName ? apName :  WiFi.softAPSSID().c_str());
}


//Connect to Wifi
void connectWiFi(const WIFI_CONNECT &wcn){
  //Confiogure ip settings
  WiFi.config(wcn.ipaddress, wcn.gateway, wcn.subnetMask, wcn.dns1, wcn.dns2);

  //Enable wifi
  WiFi.begin(wcn.ssid, wcn.pwd[0] == 0 ? NULL : wcn.pwd);    
}

void disconnectWiFi(){
  WiFi.disconnect(true);    
}

//Connect AP
void connectWiFiAP(const WIFI_AP_CONNECT &wcn){
  //Setup IP Address
  WiFi.softAPConfig( wcn.ipaddress, wcn.gateway, wcn.subnetMask);  

  //Enable AP SSID
  WiFi.softAP(wcn.ssid[0] == 0 ? NULL : wcn.ssid, wcn.pwd[0] == 0 ? NULL : wcn.pwd);

  //Enable captive portal
  enableCaptivePortalDNS(true); 
}


void disconnectWiFiAP(){
  //Disable captive portal
  enableCaptivePortalDNS(false);

  //Disable AP
  WiFi.softAPdisconnect(true);
}

///////////////////////////
// Serialization for  notifications of wifi data structures

DEFINE_STR_PROGMEM(rs_MacAddr,        "macaddress");
DEFINE_STR_PROGMEM(rs_Host,           "host");
DEFINE_STR_PROGMEM(rs_WebPort,        "webport");
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
  //Mode
  resp.put_F(rs_WiFiMode, (uint8_t)WiFi.getMode());
  //Mac address  
  resp.put_F(rs_MacAddr, WiFi.macAddress().c_str()); 
  //Host name
  resp.put_F(rs_Host, WiFi.getHostname() ? WiFi.getHostname() : "");
  //Status      
  resp.put_F(rs_WiFiStatus, (uint8_t)WiFi.status());  
  //SSID
  resp.put_F(rs_SSID, WiFi.SSID().c_str()); 
  //Password
  //resp.put_F(rs_Pwd, WiFi.psk().c_str());                        
  //IP 
  resp.put_F(rs_IPAddress, IP_ADDRESS_STR( WiFi.localIP()));
  resp.put_F(rs_Gateway, IP_ADDRESS_STR(WiFi.gatewayIP()));
  resp.put_F(rs_NetMask, IP_ADDRESS_STR(WiFi.subnetMask()));
  //DNS
  resp.put_F(rs_DNS1, IP_ADDRESS_STR(WiFi.dnsIP(0)));
  resp.put_F(rs_DNS2, IP_ADDRESS_STR(WiFi.dnsIP(1)));    
}

void putNtfObject(NtfBase &resp, const WIFI_STATUS_AP &data){
  //Mode
  resp.put_F(rs_WiFiMode, (uint8_t)WiFi.getMode()); 
  //Mac address
  resp.put_F(rs_MacAddr, WiFi.softAPmacAddress().c_str());       
  //SSID
  resp.put_F(rs_SSID, WiFi.softAPSSID().c_str());  
  //Password
  //resp.put_F(rs_Pwd, WiFi.softAPPSK().c_str());      
  //IP
  resp.put_F(rs_IPAddress, IP_ADDRESS_STR(WiFi.softAPIP())); 
  //Number of connected stations
  resp.put_F(rs_Stations, WiFi.softAPgetStationNum());  
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
  resp.put_F(rs_Pwd, data.pwd);
  resp.put_F(rs_IPAddress, IP_ADDRESS_STR(IPAddress(data.ipaddress)));
  resp.put_F(rs_Gateway, IP_ADDRESS_STR(IPAddress(data.gateway)));
  resp.put_F(rs_NetMask, IP_ADDRESS_STR(IPAddress(data.subnetMask)));
}


void putNtfObject(NtfBase &resp, const WIFI_NETWORK &data){
  resp.put_F(rs_Host, data.host);
  resp.put_F(rs_WebPort, data.webPort);
}



////////////////////////////////////////
//Wifi commands and parser

DEFINE_STR_PROGMEM(rs_CmdParam_WiFi,       "wifi|w")

DEFINE_STR_PROGMEM(rs_CmdParam_Status,     "status|")

DEFINE_STR_PROGMEM(rs_CmdParam_Network,    "network|n")
DEFINE_STR_PROGMEM(rs_CmdParam_Host,       "host|h")
DEFINE_STR_PROGMEM(rs_CmdParam_WepPort,    "webport|wp")

DEFINE_STR_PROGMEM(rs_CmdParam_Config,     "config|cfg")
DEFINE_STR_PROGMEM(rs_CmdParam_Get,        "get|")
DEFINE_STR_PROGMEM(rs_CmdParam_Set,        "set")
DEFINE_STR_PROGMEM(rs_CmdParam_Clear,      "clear")

DEFINE_STR_PROGMEM(rs_CmdParam_Connect,    "connect|c")
DEFINE_STR_PROGMEM(rs_CmdParam_Disconnect, "disconnect|d")

DEFINE_STR_PROGMEM(rs_CmdParam_AP,         "ap")

DEFINE_STR_PROGMEM(rs_CmdParam_Station,    "station|st")

DEFINE_STR_PROGMEM(rs_CmdParam_Scan,       "scan|s")

DEFINE_STR_PROGMEM(rs_CmdParam_SSID,       "ssid")
DEFINE_STR_PROGMEM(rs_CmdParam_Password,   "pwd")

DEFINE_STR_PROGMEM(rs_CmdParam_IP,         "ip")
DEFINE_STR_PROGMEM(rs_CmdParam_Gateway,    "gateway|gw")
DEFINE_STR_PROGMEM(rs_CmdParam_SubnetMask, "subnetmask|sm")
DEFINE_STR_PROGMEM(rs_CmdParam_DNS1,       "dns1")
DEFINE_STR_PROGMEM(rs_CmdParam_DNS2,       "dns2")


BEGIN_PARSE_ROUTINE(parseWiFiCmd)
  BEGIN_GROUP_TOKEN(rs_CmdParam_WiFi) 
    
  
    BEGIN_GROUP_TOKEN(rs_CmdParam_AP)                             //AP control
      VALUE_IS_TOKEN(rs_CmdParam_Status, EEMC_WIFI_AP_STATUS)            //Get AP status
      VALUE_IS_TOKEN(rs_CmdParam_Connect, EEMC_WIFI_AP_CONNECT)          //AP off               
      VALUE_IS_TOKEN(rs_CmdParam_Disconnect, EEMC_WIFI_AP_DISCONNECT)     //AP off 
      BEGIN_GROUP_TOKEN(rs_CmdParam_Config)                               //AP configuration
        
        VALUE_IS_TOKEN(rs_CmdParam_Get, EEMC_WIFI_AP_CFG_GET)                //Get AP configuration        
        BEGIN_OBJECT(rs_CmdParam_Set, WIFI_AP_CONNECT, EEMC_WIFI_AP_CFG_SET) //Set AP configuration
          DATA_MEMBER(rs_CmdParam_SSID, ssid)                                   //SSID
          DATA_MEMBER(rs_CmdParam_Password, pwd, "")                            //Password
          DATA_MEMBER_AS_IP(rs_CmdParam_IP, ipaddress, 0)                       //IP address
          DATA_MEMBER_AS_IP(rs_CmdParam_Gateway, gateway, 0)                    //Gateway
          DATA_MEMBER_AS_IP(rs_CmdParam_SubnetMask, subnetMask, 0)              //Subnet mask
        END_OBJECT() 
        VALUE_IS_TOKEN(rs_CmdParam_Clear, EEMC_WIFI_AP_CFG_CLEAR)             //Clear AP configuration
      END_GROUP_TOKEN()      
    END_GROUP_TOKEN()
        
    BEGIN_GROUP_TOKEN(rs_CmdParam_Station)                          //Station control
      VALUE_IS_TOKEN(rs_CmdParam_Status, EEMC_WIFI_STATUS)          //Get station status
      VALUE_IS_TOKEN(rs_CmdParam_Scan, EEMC_WIFI_SCAN)              //Scan networks
  
      BEGIN_OBJECT(rs_CmdParam_Connect, WIFI_CONNECT, EEMC_WIFI_CONNECT)  //Connect
        DATA_MEMBER(rs_CmdParam_SSID, ssid)                                  //SSID
        DATA_MEMBER(rs_CmdParam_Password, pwd, "")                           //Password
        DATA_MEMBER_AS_IP(rs_CmdParam_Gateway, gateway, 0)                   //Gateway
        DATA_MEMBER_AS_IP(rs_CmdParam_SubnetMask, subnetMask, 0)             //Subnet mask
        DATA_MEMBER_AS_IP(rs_CmdParam_DNS1, dns1, 0)                         //DNS 1
        DATA_MEMBER_AS_IP(rs_CmdParam_DNS2, dns2, 0)                         //DNS 2
      END_OBJECT()    
    VALUE_IS_TOKEN(rs_CmdParam_Disconnect, EEMC_WIFI_DISCONNECT)        //Disconnect from WiFi network
      BEGIN_GROUP_TOKEN(rs_CmdParam_Config)                                 //Station configuration
        VALUE_IS_TOKEN(rs_CmdParam_Get, EEMC_WIFI_CFG_GET)                     //Get station configuration
        BEGIN_OBJECT(rs_CmdParam_Set, WIFI_CONNECT, EEMC_WIFI_CFG_SET)         //Set station configuration
          DATA_MEMBER(rs_CmdParam_SSID, ssid)                                   //SSID
          DATA_MEMBER(rs_CmdParam_Password, pwd, "")                            //Password
          DATA_MEMBER_AS_IP(rs_CmdParam_IP, ipaddress, 0)                       //IP address
          DATA_MEMBER_AS_IP(rs_CmdParam_Gateway, gateway, 0)                    //Gateway
          DATA_MEMBER_AS_IP(rs_CmdParam_SubnetMask, subnetMask, 0)              //Subnet mask
          DATA_MEMBER_AS_IP(rs_CmdParam_DNS1, dns1, 0)                          //DNS 1
          DATA_MEMBER_AS_IP(rs_CmdParam_DNS2, dns2, 0)                          //DNS 2
        END_OBJECT()
        VALUE_IS_TOKEN(rs_CmdParam_Clear, EEMC_WIFI_CFG_CLEAR)              //Clear station configuration
      END_GROUP_TOKEN()      
    
    END_GROUP_TOKEN()
  
  END_GROUP_TOKEN()
END_PARSE_ROUTINE()


///////////////////////////////////////
// WiFi Command Handling

bool isStaticIP = false; //Static IP is set
bool isDNS1     = false; //DNS1 is set
bool isDNS2     = false; //DNS2 is set

bool onWiFiCmd(struct CtrlQueueItem &itm, NtfSet &ntf){
  switch(itm.cmd){
    case EEMC_WIFI_STATUS: // Retreieve current wifi status
      ntf.put(CmdResponse<WIFI_STATUS_STATION>{itm.cmd});      
    break;

    case EEMC_WIFI_STATUS_CHANGE: { //Change of WiFi status       
      if(WiFi.status() == WL_CONNECTED){
        //Save last known configutation
        WIFI_CONNECT wc;

        strncpy(wc.ssid, WiFi.SSID().c_str(), SSID_LENGHT);
        strncpy(wc.pwd, WiFi.psk().c_str(), PWD_LENGHT);
        wc.ipaddress  = isStaticIP ? (uint32_t)WiFi.localIP() : 0 ;
        wc.gateway    = isStaticIP ? (uint32_t)WiFi.gatewayIP() : 0;
        wc.subnetMask = isStaticIP ? (uint32_t)WiFi.subnetMask(): 0;
        wc.dns1       = isDNS1 ? (uint32_t)WiFi.dnsIP(0) : 0;
        wc.dns2       = isDNS2 ? (uint32_t)WiFi.dnsIP(1) : 0;
        
        
        //Save configuration
        DBG_OUTLN("Wifi Write Config");
        writeWiFiConfig(wc);
      }

      ntf.put(CmdResponse<WIFI_STATUS_STATION>{itm.cmd});  
    }
    break;

    case EEMC_WIFI_CONNECT:{ //Connect to WiFi network      

      WIFI_CONNECT *wc = (WIFI_CONNECT *)itm.data.str;          
      connectWiFi(*wc);
      ntf.put(CmdResponse<>{itm.cmd, EEER_DELAYED});  
      
      //Rememeber settings
      isStaticIP = wc->ipaddress != 0;
      isDNS1     = wc->dns1      != 0;
      isDNS2     = wc->dns2      != 0;
    }    
    break;

    case EEMC_WIFI_DISCONNECT: //Disconnect 
      disconnectWiFi();
      ntf.put(CmdResponse<>{itm.cmd, EEER_DELAYED});        
    break;

    case EEMC_WIFI_CFG_GET:{ //Get WiFi configuration
      WIFI_CONNECT wc;
      readWiFiConfig(wc);

      ntf.put(CmdResponse<WIFI_CONNECT>{itm.cmd, wc}); 
    }
    break;

    case EEMC_WIFI_CFG_SET:{ //Set WiFi configuration
      WIFI_CONNECT *wc = (WIFI_CONNECT *)itm.data.str;     
      writeWiFiConfig(*wc);
      readWiFiConfig(*wc);

      ntf.put(CmdResponse<WIFI_CONNECT>{itm.cmd, *wc}); 
    }
    break;

    case EEMC_WIFI_CFG_CLEAR: { //Clear WiFi configuration
      WIFI_CONNECT wc;
      clearWiFiConfig();      
      readWiFiConfig(wc);
      ntf.put(CmdResponse<WIFI_CONNECT>{itm.cmd, wc}); 
    }
    break;

    case EEMC_WIFI_AP_STATUS: { //Get AP status
      ntf.put(CmdResponse<WIFI_STATUS_AP>{itm.cmd}); 
    }
    break;
    
    case EEMC_WIFI_AP_CONNECT:{ //Connect AP     
      WIFI_AP_CONNECT wcAp;
      
      readWiFiAPConfig(wcAp);
      connectWiFiAP(wcAp);      
    
      ntf.put(CmdResponse<WIFI_STATUS_AP>{itm.cmd}); 
    }
    break;

    case EEMC_WIFI_AP_DISCONNECT: { //Disconnect AP 
      disconnectWiFiAP();
      ntf.put(CmdResponse<WIFI_STATUS_AP>{itm.cmd});       
    }
    break;

    case EEMC_WIFI_AP_CFG_GET:{ //Get AP configuration
      WIFI_AP_CONNECT wcAp;
      readWiFiAPConfig(wcAp);

      ntf.put(CmdResponse<WIFI_AP_CONNECT>{itm.cmd, wcAp}); 
    }
    break;

    case EEMC_WIFI_AP_CFG_SET:{ //Set AP configuration
      WIFI_AP_CONNECT *wcAp = (WIFI_AP_CONNECT *)itm.data.str;     
      writeWiFiAPConfig(*wcAp);
      readWiFiAPConfig(*wcAp);

      ntf.put(CmdResponse<WIFI_AP_CONNECT>{itm.cmd, *wcAp}); 
    }
    break;
  
    case EEMC_WIFI_AP_CFG_CLEAR: { //Clear AP configuration
      WIFI_AP_CONNECT wcAp;
      clearWiFiApConfig();
      readWiFiAPConfig(wcAp);
      ntf.put(CmdResponse<WIFI_AP_CONNECT>{itm.cmd, wcAp}); 
    }
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



    default: 
    return false;
  }

  return true;
}


#endif