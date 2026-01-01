#include "CtrlSerial.h"
#include "DbgTool.h"


///////////////////////////////////
// Serial input

SerialInput::SerialInput(){  
  //Init serial
  Serial.begin(57600);

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

///////////////////////////////////////////////////
// SerailInputBinary - binary protocol input from serial port

#if defined(__AVR_ATmega32U4__)
// Pro Micro / Leonardo / Micro, etc.
  #define SerialBin Serial1
#else
// Uno, Nano, etc. (no Serial1)
  #define SerialBin Serial
#endif

SerialInputBinary::SerialInputBinary(){  
  //Init serial
  SerialBin.begin(57600);

  //Reset data
  reset();
}

void SerialInputBinary::reset(){    
  _lenRead  = 0;  
  memset(_bufRead, 0, sizeof(_bufRead));
}

//Define protocol bytes
#define SI_START_1       0x55
#define SI_START_2       0xAA

#define SI_END_1         0x5A
#define SI_END_2         0xA5

#define SI_OK_1          0x6F
#define SI_OK_2          0x6B

#define SI_MAX_WAIT_TIME 250

//Protocol:
// ==> SI_START_1, SI_START_2
// <<== SI_OK_1, SI_OK_2
// ==>LEN, DATA..., SI_END_1, SI_END_2

//LEN - length of DATA only, max SI_BUFF_LEN

//Examlple: CtrlQueueItem
//55AA16010000000000000000000000000000000000000000005AA5
//55 AA - Start
//6F 6B - OK
//16 - datalength (22 bytes)
//01 - cmd
//00... - CtrlQueueData
//5A A5 - finish

//Internal states
#define SI_STATE_WAIT     0x00
#define SI_STATE_SB       0x01
#define SI_STATE_LEN      0x02
#define SI_STATE_DATA     0x03
#define SI_STATE_SE       0x04
#define SI_STATE_READY    0x05


void SerialInputBinary::read(){  
  //If already ready, reset first
  if(isReady()){
    reset();
  }

  uint8_t state = SI_STATE_WAIT;

  //First - recive control header 55 AA
  while(SerialBin.available() ){
    uint8_t c = SerialBin.read(); 
    
    if(state == SI_STATE_WAIT && c == SI_START_1){    //First start byte received    
      state = SI_STATE_SB;
    }
    else if(state == SI_STATE_SB && c == SI_START_2){ //Second start byte received
      state = SI_STATE_LEN;      
      break; //Endof cycle
    }
    else{ //Invalid
      state = SI_STATE_WAIT;
    }
  }

  //Exit if not in data state
  if(state != SI_STATE_LEN){
    return; 
  }

  //Second - send confirmation on receiving control header
  SerialBin.write(SI_OK_1);
  SerialBin.write(SI_OK_2);    
  
  //Third - recive data, within timeout
  unsigned long startTime = millis();
  uint8_t lenExpected = 0;

  while(millis() < startTime + SI_MAX_WAIT_TIME){ //Stay within timeout

    while(SerialBin.available()){
      
      uint8_t c = SerialBin.read(); 

      if(state == SI_STATE_LEN){ //Length byte
        if(c > 0 && c <= SI_BUFF_LEN){ 
          lenExpected = c;
          state       = SI_STATE_DATA;          
        }
        else{ //Invalid
          state = SI_STATE_WAIT;      
          break; //End of cycle
        }
      }
      else if(state == SI_STATE_DATA){ //Data byte
        if(_lenRead < lenExpected){ 
          _bufRead[_lenRead++] = c;              
        }
        else if(c == SI_END_1){ //First end byte received
          state = SI_STATE_SE;          
        }
        else{ //Invalid
          state = SI_STATE_WAIT;
          break; //End of cycle
        }        
      }
      else if(state == SI_STATE_SE){ //Second end byte received
        if(c == SI_END_2){ 
          state = SI_STATE_READY;
          
          break;
        }
        else{ //Invalid
          state = SI_STATE_WAIT;
          
          break; //End of cycle
        }        
      }
    }

    if(state == SI_STATE_WAIT || state == SI_STATE_READY){ //Invalid state      
      break; //End of cycle    
    }
  }

  if(state != SI_STATE_READY){
    reset();  
  }
}

#ifdef ESPHOME_CTRL
/////////////////////////////////////
// CtrlQueueSerialBinary

#define CS_STATE_IDLE        0
#define CS_STATE_HEADER      1
#define CS_STATE_WAIT_OK     2
#define CS_STATE_DATA        3

#define CS_MAX_WAIT 1000
#define MAX_RETRIES 5

CtrlQueueSerialBinary::CtrlQueueSerialBinary() {
  _state   = CS_STATE_IDLE;
  _time    = 0;
  _retries = 0;
}


void CtrlQueueSerialBinary::sendCtrlCommand(const CtrlQueueItem &item){
  _state   = CS_STATE_HEADER;
  _itm     = item;
  _retries = 0;
}

void CtrlQueueSerialBinary::sendCtrlCommand(uint8_t cmd, uint8_t flag, int value, int min, int max){
  CtrlQueueItem item;

  item.cmd = cmd;
  item.data.flag = flag;
  item.data.value = value;
  item.data.min = min;
  item.data.max = max;

  sendCtrlCommand(item);
}

// Called when state is CS_STATE_IDLE
void CtrlQueueSerialBinary::onIdle(){
  //Drain serial buffer
  while(Serial.available() > 0){
    Serial.read();        
  }
}

// Called when state is CS_STATE_HEADER
void CtrlQueueSerialBinary::onHeader(){
  // Send start control bytes
  Serial.write(SI_START_1);
  Serial.write(SI_START_2);

  //Change state and remember time
  _state = CS_STATE_WAIT_OK;
  _time = millis();
}

// Called when state is CS_STATE_WAIT_OK
void CtrlQueueSerialBinary::onWaitOk(){
  //Nothing came during timeout
  if(_time + CS_MAX_WAIT < millis()){
    //Retry a few times
    _retries ++;
    if(_retries < MAX_RETRIES){
      _state = CS_STATE_HEADER;
      return;
    }
    //Give up, go to idle
    _state = CS_STATE_IDLE;
    return;
  }

  //Check if we have data in serial buffer
  if(!Serial.available()){
    return;
  }

  //Read data
  uint8_t ok[2] = {0, 0};
  if(Serial.readBytes(ok, 2) != 2 || ok[0] != SI_OK_1 || ok[1] != SI_OK_2){
    //Not ok, go back to idle
    _state = CS_STATE_IDLE;
    
  }
  else{
    _state = CS_STATE_DATA; 
  }
}


// Called when state is CS_STATE_DATA
void CtrlQueueSerialBinary::onData(){
  //Send data length and data
  Serial.write((uint8_t)sizeof(CtrlQueueItem));  
  Serial.write((uint8_t*)&_itm, sizeof(CtrlQueueItem));

  //Send end control bytes
  Serial.write(SI_END_1);
  Serial.write(SI_END_2);

  _state = CS_STATE_IDLE;
}


void CtrlQueueSerialBinary::loop(){
  switch(_state){
    case CS_STATE_IDLE:
      onIdle();
    break;

    case CS_STATE_HEADER:
      onHeader();
    break;

    case CS_STATE_WAIT_OK:
      onWaitOk();
    break;

    case CS_STATE_DATA:
      onData();
    break;
  }
}

#endif // ESPHOME_CTRL