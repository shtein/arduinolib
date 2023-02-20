#include <Arduino.h>
#include <DbgTool.h>
#include <AnalogInput.h>
#include <CtrlSerial.h>
#include <CtrlWebSrv.h>
#include <CtrlWiFi.h>
#include <LittleFS.h>
#include <DNSServer.h> 

#define ERR_SUCCESS    0x00
#define ERR_INVALID    0x01
#define ERR_UNHANDLED  0x02

#define CMD_WIFI_CONNECT     0x01
#define CMD_WIFI_DISCONNECT  0x02
#define CMD_WIFI_STATUS      0x03
#define CMD_WIFI_NETWORKS    0x04
#define CMD_WIFI_AP          0x05       

template <typename ...Ts>
struct RESP{};

template<>
struct RESP<>{
  uint8_t cmd;
  uint8_t error;
};

template<typename T> 
struct RESP<T>{
  uint8_t cmd;
  T data;
}; 

inline void putNtfObject(NtfBase &resp, const RESP<> &r){
  resp.put_F(F("cmd"), r.cmd);
  resp.put_F(F("error"), r.error);
}

//Default serialization

template <class T>
void putNtfObject(NtfBase &resp, const RESP<T> &r){
  resp.put_F(F("cmd"), r.cmd);
  resp.put_F(F("data"), r.data);
}


BEGIN_PARSE_ROUTINE(TestParse)

  BEGIN_GROUP_TOKEN("wifi")  
    BEGIN_OBJECT("connect|c", WIFI_CONNECT, CMD_WIFI_CONNECT)
      DATA_MEMBER("ssid|s", ssid)
      DATA_MEMBER("pwd|p", pwd) 
    END_OBJECT()
    VALUE_IS_TOKEN("disconnect|d", CMD_WIFI_DISCONNECT)
    VALUE_IS_TOKEN("status|", CMD_WIFI_STATUS)
    VALUE_IS_TOKEN("scan|s", CMD_WIFI_NETWORKS)
    BEGIN_GROUP_TOKEN("ap")
      VALUE_IS_TOKEN("on", CMD_WIFI_AP, 1)
      VALUE_IS_TOKEN("off", CMD_WIFI_AP, 0)
    END_GROUP_TOKEN()
  END_GROUP_TOKEN()

END_PARSE_ROUTINE()



CtrlPanel cp;
NtfBaseSet<2> ntf;

DNSServer dnsServer;


void setup() {
  

  DBG_INIT();
  DBG_OUTLN("Started");

  LittleFS.begin();
  webServer.begin(80);

  ADD_API_REQUEST_HANDLER(HTTP_GET, "/api");

  webServer.serveStatic("/", LittleFS, "/wifi_settings.html");
  webServer.serveStatic("/favicon.ico", LittleFS, "/favicon.ico");

  webServer.onNotFound([](){
    DBG_OUTLN("URI %s, method %d", webServer.uri().c_str(), (int)webServer.method());

    if(WiFi.softAPIP() == webServer.client().localIP()){
      String url = "http://" + WiFi.softAPIP().toString() + "/wifi_settings.html";

      webServer.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/", true); 
      webServer.send(302, "text/plain", ""); 
      DBG_OUTLN("redirecting"); 
    }
    else{
      webServer.send(404);
    }
  });

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", IPAddress(172, 16, 25, 25));

  
  static WebApiInput webIn;
  static CtrlItemWebApi<TestParse> webCtrl(&webIn);
  cp.addControl(&webCtrl);
  ntf.addNtf(&webCtrl);
  


  static SerialInput serIn;
  static CtrlItemSerial<TestParse> serCtrl(&serIn);
  cp.addControl(&serCtrl);
  ntf.addNtf(&serCtrl);

  static CtrlWifiStatus wifiStatus(CMD_WIFI_STATUS);
  cp.addControl(&wifiStatus);

  
}



void loop(){
  dnsServer.processNextRequest();

  CtrlQueueItem itm;
  cp.loop(itm);


  if(itm.cmd != EEMC_NONE){

    switch(itm.cmd){
      case CMD_WIFI_CONNECT:{
        WIFI_CONNECT *c = (WIFI_CONNECT *)itm.data.str;        
        WiFi.begin(c->ssid, c->pwd);
      }        
      break;

      case CMD_WIFI_DISCONNECT:
        WiFi.disconnect(true);
      break;

      case CMD_WIFI_STATUS: {
        ntf.put(RESP<WIFI_STATUS>{itm.cmd} );
      }
      break;
          
      case CMD_WIFI_NETWORKS: {
        int8_t scan = WiFi.scanComplete();
        if(scan == WIFI_SCAN_FAILED){
          scan = WiFi.scanNetworks(true, true);
        }      

        ntf.put(RESP<WIFI_SCAN>{itm.cmd});

        if(scan >= 0){
          WiFi.scanDelete();
        }
      break; }

      case CMD_WIFI_AP:{
        if(itm.data.value == 0){ //Disconnect AP
          WiFi.softAPdisconnect(true);
        }
        else {          
          WiFi.softAPConfig(IPAddress(172, 16, 25, 25),
                            IPAddress(172, 16, 25, 25), 
                            IPAddress(255, 255, 255, 0) 
                          );            
        
          WiFi.softAP("ESP8266_test");
        }
        ntf.put(RESP<WIFI_STATUS>{itm.cmd} );
      }
      break;

      case EEMC_ERROR: {
        ntf.put(RESP<>{itm.cmd, ERR_INVALID});
      }
      break;

      default: {
        ntf.put(RESP<>{itm.cmd, ERR_UNHANDLED});
      }        
      break;
    };
  }
}



