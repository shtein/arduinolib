#ifndef __ANALOGINPUT_H
#define __ANALOGINPUT_H


#include "ControlCtx.h"

////////////////////////////
// BaseInput
class BaseInput {
  public:
    BaseInput();
    ~BaseInput();

    virtual void read() = 0;
};

//////////////////////////
// AnalogInput
class AnalogInput: public BaseInput{
  public:
    AnalogInput(uint8_t pin);
    ~AnalogInput();

    virtual void read();
    int value() const;

  protected:
    uint16_t  _pin:5;  
    uint16_t  _value:11;

};

/////////////////////////
// Simple on/off switch
class Switch2Pos: public BaseInput{
  public:
    Switch2Pos(uint8_t pin);

    virtual void read();
    bool value() const;

  protected:
    uint8_t _pin:4;
    uint8_t _value:4;
};


///////////////////////
// Simple push button
// Define pin
 
class PushButton: public BaseInput{
  public:
    PushButton(uint8_t pin);
    ~PushButton();

    void read();
        
    bool clickedShort() const;
    bool pushedLong() const;
    bool clickedLong() const;

    bool value(uint8_t ctrl) const;

  protected:
    void readValue();
    
  protected:
    uint8_t        _pin:4;
    uint8_t        _valueOff:1;
    uint8_t        _value:1; 
    uint8_t        _state:2;
    unsigned long  _millis;  
    unsigned long  _millisDebounce;  
};


///////////////////////////////////////
// Simple IR Receiver for Keyes remote
#ifdef USE_IR_REMOTE

//#include <IRremoteInt.h>

//Keys
#define RKEY_REPEAT 0xFFFFFFFF
#define RKEY_OK     0xFF02FD
#define RKEY_LEFT   0xFF22DD
#define RKEY_RIGHT  0xFFC23D
#define RKEY_UP     0xFF629D
#define RKEY_DOWN   0xFFA857
#define RKEY_0      0xFF4AB5
#define RKEY_1      0xFF6897
#define RKEY_2      0xFF9867
#define RKEY_3      0xFFB04F
#define RKEY_4      0xFF30CF
#define RKEY_5      0xFF18E7
#define RKEY_6      0xFF7A85
#define RKEY_7      0xFF10EF
#define RKEY_8      0xFF38C7
#define RKEY_9      0xFF5AA5
#define RKEY_STAR   0xFF42BD
#define RKEY_HASH   0xFF52AD


class IRRemoteRecv: public BaseInput {
  public:
    IRRemoteRecv(uint8_t pin);
    ~IRRemoteRecv();

    void read();
    int pushed(unsigned long key) const;

  protected:
    IRrecv         _recv;          
    unsigned long  _millis;
    unsigned long  _value; 
    uint16_t       _repeat:7;
    uint16_t       _pushed:1;
};

#endif //USE_IR_REMOTE

////////////////////////////
//RotaryEncoder

class RotaryEncoder: public BaseInput {
  public:
    RotaryEncoder(uint8_t pinData, uint8_t pinClock);
    ~RotaryEncoder();

    void read();
    int value() const;

  protected:
    uint16_t  _pinData:5;  
    uint16_t  _pinClock:5;  
    uint16_t  _value:5;
    uint16_t  _valClock:1;
};


//////////////////////////////
//Input from serial port
#define SI_CR          '\r'
#define SI_LF          '\n'
#define SI_BS          '\b'
#define SI_NULLCHAR    '\0'
#define SI_SPACE       ' '
#define SI_BUFF_LEN     32


class SerialInput: public BaseInput{
  public:
    SerialInput();
    ~SerialInput();

    void read();
    bool isReady() const;
    char *getCommandLine();
    void reset();

  protected:   
    char    _bufRead[SI_BUFF_LEN + 1]; 
    uint8_t _lenRead; 
};


#endif //__ANALOGINPUT_H
