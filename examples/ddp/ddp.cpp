// Low-level DDP ambient light example.
//
// Receives DDP pixel data over WiFi (UDP, default port 4048) and drives a
// WS2812 strip directly - no ESPHome, no Home Assistant "light" abstraction.
// When the stream stops (TV off), the strip blanks and stays dark until data
// resumes.
//
// Build/flash with one of the ddp_* envs in platformio.ini, e.g.:
//   pio run -e ddp_neopixelbus -t upload
//
// Set your WiFi credentials below (or swap in initWiFi() from CtrlWiFi.h if you
// prefer this library's stored-config / captive-portal flow).

#include <arduinolib.h>
#include <DbgTool.h>
#include <AmbientLight.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

#define WIFI_SSID "zoo"
#define WIFI_PWD  "$htein12"

AmbientLight theLight;
DDPLightReceiver theReceiver;

void setup(){
  DBG_INIT();
  DBG_OUTLN("DDP light starting");

  //Bring up WiFi (station mode)
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while(WiFi.status() != WL_CONNECTED){
    delay(250);
    DBG_OUT(".");
  }
  DBG_OUTLN("\nWiFi connected: %s", WiFi.localIP().toString().c_str());

  //Start the DDP receiver on port 4048
  theReceiver.init(4048);
  theLight.init(&theReceiver, true);

  DBG_OUTLN("Listening for DDP on UDP 4048");
}

void loop(){
  theLight.loop();
}
