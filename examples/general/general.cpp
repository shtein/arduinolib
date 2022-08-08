#include <Arduino.h>
#include <DbgTool.h>
#include <AnalogInput.h>
#include <Controls.h>


uint8_t TestParse (char *cmdLine, CtrlQueueData &data){
  DBG_OUTLN("received %s", cmdLine);
  return EEMC_NONE;
}

void setup() {
  DBG_INIT();

  Serial.begin(57600);
  while (!Serial);

  Serial.println("Started");

  
  SerialInput input;
  CtrlItemSerial ctrl(&input, TestParse);
  CtrlQueueItem itm;
  
  while(1){
    ctrl.loop(itm);
  }
}

void loop(){
}

