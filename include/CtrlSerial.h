#ifndef __CTRL_SERIAL_H
#define __CTRL_SERIAL_H

#include "Utils.h"
#include "Controls.h"
#include "Notification.h"

/////////////////////////////////////////
// Control from serial buffer
// Multi-command interface

class SerialInput;
typedef uint8_t (*FuncParseCmd_t) (char *cmdLine, CtrlQueueData &data);

class CtrlItemSerial: public CtrlItem, public NtfSerial {
  public:
    CtrlItemSerial(SerialInput *input, FuncParseCmd_t funcParse);
    ~CtrlItemSerial();

  protected:
    // CtrlItem functions
    bool triggered() const;
    void getData(CtrlQueueData &data);    

  protected:
    FuncParseCmd_t _funcParse;
};

//Helpers for creating parser function for SerialInput 

//Helper functions for pasrer
bool getTokens(char *cmdLine, char *tokens[], size_t maxTokens);
bool checkTokenMatch(const char *token, const char *match);
bool strToInt(const char *str, int &n);

//Helper macros
#define PARSE_DELIMETER    " "
#define COMMAND_SEPARATOR  '|'
#define MAX_TOKENS         5
#define MAX_COMMAND_LEN    32

//Set corresponding data members of CtrlQueueData
#define CQD_SET_NONE(data, ...) 
#define CQD_SET_CMD(data, ...) return ARG_NUM_1(__VA_ARGS__);
#define CQD_SET_VAL(data, ...) data.value = ARG_NUM_2(__VA_ARGS__); CQD_SET_CMD(data, __VA_ARGS__)
#define CQD_SET_FLG(data, ...) data.flag  = ARG_NUM_3(__VA_ARGS__); CQD_SET_VAL(data, __VA_ARGS__)
#define CQD_SET_MIN(data, ...) data.min   = ARG_NUM_4(__VA_ARGS__); CQD_SET_FLG(data, __VA_ARGS__)
#define CQD_SET_MAX(data, ...) data.max   = ARG_NUM_5(__VA_ARGS__); CQD_SET_MIN(data, __VA_ARGS__)

//Parameter sequence is cmd, data.value, data.flag, data.min, data.max
#define CQD_SET_DATA(data, ...) ARG_NUM( NUM_ARGS(data, ##__VA_ARGS__), CQD_SET_NONE, CQD_SET_CMD, CQD_SET_VAL, CQD_SET_FLG, CQD_SET_MIN, CQD_SET_MAX) (data, __VA_ARGS__)

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
        CQD_SET_DATA(data, ##__VA_ARGS__) \
      }
                
#define END_GROUP_TOKEN() _END_TOKEN()
    
//Final token, nothing after it, it is a fixed string    
#define TOKEN_IS_TEXT(token, cmd, ...) \
    _BEGIN_TOKEN(token) \
      if(!tokens[index + 1]) { \
         CQD_SET_DATA(data, cmd, ##__VA_ARGS__) \
      } \
    _END_TOKEN()

#define TOKEN_IS_NUMBER(cmd, ...) \
      index ++; \
      if(!tokens[index + 1]) { \
        int n; \
        if(strToInt(tokens[index], n)){ \
          CQD_SET_DATA(data, cmd, n, ##__VA_ARGS__) \
        } \
      } \
      index --;  



/*
Usage of serail parser:

BEGIN_PARSE_ROUTINE(YourFunctionName)
  BEGIN_GROUP_TOKEN("group_token_a")
    BEGIN_GROUP_TOKEN("group_token_a_1")
        TOKEN_IS_TEXT("token_a_1_1", cmd, value, flags, max, min)
        TOKEN_IS_TEXT("token_a_1_2", cmd, value, flags, )
        TOKEN_IS_NUMBER(cmd)
    END_GROUP_TOKEN()
    TOKEN_IS_TEXT("token_a_2_1")    
  END_GROUP_TOKEN()
  BEGIN_GROUP_TOKEN("group_token_b")
    ...
  END_GROUP_TOKEN()
END_PARSE_ROUTINE()

*/

#endif //__CTRL_SERIAL_H