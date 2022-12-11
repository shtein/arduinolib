#ifndef __CONTROLS_H
#define __CONTROLS_H

#include "ControlCtx.h"
#include "AnalogInput.h"


//Change commands
#define EEMC_NONE        0x00   //Nothing changed
#define EEMC_ERROR       0xFF   //Input error

///////////////////////////////////
// Control flags
#define CTF_NONE        0x00  //Nothing, value is absolute number
#define CTF_VAL_ABS     0x00  //Absolute number
#define CTF_VAL_BOOL    0x01  //Value is bool
#define CTF_VAL_DELTA   0x02  //Value is delta
#define CTF_VAL_NEXT    0x03  //Go next - in cycles
#define CTF_VAL_PREV    0x04  //Go previous - in cycles




////////////////////////////////////
// Control queue data
struct CtrlQueueData{
  uint8_t flag;   //Flag that shows how to interpret the value: absolute number, inrement, etc 
  int     value;  //Value
  int     min;    //Value minimum
  int     max;    //Value maximum   
  
  CtrlQueueData(){
      flag   = CTF_NONE;
      value  = 0;
      min    = 0;
      max    = 0;
  }

  int translate(int base, int vmin, int vmax) const{
    
     switch(flag){ 
      case CTF_VAL_ABS:  //Absolute value
        base = min == max ? value : map(value, min, max, vmin, vmax);
        if(base < vmin)      base = vmin;
        else if(base > vmax) base = vmax;
      break;
      case CTF_VAL_NEXT: //Go next
        base ++;
        if(base > vmax) base = vmin;
      break;
      case CTF_VAL_PREV: //Go Prev
        base --;
        if(base < vmin) base = vmax;
      break;
      case CTF_VAL_DELTA://Delta
        base += value;
        if(base < vmin)      base = vmin;
        else if(base > vmax) base = vmax;
      break;
    }

    return base;
  }
};


////////////////////////////////////
// Control queue element
struct CtrlQueueItem {
  uint8_t       cmd;    // Command
  CtrlQueueData data;   // Data

  CtrlQueueItem(){
     cmd = 0;
  }
};






//////////////////////////////////////////
// ProcessControl - base class'

class CtrlItem{
  public:
    CtrlItem(uint8_t cmd, BaseInput *input);
   ~CtrlItem();

    void loop(CtrlQueueItem &itm);    
    BaseInput *getInput() const;

  protected:
    virtual bool triggered() const = 0;
    virtual void getData(CtrlQueueData &data) = 0;

  protected:
    uint8_t       _cmd;     //Command
    BaseInput    *_input;   //Analog input to retrieve control data
};

////////////////////////////////
// Push button control, reacts on either short click, long click or long push

template <const uint8_t CTRL = PB_CONTROL_CLICK,  const uint8_t FLAG = CTF_VAL_NEXT>
class CtrlItemPb: public CtrlItem{
  public:
    CtrlItemPb(uint8_t cmd, PushButton *input, int8_t value = 0):
      CtrlItem(cmd, input){
      _value = value;
    }

  protected:
    bool triggered() const{
      return ((PushButton *)getInput())->value(CTRL);
    }
    void getData(CtrlQueueData &data){
      data.flag  = FLAG;
      data.value = _value;
      data.min   = 0;
      data.max   = 0;
    }

  protected:
    int8_t  _value;
};

////////////////////////////////
// CtrlItemPtmtr - analog input is Potentiometer - AnalogInput
#define POT_MIN             0
#define POT_MAX             1023
#define POT_LOWER_MARGIN    10
#define POT_UPPER_MARGIN    10

#define POT_NOISE_THRESHOLD 3

#define POT_MARGIN_MAX          500
#define POT_NOISE_THRESHOLD_MAX 16

#define alfa 0.5


template <const uint16_t NOISE_THRESHOLD = POT_NOISE_THRESHOLD, 
         const uint16_t LOWER_MARGIN = POT_LOWER_MARGIN,
         const uint16_t UPPER_MARGIN = POT_UPPER_MARGIN >
class CtrlItemPtmtr: public CtrlItem{
  public:
    CtrlItemPtmtr(uint8_t cmd, AnalogInput *ptn):
      CtrlItem(cmd, ptn){
     _value          = POT_MAX; //just to make sure it is different from what we read
    }

  protected:
    bool triggered() const{
      uint16_t value = getValue(); 
      return (abs(value - _value) >  min(NOISE_THRESHOLD, 
                                         min(value - POT_MIN + LOWER_MARGIN, POT_MAX - UPPER_MARGIN - value)
                                        )
             ); 
    }

