#include <Arduino.h>
#include <DbgTool.h>
#include <AnalogInput.h>
#include <CtrlSerial.h>
#include <CtrlWebSrv.h>

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>



BEGIN_PARSE_ROUTINE(TestParse)

  BEGIN_GROUP_TOKEN("set|")  
     VALUE_IS_TOKEN("off", 1, 0)
     VALUE_IS_TOKEN("on", 1, 1)
  END_GROUP_TOKEN()

END_PARSE_ROUTINE()



void putNtfObject(NtfBase &resp, const CtrlQueueData &data){
  resp.put_F(F("value"), data.value);
  resp.put_F(F("flag"), data.flag);
  resp.put_F(F("min"), data.min);
  resp.put_F(F("max"), data.max);
}

void putNtfObject(NtfBase &resp, const CtrlQueueItem &data){
  resp.put_F(F("cmd"), data.cmd);
  resp.put_F(F("data"), data.data);
}


CtrlPanel cp;
AsyncWebServer server(80);
NtfBaseSet<2> ntf;


void setup() {

  DBG_INIT();
  DBG_OUTLN("Started");

  WiFi.mode(WIFI_STA);
  WiFi.begin("zoo", "$htein12");
  //WiFi.begin("Taal", "Ring023@");

  DBG_OUT("Connecting to WIFI");
  unsigned ms = millis();

  while(WiFi.status() != WL_CONNECTED){
    if(millis() - ms > 100){
      DBG_OUT(".");
      ms = millis();
    }
  }

  DBG_OUTLN(" connected, ipaddress %s", WiFi.localIP().toString().c_str());


  static WebApiInput webIn;
  static CtrlItemWebApi<TestParse> webCtrl(&webIn);
  cp.addControl(&webCtrl);
  ntf.addNtf(&webCtrl);

  static SerialInput serIn;
  static CtrlItemSerial<TestParse> serCtrl(&serIn);
  cp.addControl(&serCtrl);
  ntf.addNtf(&serCtrl);



  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });

  server.on("/test", HTTP_GET, APIRequestHandler);
  server.begin();  
}



void loop(){
  CtrlQueueItem itm;
  cp.loop(itm);

  if(itm.cmd != EEMC_NONE){
    ntf.put(itm);
  }
}



