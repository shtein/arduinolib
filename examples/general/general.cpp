#include <Arduino.h>
#include <DbgTool.h>
#include <Controls.h>
#include <AnalogInput.h>
#include <CtrlSerial.h>
#include <string.h>
//#include <FS.h>

//#define FASTLED_ESP8266_D1_PIN_ORDER
#include <fastled.h>

struct TEST_DATA{  
  int  n; 
  int p;
  char str[32];
};

BEGIN_PARSE_ROUTINE(TestParse)

  BEGIN_GROUP_TOKEN(PSTR("set|s"))  
    VALUE_IS_TOKEN(PSTR("off"), 1, 0)
    VALUE_IS_TOKEN(PSTR("on"), 1, 1)
  END_GROUP_TOKEN()
  
  BEGIN_GROUP_TOKEN(PSTR("test|t"))
    VALUE_IS_PAIR(PSTR("str|s"), 2, CTF_VAL_STRING)
    VALUE_IS_PAIR(PSTR("num|n"), 2, CTF_VAL_ABS)

    BEGIN_OBJECT(PSTR("data|d"), TEST_DATA, 3)
      DATA_MEMBER(PSTR("string|s"), str)
      DATA_MEMBER(PSTR("num|n"), n)
      DATA_MEMBER(PSTR("p"), p, 20)
    END_OBJECT()

  END_GROUP_TOKEN()
  
END_PARSE_ROUTINE()


void p1(NtfBase &resp, const TEST_DATA &data){
      resp.put_F(PSTR("str"), data.str);
      resp.put_F(PSTR("n"), data.n);
      resp.put_F(PSTR("p"), data.p);
    };

void putNtfObject(NtfBase &resp, const TEST_DATA &data){
  resp.put_F(PSTR("str"), data.str);
  resp.put_F(PSTR("n"), data.n);
  resp.put_F(PSTR("p"), data.p);
}


void putNtfObject(NtfBase &resp, const CtrlQueueData &data){
  if(data.flag == CTF_VAL_STRING){
    resp.put_F(PSTR("value"), data.str);
  }
  else{
    resp.put_F(PSTR("value"), data.value);
  }
  resp.put_F(PSTR("flag"), data.flag);
  resp.put_F(PSTR("min"), data.min);
  resp.put_F(PSTR("max"), data.max);
}

void putNtfObject(NtfBase &resp, const CtrlQueueItem &data){
  resp.put_F(PSTR("cmd"), data.cmd);
  if(data.cmd == 3){
    
    TEST_DATA *d = (TEST_DATA *)data.data.str;
    resp.put_F(PSTR("data"), *d);
/*    
    resp.put("data", *d, *[](NtfBase &resp, const TEST_DATA &data){
      resp.put_F(F("str"), data.str);
      resp.put_F(F("n"), data.n);
      resp.put_F(F("p"), data.p);

    });    
*/
    
  }
  else{
    resp.put_F(PSTR("data"), data.data);    
  }
  
}
  
typedef NtfBaseSet<2> NtfSet;

void setup() {

  DBG_INIT();

  delay(500);
  DBG_OUTLN("Started");

/******************/
  
  CHSV hsvBase(random8(), 0xFF, 0xFF);
  
  
  while(hsvBase.value != 0){    

    CHSV hsv = hsvBase;
    hsv.value = triwave8(hsvBase.value);
    CRGB rgb1 = hsv;

    CRGB rgb2;    

    rgb2.setHSV(hsv.h, hsv.s, hsv.v);


    DBG_OUTLN("c1:%d %d %d  c2:%d %d %d", rgb1.red, rgb1.blue, rgb1.green, rgb2.red, rgb2.blue, rgb2.green);

    hsvBase.value -= 5;
  }
  
/******************/


  CtrlPanel panel;

  SerialInput serial;
  CtrlItemSerial<TestParse> ctrlSr(&serial);  
  panel.addControl(&ctrlSr);

  static NtfSet set;
  set.addNtf(&ctrlSr);

  
  auto PBMAP = 
  [](uint8_t ctrl, CtrlQueueData &data) -> uint8_t {     
    DBG_OUTLN("Ctrl %X", ctrl);

    switch(ctrl){
      //case PB_CONTROL_CLICK_SHORT:                                    
      //return 1;
      case PB_CONTROL_CLICK:
        data.flag = CTF_VAL_ABS;
        data.value = 22;
      return 2;    
    }

    return EEMC_NONE;
  };


  PushButton btn(10);  
  CtrlItemPb ctrlPb(PBMAP, &btn);
  panel.addControl(&ctrlPb);


  auto IRCmdMap = 
  [](unsigned long btn, CtrlQueueData &data) -> uint8_t { 
    memset(&data, 0, sizeof(data));

    switch(btn){
      case R_DEC_KEY_OK:                            
      return 12;
      case R_DEC_KEY_UP:                            
        data.flag  = CTF_VAL_NEXT;                            
      return 14;
      case R_DEC_KEY_DOWN:
        data.flag  = CTF_VAL_PREV;
        data.value = 11;
      return 14;    
      case R_DEC_KEY_LEFT:
        data.flag  = CTF_VAL_DELTA;
        data.value = 5;
      return 16;
      case R_DEC_KEY_RIGHT:
        data.flag  = CTF_VAL_DELTA;
        data.value = -5;
      return 16;
    }

    return EEMC_NONE;
  };

  
  IRRemoteRecv ir(7);
  CtrlItemIR  ctrlIrB1(IRCmdMap, &ir);
  panel.addControl(&ctrlIrB1);


  CRGB leds[30];
  FastLED.addLeds<NEOPIXEL, 2>(leds, 30).setCorrection( TypicalLEDStrip );
  fill_solid(leds, 30, CRGB::BlueViolet);

  unsigned long ms = 0;

  CtrlQueueItem itm;
  
  for(;;){

   if(millis() - ms > 30){
      ms = millis();
      FastLED.show();
    }

   
    panel.loop(itm);         

    if(itm.cmd != EEMC_NONE){      
            
      if(itm.cmd == 1){
        if(itm.data.value == 0){
          fill_solid(leds, 30, CRGB::Black);
        }
        else if(itm.data.value == 1){
          fill_solid(leds, 30, CRGB::Red);
        }
      }

      if(itm.cmd == EEMC_ERROR){
        CmdResponse<> resp{itm.cmd, 0xFF};
        set.put(resp);
      }
      else {
        CmdResponse<const CtrlQueueItem &> resp{itm.cmd, itm};
        set.put(resp);
      }

    }  

    itm.cmd = EEMC_NONE;         
  }  
}



void loop(){

}