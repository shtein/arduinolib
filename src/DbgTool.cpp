#include "DbgTool.h"

#if defined(DBG_TO_SERIAL) && defined(DBG_TO_SW_SERIAL)
  #error "Debug output can be only to one serial (serial and sofwtare serial are defined)"
#endif

#if defined(DBG_TO_SERIAL) 
  #pragma message "Warning: debugging via Serial is enabled"

  #define DBG_SERIAL        Serial
  #define DBG_SERIAL_SPEED  57600

#elif defined(DBG_TO_SW_SERIAL)
  #include "SoftwareSerial.h"

  SoftwareSerial swSerial(3, 4);
  #define DBG_SERIAL        swSerial
  #define DBG_SERIAL_SPEED  9600

  #pragma message "Warning: debugging via SoftwareSerial is enabled"
#endif

#if defined(DBG_TO_SERIAL) || defined(DBG_TO_SW_SERIAL)
void _dbgInit(){
  DBG_SERIAL.begin(DBG_SERIAL_SPEED);
}

#define BUF_SIZE 64

void _dbgOut(const __FlashStringHelper *fmt, ...){
  char buf[BUF_SIZE];
  va_list args;
  
  va_start (args, fmt);
  vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args); 
  va_end(args);
  
  DBG_SERIAL.print(buf);
}
#endif // DBG_TO_SERIAL || DBG_TO_SW_SERIAL

