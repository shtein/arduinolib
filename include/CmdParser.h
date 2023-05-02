#ifndef __CMD_PARSER_H
#define __CMD_PARSER_H

#include "ControlCtx.h"
#include "utils.h"


//Helpers for creating parser function for multi-command input 
bool getTokens(char *str, const char *tokens[], size_t maxTokens, char separator, char escape);
bool checkTokenMatch(const char *token, const char *match);
const char *getValueAfterToken(const char * tokens[], const char *match);
bool strTo(const char *str, int &n);
bool strTo(const char *str, char *dest); 

inline void setValue(const char *src, char *dst){
  strcpy(dst, src);
}

template <typename T1, typename T2 >
inline void setValue(const T1 &src, T2 &dst){
  dst = src;
}


//Helper macros

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
uint8_t FunctionName(const char *src[], CtrlQueueData &data){ \
  const char **tokens = NULL;

#define END_PARSE_ROUTINE() \
  return EEMC_ERROR; \
}

#define _IF_TOKEN_MATCH(a, b) if(checkTokenMatch(a, PSTR(b)) ) \

#define _BEGIN_TOKEN(token) \
  tokens = tokens == NULL? src : tokens + 1; \
  _IF_TOKEN_MATCH(*tokens, token){ 
    
    
#define _END_TOKEN() \
  } \
  tokens = tokens == src ? NULL : tokens - 1;

//Group token
#define BEGIN_GROUP_TOKEN(token, ...) \
  _BEGIN_TOKEN(token) \
    if(!tokens[1]) { \
      _CQD_SET_DATA(data, ##__VA_ARGS__) \
    }
                
#define END_GROUP_TOKEN() _END_TOKEN()
    
//Final token, nothing after it, it is a fixed string    
#define VALUE_IS_TOKEN(token, cmd, ...) \
  _BEGIN_TOKEN(token) \
    if(!tokens[1]) { \
      _CQD_SET_DATA(data, cmd, ##__VA_ARGS__) \
    } \
  _END_TOKEN()

//Final, nothing after it, it is a number
#define VALUE_IS_NUMBER(cmd, ...) \
  tokens ++; \
  if(!tokens[1]) { \
    int n; \
    if(strTo(*tokens, n)) { \
      _CQD_SET_DATA(data, cmd, n, ##__VA_ARGS__) \
    } \
  } \
  tokens --;  

//Final, nothing after it, it is a string
#define VALUE_IS_STRING(cmd) \
  tokens ++; \
  if(!tokens[1]) { \
    _CQD_SET_DATA(data, cmd, *tokens, CTF_VAL_STRING) \
  } \
  tokens --;


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
#define _DM_DEFAULT_SPEC(value, ...) setValue(ARG_NUM_1(__VA_ARGS__), value);
#define _DM_DEFAULT(value, ...) ARG_NUM( NUM_ARGS(value, ##__VA_ARGS__), _DM_DEFAULT_NONE, _DM_DEFAULT_SPEC)(value, __VA_ARGS__)

#define _DM_MANDATORY_FALSE()
#define _DM_MANDATORY_TRUE() return EEMC_ERROR;
#define _DM_MANDATORY(...) ARG_NUM( NUM_ARGS(_, ##__VA_ARGS__), _DM_MANDATORY_TRUE, _DM_MANDATORY_FALSE)()


#define _DM_AS(token, member, format, ...) \
  { \
    const char *v = getValueAfterToken(&tokens[1], PSTR(token)); \
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


#endif //__CMD_PARSER_H