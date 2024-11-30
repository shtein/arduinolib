#include "arduinolib.h"
#include <Arduino.h>

#include "DbgTool.h"
#include "AnalogInput.h"
#include "utils.h"

///////////////////
//BaseInput

BaseInput::BaseInput(){
}

BaseInput::~BaseInput(){
}

////////////////////
//AnalogInput
AnalogInput::AnalogInput(uint8_t pin){
  _pin        = pin;
  _value      = analogRead(_pin);
}

AnalogInput::~AnalogInput(){  
}

void AnalogInput::read(){ 
  _value      = analogRead(_pin);
}

int AnalogInput::value() const{
  return _value;
}

/////////////////////////////////////
// Switch2Pos

Switch2Pos::Switch2Pos(uint8_t pin){
  _pin = pin;
  pinMode(_pin, INPUT);
  digitalWrite(_pin, HIGH);  

  _value = 0;
}

void Switch2Pos::read(){
  _value = digitalRead(_pin);
}

bool Switch2Pos::value() const{
  return _value;
}

//////////////////
// PushButton
#define BUTTON_DEBOUNCE_TIME  25
#define PUSH_LONG_INTERVAL    3000

#define BUTTON_STATE_OFF            0x00
#define BUTTON_STATE_PUSHED_SHORT   0x01
#define BUTTON_STATE_PUSHED_LONG    0x02
#define BUTTON_STATE_PUSHED_WAIT    0x03


PushButton::PushButton(uint8_t pin){
  _pin            = pin;
   pinMode(_pin, INPUT_PULLUP);
   digitalWrite(_pin, HIGH);  
  _value          = digitalRead(_pin); 
  _valueOff       = digitalRead(_pin); //Remember initial state
  _state          = BUTTON_STATE_OFF; 
  _millis         = 0;
  _millisDebounce = 0;
}

PushButton::~PushButton(){
}

//Pushed state - long push, repeats every three second
//Clicked state - push and release, within less than three second

void PushButton::read(){  

  //Check last saved buttone value
  if(_value == _valueOff){

    if(_state != BUTTON_STATE_OFF){ 
      //Reset everything
       _millis = 0;
       _state  = BUTTON_STATE_OFF;  
    }
  }

  //Analyse current state
  switch(_state){
    case BUTTON_STATE_OFF: //off      
    
      //Check if just pressed
      if(_value == !_valueOff){
        //Be ready for short push
        _state = BUTTON_STATE_PUSHED_SHORT;
        //Set timer
        SET_MILLIS(_millis); 
        DBG_OUTLN("pushed - button D%d", (int)_pin);
      }
    break;
    case BUTTON_STATE_PUSHED_SHORT: //Short push
    case BUTTON_STATE_PUSHED_WAIT:  //Wait for next interval

       //Check timer
      if(DELTA_MILLS(_millis) >= PUSH_LONG_INTERVAL){
        _state = BUTTON_STATE_PUSHED_LONG;
        //Reset the timer so next long push event happens
        SET_MILLIS(_millis);       
      }
    break;
    case BUTTON_STATE_PUSHED_LONG:
        
      //Reset long push
      _state = BUTTON_STATE_PUSHED_WAIT;
    break;
  }
  

  //Read signal
  readValue();
}

void PushButton::readValue(){
   //Read button state
  uint8_t value = digitalRead(_pin);
  
  //Debouncing
  if(_value != value){

    if(_millisDebounce == 0){
      //Wait for some time
      SET_MILLIS (_millisDebounce);
    }
    else{    
       //Change if the value is the same  
      if(DELTA_MILLS(_millisDebounce) >= BUTTON_DEBOUNCE_TIME){
        _value = value;
      }
    }
  }
  else{
    //Reset
    _millisDebounce = 0;
  }
}

bool PushButton::pushedLong() const{
  //Pushed for long time
  return _state == BUTTON_STATE_PUSHED_LONG ? true : false;
}

