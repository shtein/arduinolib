#ifndef __CONTROLS_H
#define __CONTROLS_H

#include "ControlCtx.h"
#include "AnalogInput.h"
#include "utils.h"

//Change commands
#define EEMC_NONE        0x00   //Nothing changed
#define EEMC_ERROR       0xFF   //Input error

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

#if defined(ESP8266) || defined(ESP32)
#define MAX_STR_VALUE 128
#else
#define MAX_STR_VALUE 16
#endif

////////////////////////////////////
// Control queue data
struct CtrlQueueData{
  uint8_t flag;   //Flag that shows how to interpret the value: absolute number, inrement, etc 
  union{
    int   value;              //integer value
    char  str[MAX_STR_VALUE]; //string value or packed object
  };
  int     min;    //Value minimum
  int     max;    //Value maximum   
  
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
struct CtrlQueueItem {
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

template <const uint8_t CTRL = PB_CONTROL_CLICK,  const uint8_t FLAG = CTF_VAL_NEXT, const uint8_t VALUE = 0>
class CtrlItemPb: public CtrlItem{
  public:
    CtrlItemPb(uint8_t cmd, PushButton *input):
      CtrlItem(cmd, input){
    }

  protected:
    bool triggered() const{
      return ((PushButton *)getInput())->value(CTRL);
    }
    void getData(CtrlQueueData &data){
      data.flag  = FLAG;
      data.value = VALUE;
      data.min   = 0;
      data.max   = 0;
    }
};

////////////////////////////////
// CtrlItemPtmtr - analog input is Potentiometer - AnalogInput

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
      int16_t value = (int16_t)getValue(); 
      
      return (abs(value - (int16_t)_value) >  min(NOISE_THRESHOLD, 
                                                  (uint16_t)min(value - POT_MIN + LOWER_MARGIN, POT_MAX - UPPER_MARGIN - value)
                                                )
             );     
    }

    void getData(CtrlQueueData &data){
      _value  = getValue();
      
      data.flag  = CTF_VAL_ABS;
      data.min   = POT_MIN + LOWER_MARGIN;
      data.max   = POT_MAX - UPPER_MARGIN;
      data.value = _value;
    }

    uint16_t getValue() const{    

      uint16_t value = ( _value + ((AnalogInput *)getInput())->value() ) / 2;
      if(_value > value && value > 0)
        value -= 1;
      else if(_value < value)
        value += 1;


      return value < POT_MIN + LOWER_MARGIN ? 
                       POT_MIN + LOWER_MARGIN : value > POT_MAX - UPPER_MARGIN ? 
                        POT_MAX - UPPER_MARGIN : value;

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

template<const unsigned long BTN, const bool DIR = true, const uint8_t REPEAT = 0>
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


/////////////////////////////////////////
// Multi-command interface
template<class INP, class NTF, uint8_t (*PARSER) (char *cmdLine, CtrlQueueData &data)>
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
      char *cmdLine = ((INP *)_input)->getCommandLine();

      if( cmdLine ){
        //Buffer is ready
        _cmd = PARSER(cmdLine, data);     
        cmdLine[0] = 0;
      }
      else {
        _cmd = EEMC_NONE;
      } 
    }    
};


//Helpers for creating parser function for SerialInput 
//Helper functions for pasrer
bool getTokens(char *cmdLine, char *tokens[], size_t maxTokens);
bool checkTokenMatch(const char *token, const char *match);
const char *getValueAfterToken(char *tokens[], const char *match);
bool strTo(const char *str, int &n);
bool strTo(const char *str, char *dest); 


//Helper macros
#define MAX_TOKENS         10
#define MAX_COMMAND_LEN    32


//Set corresponding data members of CtrlQueueData
#define _CQD_SET_NONE(data, ...) 
#define _CQD_SET_CMD(data, ...) return ARG_NUM_1(__VA_ARGS__);
#define _CQD_SET_VAL(data, ...) data.setValue(ARG_NUM_2(__VA_ARGS__)); _CQD_SET_CMD(data, __VA_ARGS__);
#define _CQD_SET_FLG(data, ...) data.flag  = ARG_NUM_3(__VA_ARGS__); _CQD_SET_VAL(data, __VA_ARGS__);
#define _CQD_SET_MIN(data, ...) data.min   = ARG_NUM_4(__VA_ARGS__); _CQD_SET_FLG(data, __VA_ARGS__);
#define _CQD_SET_MAX(data, ...) data.max   = ARG_NUM_5(__VA_ARGS__); _CQD_SET_MIN(data, __VA_ARGS__);

//Parameter sequence is cmd, data.value, data.flag, data.min, data.max
#define _CQD_SET_DATA(data, ...) ARG_NUM( NUM_ARGS(data, ##__VA_ARGS__), _CQD_SET_NONE, _CQD_SET_CMD, _CQD_SET_VAL, _CQD_SET_FLG, _CQD_SET_MIN, _CQD_SET_MAX) (data, __VA_ARGS__)

