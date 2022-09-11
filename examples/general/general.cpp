#include <Arduino.h>
#include <DbgTool.h>
#include <AnalogInput.h>
#include <CtrlSerial.h>


BEGIN_PARSE_ROUTINE(TestParse)

  BEGIN_GROUP_TOKEN("mode")
  
    //mode set next[default]|prev|number
    BEGIN_GROUP_TOKEN("set")
        TOKEN_IS_TEXT("next|", 26, 1)
        TOKEN_IS_TEXT("prev", 27, 2)
        TOKEN_IS_NUMBER(25)
    END_GROUP_TOKEN()
    //mode get[default]      
    TOKEN_IS_TEXT("get|", 28)   
  END_GROUP_TOKEN()

  BEGIN_GROUP_TOKEN("effect")
    BEGIN_GROUP_TOKEN("set")
        TOKEN_IS_TEXT("next|", 16, 1)
        TOKEN_IS_TEXT("prev", 17, 2)
        TOKEN_IS_NUMBER(15)
    END_GROUP_TOKEN()
  END_GROUP_TOKEN();

  
END_PARSE_ROUTINE()




void setup() {
  DBG_INIT();

  DBG_OUTLN("Started");


  SerialInput   serial;
  CtrlItemSerial ctrl(&serial, TestParse);

  CtrlPanel panel;
  panel.addControl(&ctrl);

  CtrlQueueItem itm;

  for(;;){
    
    panel.loop(itm);    

    if(itm.cmd != EEMC_NONE){      
      DBG_OUTLN ("cmd: %d, value: %d, flag: %d", itm.cmd, itm.data.value, itm.data.flag);            
    }  
  }

}

void loop(){
}
