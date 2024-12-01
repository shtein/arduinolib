#ifndef __ANALOGINPUT_H
#define __ANALOGINPUT_H

//////////////////////////
// Input constants
#define PB_CONTROL_CLICK_SHORT 0x01
#define PB_CONTROL_CLICK_LONG  0x02
#define PB_CONTROL_CLICK       0x03
#define PB_CONTROL_PUSH_LONG   0x04
#define PB_CONTROL_ANY         0xFF

#define POT_MIN             0
#define POT_MAX             1023
#define POT_LOWER_MARGIN    10
#define POT_UPPER_MARGIN    10
#define POT_NOISE_THRESHOLD 3


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
    uint8_t   _pin:4;
    uint8_t   _valueOff:1;
    uint8_t   _value:1; 
    uint8_t   _state:2;
    uint16_t  _millis;  
    uint16_t  _millisDebounce;  
};


///////////////////////////////////////
// Simple IR Receiver for Keyes remote
#ifdef USE_IR_REMOTE

//Keys
//DEC codes
#define R_DEC_KEY_OK     0x40
#define R_DEC_KEY_LEFT   0x44
#define R_DEC_KEY_RIGHT  0x43
#define R_DEC_KEY_UP     0x46
#define R_DEC_KEY_DOWN   0x15
#define R_DEC_KEY_0      0x52
#define R_DEC_KEY_1      0x16
#define R_DEC_KEY_2      0x19
#define R_DEC_KEY_3      0x0D
#define R_DEC_KEY_4      0x0C
#define R_DEC_KEY_5      0x18
#define R_DEC_KEY_6      0x5E
#define R_DEC_KEY_7      0x08
#define R_DEC_KEY_8      0x1C
#define R_DEC_KEY_9      0x5A
#define R_DEC_KEY_STAR   0x42
#define R_DEC_KEY_HASH   0x4A

#define R_KEY_ANY        0xFFFFFFFF


class IRRemoteRecv: public BaseInput {
  public:
    IRRemoteRecv(uint8_t pin);
    ~IRRemoteRecv();

    void read();
    uint8_t pushed(unsigned long key = R_KEY_ANY) const;
    unsigned long pushedBtn() const;

  protected: 
    uint16_t       _millis;
    unsigned long  _value; 
    uint8_t       _repeat:7;
    uint8_t       _pushed:1;
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




#endif //__ANALOGINPUT_H
