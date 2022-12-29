#include "arduinolib.h"

#include <Arduino.h>
#include "AnalogInput.h"
#include "CtrlSerial.h"
#include "DbgTool.h"

//Support for ESP8266, ESP32 etc
#if defined(ESP8266) || defined(ESP32)

const char * 	strchr_P (const char *p, int val){
  if(!p)
    return NULL;

  return (const char *)memchr_P(p, val, strlen_P(p)); 
}


#endif



//////////////////////////////////////////
//Parser Helpers

//Get tokens into from command line into array
//Return false of numnber of tokens is more than number of elements in array
bool getTokens(char *cmdLine, char *tokens[], size_t maxTokens){

  //Make sure there data
  if(!cmdLine)
    return false;

  //Convert cmdline to string array
  char *token = strtok(cmdLine, PARSE_DELIMETER);
  for(size_t i = 0; token != NULL; i++){    
    
    //Save only if size allows
    if(i < maxTokens){
      //Remeber token
      tokens[i] = token;
    }  
    else {
      //Too many tokens
      return false;
    }

    token = strtok(NULL, PARSE_DELIMETER);
  }

  return true;
}


bool checkTokenMatch(const char *token, const char *match){

  //Remember length of the token
  size_t lenToken = token ? strlen(token) : 0;
  size_t lenMatch = 0;

  for(const char *begin = match, *end = strchr_P(begin, COMMAND_SEPARATOR); 
      begin != NULL; 
      begin = end, end = begin ? strchr_P(begin, COMMAND_SEPARATOR) : NULL){   

    //Calculate length of the matching string
    lenMatch = end ? end - begin : strlen_P(begin);

    //Special case - no string, i.e. default value
    if(lenMatch == 0 && lenToken == 0)
      return true;

    //Look for full case insensitive  match, length and symbols
    if(lenMatch == lenToken && strncasecmp_P(token, begin, lenMatch) == 0){
      return true;
    }

    //Skip separator if still there
    if(end)
      end ++;
  }


  return false;
}

bool strToInt(const char *str, int &n){
  if(!str)
    return false;

  n = atoi(str);

  return true;
}
