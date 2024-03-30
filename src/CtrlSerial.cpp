#include "arduinolib.h"

#include <Arduino.h>
#include "CtrlSerial.h"
#include "DbgTool.h"


/////////////////////////////
// Command response
DEFINE_STR_PROGMEM(rs_Cmd,          "cmd")
DEFINE_STR_PROGMEM(rs_Data,         "data")
DEFINE_STR_PROGMEM(rs_Error,        "error")


///////////////////////////////////
// Serial input

SerialInput::SerialInput(){  
  //In case it was already initialized
  Serial.end(); 

  //Init serial
  Serial.begin(57600);
    while (!Serial);  

  //Reset data
  reset();
}

SerialInput::~SerialInput(){  
}

void SerialInput::read(){  
  while (Serial.available()){

    int8_t c = Serial.read();
  
    switch (c){
      case SI_CR:   //Skip it
      break;

      case SI_LF:
        _bufRead[_lenRead] = SI_NULLCHAR;

        if (_lenRead > 0){
          _lenRead = 0;                           
        }
        
      break;

      case SI_BS:                                    
        if (_lenRead > 0) {                        
          _bufRead[--_lenRead] = SI_NULLCHAR;

          Serial.print(SI_SPACE);
          Serial.print(SI_BS);          
        }
      break;

      default:
        if (_lenRead < SI_BUFF_LEN) {
          _bufRead[_lenRead++] = c;                   
          _bufRead[_lenRead]   = SI_NULLCHAR;
        }
        else{
          //Don't show input
          Serial.print(SI_BS);          
          Serial.print(SI_SPACE);
          Serial.print(SI_BS);   
        }                
      break;
    }
  }
}

#define PARSE_DELIMETER  ' '
#define ESCAPE_CHARACTER '\\'
//Get tokens into from command line into array
//Return false of numnber of tokens is more than number of elements in array

bool SerialInput::getTokens(const char *tokens[], size_t maxTokens){
  //Make sure it is ready
  if(!isReady()){
    //Return true, handle 0 tokens as no command
    return true;
  }

  //Shift one bite before reset
  memmove(_bufRead + 1, _bufRead, SI_BUFF_LEN);
  //zero first byte 
  reset();

  //Parse}
  return ::getTokens(_bufRead + 1, tokens, maxTokens, PARSE_DELIMETER, ESCAPE_CHARACTER);
}

bool SerialInput::isReady() const{
  return _bufRead[0] == SI_NULLCHAR ? false : _lenRead == 0 ? true : false; 
}

void SerialInput::reset(){
  _bufRead[0] = SI_NULLCHAR;
  _lenRead    = 0;
}

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
#define STR_FMT(format) F(format"\n")
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

void NtfSerial::end(){  
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

void NtfSerial::endArray(){
  _ident -= 1;

  //STR_PRN("");
}

void NtfSerial::put(const char *key, bool v){
  char str[6];
  strcpy_P(str, v ? PSTR("true"): PSTR("false"));
  
  STR_PRN_VAL("%s", key, str);
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

