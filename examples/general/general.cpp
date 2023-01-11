#include <Arduino.h>
#include <DbgTool.h>
#include <AnalogInput.h>
#include <CtrlSerial.h>
#include <string.h>

//#define FASTLED_ESP8266_D1_PIN_ORDER
#include <fastled.h>



BEGIN_PARSE_ROUTINE(TestParse)

  BEGIN_GROUP_TOKEN("set|")  
     TOKEN_IS_TEXT("off", 1, 0)
     TOKEN_IS_TEXT("on", 1, 1)
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
  


template <typename ... ARGS>
void test(NtfBase *p, const ARGS&... args){
  CtrlQueueData data;
  p->put(NULL, args...);
}

#define MAX_NTF 3
typedef NtfBaseSet<MAX_NTF> NtfSet;



void setup() {

  DBG_INIT();
  DBG_OUTLN("Started");


  CtrlPanel panel;


  SerialInput serial;
  CtrlItemSerial<TestParse> ctrlSr(&serial);  
  panel.addControl(&ctrlSr);

  static NtfSet set;
  set.addNtf(&ctrlSr);

  
  PushButton btn(0);  
  CtrlItemPb<PB_CONTROL_CLICK_SHORT, 1> ctrlPb(1, &btn);
  panel.addControl(&ctrlPb);


  CtrlQueueItem itm;

  CRGB leds[30];
  FastLED.addLeds<NEOPIXEL, 2>(leds, 30).setCorrection( TypicalLEDStrip );
  fill_solid(leds, 30, CRGB::BlueViolet);

  unsigned long ms = 0;
  
  for(;;){

   if(millis() - ms > 30){
      ms = millis();
      FastLED.show();
    }

   
    panel.loop(itm);         

    if(itm.cmd != EEMC_NONE){      
            
      set.put_F(F("response"), itm);

      if(itm.cmd == 1){
        if(itm.data.value == 0){
          fill_solid(leds, 30, CRGB::Black);
        }
        else if(itm.data.value == 1){
          fill_solid(leds, 30, CRGB::Red);
        }
      }

    }  

    itm.cmd = EEMC_NONE;         
  }
  
  
}



void loop(){

}