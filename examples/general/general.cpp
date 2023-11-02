#include <Arduino.h>
#include <DbgTool.h>
#include <Controls.h>
#include <AnalogInput.h>
#include <CtrlSerial.h>
#include <string.h>
#include <FS.h>

//#define FASTLED_ESP8266_D1_PIN_ORDER
#include <fastled.h>

struct TEST_DATA{  
  int  n; 
  int p;
  char str[32];
};

BEGIN_PARSE_ROUTINE(TestParse)

  BEGIN_GROUP_TOKEN("set|s")  
    VALUE_IS_TOKEN("off", 1, 0)
    VALUE_IS_TOKEN("on", 1, 1)
  END_GROUP_TOKEN()
  
  BEGIN_GROUP_TOKEN("test|t")
    VALUE_IS_PAIR("str|s", 2, CTF_VAL_STRING)
    VALUE_IS_PAIR("num|n", 2, CTF_VAL_ABS)

    BEGIN_OBJECT("data|d", TEST_DATA, 3)
      DATA_MEMBER("string|s", str)
      DATA_MEMBER("num|n", n)
      DATA_MEMBER("p", p, 20)
    END_OBJECT()

  END_GROUP_TOKEN()
  
END_PARSE_ROUTINE()

/*
template<class T>
void test(NtfBase &resp, const T &data, void (*f)(NtfBase &resp, const T &data)){
  f(resp, data);
}
*/

void p1(NtfBase &resp, const TEST_DATA &data){
      resp.put_F(F("str"), data.str);
      resp.put_F(F("n"), data.n);
      resp.put_F(F("p"), data.p);
    };

void putNtfObject(NtfBase &resp, const TEST_DATA &data){

/*
    test<TEST_DATA>(resp, data, [](NtfBase &resp, const TEST_DATA &data){
      resp.put_F(F("str"), data.str);
      resp.put_F(F("n"), data.n);
      resp.put_F(F("p"), data.p);
    });

    auto p = [](NtfBase &resp, const TEST_DATA &data){
      resp.put_F(F("str"), data.str);
      resp.put_F(F("n"), data.n);
      resp.put_F(F("p"), data.p);
    };
*/
    //resp.test(NULL, data, *p1);

  resp.put_F(F("str"), data.str);
  resp.put_F(F("n"), data.n);
  resp.put_F(F("p"), data.p);
}


void putNtfObject(NtfBase &resp, const CtrlQueueData &data){
  if(data.flag == CTF_VAL_STRING){
    resp.put_F(F("value"), data.str);
  }
  else{
    resp.put_F(F("value"), data.value);
  }
  resp.put_F(F("flag"), data.flag);
  resp.put_F(F("min"), data.min);
  resp.put_F(F("max"), data.max);
}


void putNtfObject(NtfBase &resp, const CtrlQueueItem &data){
  resp.put_F(F("cmd"), data.cmd);
  if(data.cmd == 3){
    
    TEST_DATA *d = (TEST_DATA *)data.data.str;
    resp.put_F(F("data"), *d);
/*    
    resp.put("data", *d, *[](NtfBase &resp, const TEST_DATA &data){
      resp.put_F(F("str"), data.str);
      resp.put_F(F("n"), data.n);
      resp.put_F(F("p"), data.p);

    });    
*/
    
  }
  else{
    resp.put_F(F("data"), data.data);    
  }
  
}
  


#define MAX_NTF 3
typedef NtfBaseSet<MAX_NTF> NtfSet;



void setup() {

  DBG_INIT();

  delay(500);
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
            
      set.put(itm);

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