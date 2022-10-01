#include <Arduino.h>
#include <DbgTool.h>
#include <AnalogInput.h>
#include <CtrlSerial.h>
//#include <Notification.h>


BEGIN_PARSE_ROUTINE(TestParse)

  BEGIN_GROUP_TOKEN("mode|m")  
    //mode set next[default]|prev|number
    BEGIN_GROUP_TOKEN("set|s")
        TOKEN_IS_TEXT("next|n|", 26, 1)
        TOKEN_IS_TEXT("prev|p", 27, 2)
        TOKEN_IS_NUMBER(25)
    END_GROUP_TOKEN()
    //mode get[default]      
    TOKEN_IS_TEXT("get|", 28)   
  END_GROUP_TOKEN()

  BEGIN_GROUP_TOKEN("effect|e")
    BEGIN_GROUP_TOKEN("set|s|")
        TOKEN_IS_TEXT("next|n|", 16, 1)
        TOKEN_IS_TEXT("prev|p", 17, 2)
        TOKEN_IS_NUMBER(15)
    END_GROUP_TOKEN()
  END_GROUP_TOKEN();
  
END_PARSE_ROUTINE()



void putNtfObject(NtfBase &resp, const CtrlQueueData &data){
  resp.put_F(F("value"), data.value);
  resp.put_F(F("flag"), data.flag);
  resp.put_F(F("min"), data.min);
  resp.put_F(F("max"), data.min);
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
  
  SerialInput   serial;
  CtrlItemSerial ctrl(&serial, TestParse);

  CtrlPanel panel;
  panel.addControl(&ctrl);

  NtfSet set;
  set.addNtf(&ctrl);
  set.addNtf(&ctrl);

  CtrlQueueItem itm;

  test(&ctrl, &itm, 1);

  for(;;){
    
    panel.loop(itm);    

    if(itm.cmd != EEMC_NONE){      
      
      
      set.put_F(F("cmd"), &itm, 1);
      //set.put("", itm);
      
    }  
  }

  

  

}



void loop(){
}
