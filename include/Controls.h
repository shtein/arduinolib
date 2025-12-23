#ifndef __CONTROLS_H
#define __CONTROLS_H

#include "arduinolib.h"
#include "AnalogInput.h"
#include "alutils.h"
#include "DbgTool.h"
#include "CmdParser.h"

//Change commands
#define EEMC_NONE        0x00   //Nothing changed
#define EEMC_ERROR       0xFF   //Input error

//Errors
#define EEER_SUCCESS    0x00 //No error
#define EEER_INVALID    0x01 //Invalid input
#define EEER_UNHANDLED  0x02 //Unhandled command
#define EEER_DELAYED    0x03 //Delayed response


///////////////////////////////////
// Control flags
#define CTF_NONE        0x00  //Nothing
#define CTF_VAL_ABS     0x01  //Absolute number
#define CTF_VAL_BOOL    0x02  //Value is bool
#define CTF_VAL_DELTA   0x03  //Value is delta
#define CTF_VAL_NEXT    0x04  //Go next - in cycles
#define CTF_VAL_PREV    0x05  //Go previous - in cycles
#define CTF_VAL_STRING  0x06  //Value is string
#define CTF_VAL_OBJECT  0x07  //Value is object

#ifdef WIFI_ENABLED
  #define MAX_STR_VALUE 256
#else
  #define MAX_STR_VALUE 16
#endif

////////////////////////////////////
// Control queue data
struct __attribute__((packed)) CtrlQueueData{
  uint8_t flag;   //Flag that shows how to interpret the value: absolute number, inrement, etc 
  union{
    int16_t   value;              //integer value
    char      str[MAX_STR_VALUE]; //string value or packed object
  };
  int16_t     min;    //Value minimum
  int16_t     max;    //Value maximum   
  
  CtrlQueueData(){
      flag   = CTF_NONE;
      value  = 0;
      min    = 0;
      max    = 0;
  }

  void setValue(int n){ 
      flag  = CTF_VAL_ABS;
      value = n; 
  }

  void setValue(const char *p){ 
    flag  = CTF_VAL_STRING;
    strncpy(str, p, MAX_STR_VALUE); 
  }

  void setValue(char *p){ 
    flag  = CTF_VAL_STRING;
    strncpy(str, p, MAX_STR_VALUE); 
  }

