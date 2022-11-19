#include <Arduino.h>
#include <DbgTool.h>
#include <AnalogInput.h>
#include <CtrlSerial.h>
#include <EEPROMCfg.h>

void setup() {
  DBG_INIT();

  DBG_OUTLN("Started");
  

  struct S1{
    uint8_t ver; 
    uint8_t val1;
    int16_t val2;
    char str[16];
  };

  S1 s1{0x01, 12, -800};
  strcpy(s1.str, "Test");
  DBG_OUTLN("%d, %d, %d, %s", s1.ver, s1.val1, s1.val2, s1.str);

  {
    EEPROMCfg cfg;
    cfg.write(&s1, sizeof(S1));
    
    s1.ver = 2;
    strcpy(s1.str, "Test 2");
    cfg.write(&s1, sizeof(S1));
  }

  memset(&s1, 0, sizeof(S1));

  { 
    EEPROMCfg cfg;
    cfg.read(&s1, sizeof(S1));
    DBG_OUTLN("%d, %d, %d, %s", s1.ver, s1.val1, s1.val2, s1.str);
    cfg.read(&s1, sizeof(S1));
    DBG_OUTLN("%d, %d, %d, %s", s1.ver, s1.val1, s1.val2, s1.str);
  }
}



void loop(){
}
