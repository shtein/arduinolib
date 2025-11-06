#include <Arduino.h>
#include <DbgTool.h>
#include <Controls.h>
#include <AnalogInput.h>
#include <CtrlSerial.h>


void setup() {

  DBG_INIT();
  delay(500);
  DBG_OUTLN("Started");

/******************/
  
  
  CtrlPanel panel;

  SerialInputBinary serial;
  CtrlItemSerialBinary ctrlSr(&serial);
  panel.addControl(&ctrlSr);

  PushButton pb(10);
  CtrlItemPb ctrlPb([](uint8_t ctrl, CtrlQueueData &data)->uint8_t{
      switch(ctrl){
      case PB_CONTROL_CLICK_SHORT:                                    
        data.flag = CTF_VAL_ABS;
        data.value = 15;
      return 1;
      case PB_CONTROL_CLICK_LONG:
        data.flag = CTF_VAL_ABS;
        data.value = 22;
      return 2;    
    }
    return EEMC_NONE; }, &pb);
  panel.addControl(&ctrlPb);

  
  
  CtrlQueueItem itm;
  
  for(;;){
      
    panel.loop(itm);         

    if(itm.cmd != EEMC_NONE){            
      DBG_OUTLN("Cmd: %d  Data flag: %d  value: %d", itm.cmd, itm.data.flag, itm.data.value);      
    }  

    itm.cmd = EEMC_NONE;         
  }  
}



void loop(){

}