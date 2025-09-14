#include <arduinolib.h>
#include <DbgTool.h>
#include <AnalogInput.h>
#include <CtrlSerial.h>
#include <CtrlWebSrv.h>
#include <CtrlWiFi.h>
#include <Notification.h>


CtrlPanel cp;
NtfSet ntf;


void setup() {

  DBG_INIT();
  DBG_OUTLN("Started");

  //ESP.eraseConfig();

  LittleFS.begin();

  //initWiFi(DEFAULT_HOST("TEST"), DEFAULT_AP("TEST"));
  initWiFi(); 
  static CtrlWifiStatus wifiStatus;
  cp.addControl(&wifiStatus);

 
  webServer.begin(80);

  ADD_API_REQUEST_HANDLER(HTTP_GET, "/api");

  webServer.serveStatic("/", LittleFS, "/wifi_settings.html");
  //webServer.serveStatic("/favicon.ico", LittleFS, "/favicon.ico");

  webServer.onNotFound([](){
    DBG_OUTLN("URI %s, method %d", webServer.uri().c_str(), (int)webServer.method());

    if(!handleCaptivePortal("/")){
      webServer.send(404);
    }
  });
  
  //Wev
  static WebApiInput webIn;
  static CtrlItemWebApi<parseWiFiCmd> webCtrl(&webIn);
  cp.addControl(&webCtrl);
  ntf.addNtf(&webCtrl);
   
  //Serial
  static SerialInput serIn;
  static CtrlItemSerial<parseWiFiCmd> serCtrl(&serIn);
  cp.addControl(&serCtrl);
  ntf.addNtf(&serCtrl);

  DBG_OUTLN("Started");
}


void loop(){  
  CtrlQueueItem itm;
  cp.loop(itm);


  if(itm.cmd != EEMC_NONE){
    if(itm.cmd && EEMC_WIFI){
      if(!onWiFiCmd(itm, ntf)){
        ntf.put(CmdResponse<>{ itm.cmd, EEER_UNHANDLED } );
      }
    }
    else{
      ntf.put(CmdResponse<>{ itm.cmd, EEER_INVALID } );
    }
  }
}



