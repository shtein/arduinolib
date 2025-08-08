#include "DbgTool.h"


#define BUF_SIZE 64

void _dbgOut(const __FlashStringHelper *fmt, ...){
  char buf[BUF_SIZE];
  va_list args;
  
  va_start (args, fmt);
  vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args); 
  va_end(args);
  
  Serial.print(buf);
}