bool PushButton::clickedShort() const{
  //Was pushed for short period of time and than released
  return _value == _valueOff && _state == BUTTON_STATE_PUSHED_SHORT? true : false;
}


bool PushButton::clickedLong() const{
  //Was pushed for long period of time and than released
  return _value == _valueOff && (_state == BUTTON_STATE_PUSHED_WAIT || _state == BUTTON_STATE_PUSHED_LONG) ? true : false;
}

bool PushButton::value(uint8_t ctrl) const{
  switch(ctrl){
    case PB_CONTROL_CLICK_SHORT:
      return clickedShort();
    break; 
    case PB_CONTROL_CLICK_LONG:
      return clickedLong();
    break;
    case PB_CONTROL_CLICK:
      return clickedShort() || clickedLong();
    break;
    case PB_CONTROL_PUSH_LONG:
      return pushedLong();
    break;
  };
  
 return false;
}

#ifdef USE_IR_REMOTE

#include <IRremote.hpp>

//////////////////////////
// IRRemoteRecv
#define REMOTE_TIMEOUT      10
#define REMOTE_REPEAT_LIMIT 127

IRRemoteRecv::IRRemoteRecv(uint8_t pin) {
  IrReceiver.begin(pin, ENABLE_LED_FEEDBACK);

  _millis = 0;
  _value  = 0;
  _repeat = 0;
  _pushed = false;
}

IRRemoteRecv::~IRRemoteRecv(){
}


void IRRemoteRecv::read(){
  
   //Reset if necessary
   if(pushed(_value)){
    _pushed = false;
   }

  //Check if we are waiting for receiver or for resuming receiver
  if (_millis == 0) { 

    //Stupid workaround for ws2811, ws2812 and ws2812b where internal clock conflicts with interrups required for IR receiver
    //See https://github.com/Fa9876rdstLED/FastLED/issues/198
    while (!IrReceiver.isIdle());
 
    //Now go and get IR signals
    if (IrReceiver.decode()) {
      
      // Remember time when reset IR
      SET_MILLIS(_millis);

      //Remember value
      _pushed = true;      

      if(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT && _repeat < REMOTE_REPEAT_LIMIT){
        _repeat ++;
      }
      else{
        _value  = IrReceiver.decodedIRData.command;
        _repeat = 1;
      }

     // Receiving  next value   
     IrReceiver.resume(); 
    }
  }
  else {
    if (DELTA_MILLS(_millis) >=  REMOTE_TIMEOUT){
      // Receiving  next value        
      IrReceiver.resume();
      // Reset timer
      _millis = 0;      
    }
    
  }
}


uint8_t IRRemoteRecv::pushed(unsigned long key) const{
  if(_pushed == true && (_value == key || key == R_KEY_ANY)){
    return _repeat;
  }

  return 0;
}

unsigned long IRRemoteRecv::pushedBtn() const{
  return _pushed ? _value : 0;
}

#endif //USE_IR_REMOTE


//////////////////////////////////////
// RotaryEncodder
RotaryEncoder::RotaryEncoder(uint8_t pinData, uint8_t pinClock){
  _pinData      = pinData;
  _pinClock     = pinClock;
  _value        = 0;
  _valClock    = HIGH;
  
  pinMode (_pinData, INPUT);
  pinMode (_pinClock, INPUT); 
}


RotaryEncoder::~RotaryEncoder(){
}

// _value can be one of the following: 0 (1 up), 1 (no change), 2 (1 down)

void RotaryEncoder::read(){    
  
 //See where the clock is
 int valClock = digitalRead(_pinClock);

 //Read pin only when clock is changing from LOW to HIGH
 if((_valClock == LOW) && (valClock == HIGH)) {
      //Now we can read data
     _value = digitalRead(_pinData) == LOW ? 2 : 0 ;     
  } 
  else {
    _value = 1;
  }

 //Remember clock
 _valClock = valClock;
}

int RotaryEncoder::value() const{  
  return (int)1 - (int)_value;
}
