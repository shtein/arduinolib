#include <Arduino.h>
#include <DbgTool.h>
#include <AnalogInput.h>
#include <CtrlSerial.h>

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);
const char* ssid = "zoo";
const char* password = "$htein12";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void setup() {

  DBG_INIT();
  DBG_OUTLN("Started");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  DBG_OUT("Connecting to WIFI");
  unsigned ms = millis();

  while(WiFi.status() != WL_CONNECTED){
    if(millis() - ms > 100){
      DBG_OUT(".");
      ms = millis();
    }
  }


  DBG_OUTLN(" connected, ipaddress %s", WiFi.localIP().toString().c_str());

  delay(500);
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });

   // Send a GET request to <IP>/get?message=<message>
  server.on("/test", HTTP_GET, [] (AsyncWebServerRequest *request) {        
        request->send(Serial, "text/plain", 12);
    });

  

  server.onNotFound(notFound);  
  server.begin();
 
  for(;;){

  }
  
}



void loop(){

}