#define BEGIN_PARSE_ROUTINE(FunctionName) \
uint8_t FunctionName(char *cmdLine, CtrlQueueData &data){ \
  char  *tokens[MAX_TOKENS + 1]; \
  memset(tokens, 0, sizeof(tokens)); \
  if(!getTokens(cmdLine, tokens, MAX_TOKENS)){ \
    return EEMC_ERROR; \
  } \
  int8_t index = -1;

#define END_PARSE_ROUTINE() \
  return EEMC_ERROR; \
}

#define _IF_TOKEN_MATCH(a, b) if(checkTokenMatch(a, PSTR(b)) ) \

#define _BEGIN_TOKEN(token) \
  index ++; \
  _IF_TOKEN_MATCH(tokens[index], token){ 
    
    
#define _END_TOKEN() \
  } \
  index --;

//Group token
#define BEGIN_GROUP_TOKEN(token, ...) \
  _BEGIN_TOKEN(token) \
    if(!tokens[index + 1]) { \
      _CQD_SET_DATA(data, ##__VA_ARGS__) \
    }
                
#define END_GROUP_TOKEN() _END_TOKEN()
    
//Final token, nothing after it, it is a fixed string    
#define VALUE_IS_TOKEN(token, cmd, ...) \
  _BEGIN_TOKEN(token) \
    if(!tokens[index + 1]) { \
        _CQD_SET_DATA(data, cmd, ##__VA_ARGS__) \
    } \
  _END_TOKEN()

//Final, nothing after it, it is a number
#define VALUE_IS_NUMBER(cmd, ...) \
  index ++; \
  if(!tokens[index + 1]) { \
    int n; \
    if(strTo(tokens[index], n)) { \
      _CQD_SET_DATA(data, cmd, n, ##__VA_ARGS__) \
    } \
  } \
  index --;  

//Final, nothing after it, it is a string
#define VALUE_IS_STRING(cmd) \
  index ++; \
  if(!tokens[index + 1]) { \
    _CQD_SET_DATA(data, cmd, tokens[index], CTF_VAL_STRING) \
  } \
  index --;


//Final, nothing after it, there are token and value
#define VALUE_IS_PAIR(token, cmd, flag, ...) \
  _BEGIN_TOKEN(token) \
  if(flag & CTF_VAL_STRING){ \
    VALUE_IS_STRING(cmd) \
  } \
  else {\
    VALUE_IS_NUMBER(cmd, ##__VA_ARGS__) \
  } \
  _END_TOKEN()

// Final, nothing after that, there are pairs token and value that correspond to object data members 
#define BEGIN_OBJECT(token, object, cmd) \
  _BEGIN_TOKEN(token) \
  object obj; \
  memset(&obj, 0, sizeof(obj)); \
  uint8_t command = cmd;

#define END_OBJECT() \
  _CQD_SET_DATA(data, command, obj, CTF_VAL_OBJECT) \
  _END_TOKEN();


#define _DM_DEFAULT_NONE(value, ...) 
#define _DM_DEFAULT_SPEC(value, ...) value = ARG_NUM_1(__VA_ARGS__);
#define _DM_DEFAULT(value, ...) ARG_NUM( NUM_ARGS(value, ##__VA_ARGS__), _DM_DEFAULT_NONE, _DM_DEFAULT_SPEC)(value, __VA_ARGS__)

#define _DM_MANDATORY_FALSE()
#define _DM_MANDATORY_TRUE() return EEMC_ERROR;
#define _DM_MANDATORY(...) ARG_NUM( NUM_ARGS(_, ##__VA_ARGS__), _DM_MANDATORY_TRUE, _DM_MANDATORY_FALSE)()


#define _DM_AS(token, member, format, ...) \
  { \
    const char *v = getValueAfterToken(&tokens[index + 1], PSTR(token)); \
    if(!format(v, obj.member)){  \
      _DM_MANDATORY(__VA_ARGS__) \
      _DM_DEFAULT(obj.member, ##__VA_ARGS__); \
    } \
  }

//Key value pairs. Third macro parameter is default value, if not defined this member is mandatory

#define DATA_MEMBER(token, member, ...) _DM_AS(token, member, strTo, ##__VA_ARGS__)
#define DATA_MEMBER_AS_IP(token, member, ...) _DM_AS(token, member, strToIPAddr, ##__VA_ARGS__)



/*
Usage of parser macro:

BEGIN_PARSE_ROUTINE(YourFunctionName)
  BEGIN_GROUP_TOKEN("group_token_a")
    BEGIN_GROUP_TOKEN("group_token_a_1")
        VALUE_IS_TOKEN("token_a_1_1", cmd, value, flags, max, min)
        VALUE_IS_TOKEN("token_a_1_2", cmd, value, flags )
        VALUE_IS_NUMBER(cmd)
    END_GROUP_TOKEN()
    VALUE_IS_TOKEN("token_a_2_1", cmd)    
  END_GROUP_TOKEN()
  BEGIN_GROUP_TOKEN("group_token_b", cmd)
    ...
  END_GROUP_TOKEN()
END_PARSE_ROUTINE()
*/





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