    void getData(CtrlQueueData &data){
      _value  = getValue();
      
      data.flag  = CTF_VAL_ABS;
      data.min   = POT_MIN + LOWER_MARGIN;
      data.max   = POT_MAX - UPPER_MARGIN;
      data.value = _value;

      if(data.value < data.min)
        data.value = data.min;
      else if (data.value > data.max)
        data.value = data.max;
    }

    uint16_t getValue() const{
      uint16_t value = _value * alfa + (1 - alfa) * ((AnalogInput *)getInput())->value() + 0.5;

      if(value < LOWER_MARGIN)
        value = LOWER_MARGIN;

      if(value > POT_MAX - UPPER_MARGIN)
        value = POT_MAX - UPPER_MARGIN;

      return value;
    }

  protected:
   uint16_t  _value;
};

///////////////////////////////
// CtrlSwicth2Pos - two position swicth - digital input

class CtrlSwicth2Pos: public CtrlItem{
  public:
    CtrlSwicth2Pos(uint8_t cmd, Switch2Pos *sw):
      CtrlItem(cmd, sw){        
      //Make sure first trigger works
      _value = !((Switch2Pos *)getInput())->value();
    }

  protected:
    bool triggered() const{
      return _value != ((Switch2Pos *)getInput())->value();
    }

    void getData(CtrlQueueData &data){
      _value = ((Switch2Pos *)getInput())->value();

      data.flag  = CTF_VAL_ABS;
      data.value = _value;
      data.min   = 0;
      data.max   = 0;
    }

  protected:
    bool _value; 
};


#ifdef USE_IR_REMOTE

//////////////////////////////
// CtrlItemIRBtn - analog input is one IR remote buttons
// Returns returns delta
// dir - direction (true is positive, false is negative)
// repeat - button repeat limit, 0 = single push, same as next or prev

template<const unsigned long BTN, const bool DIR = TRUE, const uint8_t REPEAT = 0>
class CtrlItemIRBtn: public CtrlItem{
  public:
    CtrlItemIRBtn(uint8_t cmd, IRRemoteRecv *ir):
      CtrlItem(cmd, ir){

    }

    ~CtrlItemIRBtn();

   protected:
    bool triggered() const{
      int n = ((IRRemoteRecv *)getInput())->pushed(BTN);

      //Not pushed
      if(n == 0) {
        return false;
      }
  
      //Single click button
      if(n > 1 && REPEAT == 0){
        return false;
      }
  
      //Pushed
      return true;
    }

    void getData(CtrlQueueData &data){
      data.flag  = REPEAT > 0 ? CTF_VAL_DELTA : (DIR ? CTF_VAL_NEXT: CTF_VAL_PREV ); 
      data.value = (DIR ? 1 : -1) * powInt(2, ((IRRemoteRecv *)getInput())->pushed(BTN) - 1, REPEAT);
      data.min   = 0;
      data.max   = 0;
    }
};

#endif //USE_IR_REMOTE

////////////////////////////////////////
// Rotary encoder control
// Always returns incremental/decremental value

#define ROTECT_DEFAULT_INC 10 //Default incremement value

template <const uint8_t INC = ROTECT_DEFAULT_INC>
class CtrlItemRotEnc: public CtrlItem{
  public:
    CtrlItemRotEnc(uint8_t cmd, RotaryEncoder *re):
      CtrlItem(cmd, re){        
    }

  protected:
    bool triggered() const{
      return ((RotaryEncoder *)getInput())->value() != 0;
    }

    void getData(CtrlQueueData &data){
       data.flag  = CTF_VAL_DELTA;
      data.value = ((RotaryEncoder *)getInput())->value() * INC;  
      data.min   = 0;
      data.max   = 0;
    } 
};




////////////////////////////////////////
// EffectControlPanel

#ifndef MAX_CONTROLS
  #define MAX_CONTROLS 20
#endif
#ifndef MAX_INPUTS
  #define MAX_INPUTS   10
#endif

class CtrlPanel{
  public:
    CtrlPanel();
   ~CtrlPanel();

    void addControl(CtrlItem *ctrl);

    void loop(CtrlQueueItem &itm);

  protected:
    CtrlItem      *_controls[MAX_CONTROLS]; //Controls
    uint8_t        _numControls;            //Number of controls
    uint8_t        _controlNum;             //Last processed control

    BaseInput    *_inputs[MAX_INPUTS];     //Analog inputs
    uint8_t       _numInputs;
};


#endif //__CONTROLS_H
