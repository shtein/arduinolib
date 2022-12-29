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
     
    //btn.read();
    //DBG_OUTLN("%d", btn.value(PB_CONTROL_CLICK)); 
    //delay(1);
    //DBG_OUTLN("test"); 
  }
  
/*
  const int pin = 2;
  pinMode(pin, INPUT_PULLUP);
  digitalWrite(pin, HIGH);  
  int value  = digitalRead(pin);
  DBG_OUTLN("Initial %d", value);

  for(;;){
    delay(5);

    int value1 = digitalRead(pin);
    if(value != value1){
      value = value1;

      DBG_OUTLN("Updated %d", value);
      
    }
  }
  */
  
}



void loop(){

}
/*

void setup() {
  //2qdelay(10000);
  Serial.begin(57600);
  //pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
}

// the loop function runs over and over again forever
void loop() {
  //digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
                                    // but actually the LED is on; this is because 
                                    // it is acive low on the ESP-01)
  Serial.println("LED On!");
  delay(1000);                      // Wait for a second
  //digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  Serial.println("LED Off!");
  delay(2000);                      // Wait for two seconds (to demonstrate the active low LED)
}

*/