  template<typename T>
  void setValue(const T &t){
    flag  = CTF_VAL_OBJECT;
    memcpy((void *)str, &t, (sizeof(T) < MAX_STR_VALUE) ? sizeof(T) : MAX_STR_VALUE); 
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
struct __attribute__((packed))CtrlQueueItem {
  uint8_t       cmd;    // Command
  CtrlQueueData data;   // Data

  CtrlQueueItem(){
     cmd = 0;
  }
};


//////////////////////////////////////////
// ProcessControl - base class

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
// It is a multi-command control
class CtrlItemPb: public CtrlItem{
  public:
    CtrlItemPb(uint8_t (*BTN_CMD_MAP) (uint8_t, CtrlQueueData &), PushButton *input):
      CtrlItem(EEMC_NONE, input){        
      _BTN_CMD_MAP = BTN_CMD_MAP;  
    }

  protected:
    bool triggered() const{
      return ((PushButton *)getInput())->value(PB_CONTROL_ANY);
    }
    void getData(CtrlQueueData &data){      
      //Check what triggered      
      PushButton *btn = (PushButton *)getInput();
      uint8_t ctrl    = btn->clickedShort() ? PB_CONTROL_CLICK_SHORT : (btn->clickedLong() ? PB_CONTROL_CLICK_LONG : (btn->pushedLong() ? PB_CONTROL_PUSH_LONG : PB_CONTROL_ANY));  

      //default flags
      data.flag = CTF_VAL_NEXT;

      //Retrieve data      
      _cmd = _BTN_CMD_MAP(ctrl, data);
    }

  protected:  
    uint8_t (*_BTN_CMD_MAP) (uint8_t, CtrlQueueData &);  
};

////////////////////////////////
// CtrlItemPtmtr - analog input is Potentiometer - AnalogInput

////////////////////////////////
// CtrlItemPtmtr - analog input is Potentiometer - AnalogInput
class CtrlItemPtmtr: public CtrlItem{
  public:
    CtrlItemPtmtr(uint8_t cmd, 
                  AnalogInput *ptn, 
                  uint8_t noise = POT_NOISE_THRESHOLD,
                  uint8_t lowerMargin = POT_LOWER_MARGIN, 
                  uint8_t upperMargin = POT_UPPER_MARGIN ):
      CtrlItem(cmd, ptn){
     _value       = POT_MAX; //just to make sure it is different from what we read
     _noise       = noise;
     _lowerMargin = lowerMargin;
     _upperMargin = upperMargin;
    }

  protected:
    bool triggered() const{
      int16_t value = (int16_t)getValue(); 
      
      return (abs(value - (int16_t)_value) >  min((uint16_t)_noise, 
                                                  (uint16_t)min(value - POT_MIN + _lowerMargin, POT_MAX - _upperMargin - value)
                                                )
             );     
    }

    void getData(CtrlQueueData &data){
      _value  = getValue();
      
      data.flag  = CTF_VAL_ABS;
      data.min   = getLower();
      data.max   = getUpper();
      data.value = _value;
    }

    uint16_t getValue() const{    

      uint16_t value = ( _value + ((AnalogInput *)getInput())->value() ) / 2;
      if(_value > value && value > 0)
        value -= 1;
      else if(_value < value)
        value += 1;


      return value < getLower() ? getLower() : value > getUpper() ? getUpper() : value;
    }

    uint16_t getLower() const{
      return POT_MIN + _lowerMargin;
    }

    uint16_t getUpper() const{
      return POT_MAX - _upperMargin;
    }
    
  protected:
    uint32_t  _value:11;
    uint32_t  _noise:5;
    uint32_t  _lowerMargin:8;
    uint32_t  _upperMargin:8;
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
// CtrlItemIR - analog input is  buttons
// 
// dir - direction (true is positive, false is negative)
// repeat - button repeat limit, 0 = single push, same as next or prev. 
// It is a multi-command control
// uint8_t (*IRCmdMap) (unsigned long, CtrlQueueData &) defines mapping of ir button to command and value

class CtrlItemIR: public CtrlItem{
  public:
    CtrlItemIR(uint8_t (*IR_CMD_MAP) (unsigned long, CtrlQueueData &),  
              IRRemoteRecv *ir): CtrlItem(EEMC_NONE, ir){
      _IR_CMD_MAP = IR_CMD_MAP;
    }

   protected:
    bool triggered() const{
        //See if anything is pushed
        return ((IRRemoteRecv *)getInput())->pushed() ? true : false;
    }

    void getData(CtrlQueueData &data){
      //Get pushed button  
      unsigned long btn = ((IRRemoteRecv *)getInput())->pushedBtn();

      //Retrieve cmd and data
      _cmd = _IR_CMD_MAP(btn, data);
      //Do nothing if command is not defined
      if(_cmd == EEMC_NONE || _cmd == EEMC_ERROR){
        return;
      }

      //Get repeats counts
      uint8_t pushedCnt = ((IRRemoteRecv *)getInput())->pushed(btn);

      if(data.value == 0){ 
        //Don't process repeats for single click
        if(pushedCnt > 1){
          _cmd = EEMC_NONE;
        }
      }
      else if(data.flag == CTF_VAL_DELTA){
      // If it is positive or negative then it is power of 2^repeat 
      // where repeat  is limited by absolute value of data.avlue
        data.value = (data.value > 0 ? 1 : -1) * powInt(2, pushedCnt - 1, abs(data.value));
      }
    }

  protected:
    uint8_t (*_IR_CMD_MAP) (unsigned long, CtrlQueueData &);

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


/////////////////////////////////////////
// Multi-command interface

#ifdef WIFI_ENABLED
  #define MAX_TOKENS         16
  #define MAX_COMMAND_LEN    128
#else
  #define MAX_TOKENS         8
  #define MAX_COMMAND_LEN    32
#endif


template<class INP, class NTF, uint8_t (*PARSER) (const char * tokens[], CtrlQueueData &data)>
class CtrlItemMultiCommand: public CtrlItem, public NTF{
  public:
    CtrlItemMultiCommand(INP *input):
      CtrlItem(EEMC_NONE, input){   
    }

  protected:
    // CtrlItem functions
    bool triggered() const{     
      return ((INP *)_input)->isReady(); 
    }

    void getData(CtrlQueueData &data){    
      const char *tokens[MAX_TOKENS + 1];
      memset(&tokens, 0, sizeof(tokens));

      if(!((INP *)_input)->getTokens(tokens, MAX_TOKENS) ){
        _cmd = EEMC_ERROR;
      }
      else if( tokens[0] !=  NULL){
        _cmd = PARSER(tokens, data);        
      }
      else {
        _cmd = EEMC_NONE;
      }  
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

    BaseInput    *_inputs[MAX_INPUTS];      //Analog inputs
    uint8_t       _numInputs;
};


#endif //__CONTROLS_H
