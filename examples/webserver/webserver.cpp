#include <Arduino.h>
#include <DbgTool.h>
#include <AnalogInput.h>
#include <CtrlSerial.h>

#include <CtrlWebSrv.h>

#define ERR_SUCCESS    0x00
#define ERR_INVALID    0x01
#define ERR_UNHANDLED  0x02

#define CMD_WIFI_CONNECT     0x01
#define CMD_WIFI_DISCONNECT  0x02
#define CMD_WIFI_GET_CFG     0x03
#define CMD_WIFI_SET_CFG     0x04
#define CMD_WIFI_STATUS      0x10

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
template<typename T>
void putNtfObject(NtfBase &resp, const RESP<T> &r){
  resp.put_F(F("cmd"), r.cmd);
  resp.put_F(F("data"), r.data);
}



struct WIFI_CONNECT{
  char ssid[16];
  char pwd[16];
};

struct WIFI_CFG{
  IPAddress   ip;
  IPAddress   gateway;
  IPAddress   subnet;
  IPAddress   dns1;
  IPAddress   dns2;
};

struct WIFI_STATUS{
  uint8_t status;
};

BEGIN_PARSE_ROUTINE(TestParse)

  BEGIN_GROUP_TOKEN("wifi")  
    BEGIN_OBJECT("connect|c", WIFI_CONNECT, CMD_WIFI_CONNECT)
      DATA_MEMBER("ssid|s", ssid)
      DATA_MEMBER("pwd|p", pwd)
    END_OBJECT()
    VALUE_IS_TOKEN("disconnect|d", CMD_WIFI_DISCONNECT)
    BEGIN_GROUP_TOKEN("cfg")
      VALUE_IS_TOKEN("get|g|", CMD_WIFI_GET_CFG)      
    END_GROUP_TOKEN();
  END_GROUP_TOKEN()

END_PARSE_ROUTINE()

void putNtfObject(NtfBase &resp, const WIFI_CONNECT &data){
  resp.put_F(F("ssid"), data.ssid);
  resp.put_F(F("pwd"), data.pwd);
}

void putNtfObject(NtfBase &resp, const WIFI_CFG &data){
  resp.put_F(F("ip"), data.ip.toString().c_str());
  resp.put_F(F("gateway"), data.gateway.toString().c_str());
  resp.put_F(F("subnetmask"), data.subnet.toString().c_str());
  resp.put_F(F("dns1"), data.dns1.toString().c_str());
  resp.put_F(F("dns2"), data.dns2.toString().c_str());
}

void putNtfObject(NtfBase &resp, const WIFI_STATUS &data){
  resp.put_F(F("wifistatus"), data.status);  
}


class CtrlWifiStatus: public CtrlItem{
  public:
    CtrlWifiStatus(): CtrlItem(CMD_WIFI_STATUS, NULL) {
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


CtrlPanel cp;
NtfBaseSet<2> ntf;

void handleNotFound() {
  webServer.send(404);
}


void setup() {

  DBG_INIT();
  DBG_OUTLN("Started");

  webServer.begin(80);

  ADD_API_REQUEST_HANDLER(HTTP_GET, "/api");
  webServer.onNotFound(handleNotFound);
  

  
  static WebApiInput webIn;
  static CtrlItemWebApi<TestParse> webCtrl(&webIn);
  cp.addControl(&webCtrl);
  ntf.addNtf(&webCtrl);
  


  static SerialInput serIn;
  static CtrlItemSerial<TestParse> serCtrl(&serIn);
  cp.addControl(&serCtrl);
  ntf.addNtf(&serCtrl);

  static CtrlWifiStatus wifiStatus;
  cp.addControl(&wifiStatus);

}



void loop(){
  

  CtrlQueueItem itm;
  cp.loop(itm);

  if(itm.cmd != EEMC_NONE){
    

    switch(itm.cmd){
      case CMD_WIFI_CONNECT:{
        WIFI_CONNECT *c = (WIFI_CONNECT *)itm.data.str;
        WiFi.mode(WIFI_STA);
        WiFi.begin(c->ssid, c->pwd);
      }        
      break;
      case CMD_WIFI_DISCONNECT:
        WiFi.mode(WIFI_OFF);        
      break;
      case CMD_WIFI_GET_CFG:{
        RESP<WIFI_CFG> r;
        r.cmd          = itm.cmd;        
        r.data.ip      = WiFi.localIP();
        r.data.gateway = WiFi.gatewayIP();
        r.data.subnet  = WiFi.subnetMask();
        r.data.dns1    = WiFi.dnsIP(0);
        r.data.dns2    = WiFi.dnsIP(1); 
        ntf.put(r);         
      }
      break;
      case CMD_WIFI_STATUS: {
        ntf.put(RESP<WIFI_STATUS>{itm.cmd, {(uint8_t)itm.data.value}});
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



