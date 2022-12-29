#include "arduinolib.h"
#include <arduino.h>

#include "Notification.h"

///////////////////
// NtfSerial

NtfSerial::NtfSerial(){
  reset();
}

void NtfSerial::reset(){
  _ident = 0;
}

void NtfSerial::send(){
}


#define BUFF_SIZE 32

void NtfSerial::print(const __FlashStringHelper *fmt, ...){
  char buf[BUFF_SIZE];
  va_list args;
  
  va_start (args, fmt);
  vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args); 
  va_end(args);
  
  for(size_t i = 0; i < _ident; i++){
    Serial.print(F(" "));
  }

  Serial.print(buf);
}

//Print helper
#define STR_FMT(format) F(format "\n")
#define STR_PRN(format, ...) print( STR_FMT(format), ## __VA_ARGS__ )
#define STR_PRN_VAL(valformat, key, val) STR_PRN("%s: " valformat, key, val)

void NtfSerial::begin(const char *key){  
  if(key){
    STR_PRN("%s: ", key);
  }
  else{
    //STR_PRN("");
  }
  
  _ident +=1;
}

void NtfSerial::end(const char *){  
  _ident -= 1;

  //STR_PRN("");
}

void NtfSerial::beginArray(const char *key ){
  if(key){
    STR_PRN("%s: ", key);
  }
  else{
    //STR_PRN("");
  }
  
  _ident +=1;
}

void NtfSerial::endArray(const char *key){
  _ident -= 1;

  //STR_PRN("");
}

void NtfSerial::put(const char *key, uint8_t v){
  STR_PRN_VAL("%u", key, v);
}

void NtfSerial::put(const char *key, uint16_t v){
  STR_PRN_VAL("%u", key, v);
}

void NtfSerial::put(const char *key, uint32_t v){
  STR_PRN_VAL("%u", key, v);
}


void NtfSerial::put(const char *key, int8_t v){
  STR_PRN_VAL("%d", key, v);
}

void NtfSerial::put(const char *key, int16_t v){
  STR_PRN_VAL("%d", key, v);
}

void NtfSerial::put(const char *key, int32_t v){
  STR_PRN_VAL("%d", key, v);
}


void NtfSerial::put(const char *key, const char *v){
  STR_PRN_VAL("%s", key, v);
}