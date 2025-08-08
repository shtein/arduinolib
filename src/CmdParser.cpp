#include "CmdParser.h"
#include "DbgTool.h"


//////////////////////////////////////////
//Parser Helpers

//List of tokens
bool getTokens(char *str, const char *tokens[], size_t maxTokens, char separator, char escape){
  //No data
  if(!str){
    return false;
  }

  //Too many tokesn
  if(maxTokens == 0){
    return false;
  }  

  //Find begin of the token, replace separators by 0x00
  for( ; *str != 0x00 && *str == separator; str++){  
    *str = 0x00;
  }  

  //Cut leading escape
  if(escape && *str == escape){
    str ++;
  }
  
  tokens[0] = str;
  str++;
 
  //Find end of the token
  for( ; *str != 0x00 && *str != separator; str++){
    if(escape && *str == escape){
        memmove(str, str + 1, strlen(str));
        str ++;
    }
  }
  
  return *str == 0x00 ? true : getTokens(str, &tokens[1], maxTokens - 1, separator, escape); 
}

#define COMMAND_SEPARATOR  '|'

//Check if tokens match to specified token
bool checkTokenMatch(const char *token, const char *token2){
  char match[32];
  strncpy_P(match, token2, sizeof(match));  

  //Remember length of the token
  size_t lenToken = token ? strlen(token) : 0;
  size_t lenMatch = 0;

  for(char *begin = match, *end = strchr(begin, COMMAND_SEPARATOR);  
      begin != NULL; 
      begin = end, end = begin ? strchr(begin, COMMAND_SEPARATOR) : NULL){   

    //Calculate length of the matching string
    lenMatch = end ? end - begin : strlen(begin);

    //Special case - no string, i.e. default value
    if(lenMatch == 0 && lenToken == 0)
      return true;

    //Look for full case insensitive  match, length and symbols
    if(lenMatch == lenToken && strncasecmp(token, begin, lenMatch) == 0){
      return true;
    }

    //Skip separator if still there
    if(end)
      end ++;
  }

  return false;
}

const char *getValueAfterToken(const char *tokens[], const char *match){

  for(size_t i = 0; tokens[i] != NULL; i++){
    if(checkTokenMatch(tokens[i], match))
      return tokens[i + 1];
  }

  return NULL;
}

bool strTo(const char *str, int &n){
  if(!str)
    return false;  

  n = atoi(str);

  return true;
}

bool strTo(const char *str, char *dest){
  if(!str)
    return false;

  if(!dest)
    return false;

  strcpy(dest, str);

  return true;
}


