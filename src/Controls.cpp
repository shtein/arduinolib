#include "arduinolib.h"

#include <Arduino.h>
#include "Controls.h"
#include "AnalogInput.h"
#include "DbgTool.h"




////////////////////////////
// CtrlItem
CtrlItem::CtrlItem(uint8_t cmd, BaseInput *input) {
   _input = input;
   _cmd   = cmd;
}

CtrlItem::~CtrlItem(){
}

BaseInput *CtrlItem::getInput() const{
  return _input;
}

void CtrlItem::loop(CtrlQueueItem &itm){
  //Check if triggered
  if(triggered()){

    //Retrieve data    
    getData(itm.data);

    //Set command    
    itm.cmd = _cmd;    
  }
}

////////////////////////////

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

#define PARSE_DELIMETER    " "
#define COMMAND_SEPARATOR  '|'

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

//Check if tokens match to specified token
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

const char *findTokenValue(char *tokens[], const char *match){

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



////////////////////////////
// CtrlPanel
CtrlPanel::CtrlPanel(){
  _numControls = 0;
  _controlNum  = 0;
  _numInputs   = 0;
}

CtrlPanel::~CtrlPanel(){
}

void CtrlPanel::addControl(CtrlItem *ctrl){
  if(!ctrl)
    return;

  if(_numControls == sizeof(_controls)/sizeof(_controls[0]))
    return;

  //Add control
  _controls[_numControls] = ctrl;
  _numControls ++;

  
  //Add analog input - should be only of instance to avoid reading it twice
  BaseInput *input = ctrl->getInput();
  if(!input)
    return;

  //Find if input is already there
  bool found = false;
  for(int i = 0; i < _numInputs; i++){
    if(input == _inputs[i]){
      found = true;
      break;
    }
  }

  //Save if not found
  if(!found){
    
    //Check number of inputs
    if(_numInputs == sizeof(_inputs)/sizeof(_inputs[0])) 
      return;
        
    _inputs[_numInputs] = input;
    _numInputs ++;
  }
}

void CtrlPanel::loop(CtrlQueueItem &itm){
  //Reset cmd
  itm.cmd       = EEMC_NONE;
  itm.data.flag = CTF_NONE;
  
  if(_controlNum >= _numControls){
     _controlNum = 0;
    
    //Read inputs
    for(int i = 0; i < _numInputs; i++){
      _inputs[i]->read();
    }
     
    //Send idle command once in a while
    return;
  }

  //process current control
  _controls[_controlNum]->loop(itm);

  //Go with next control next time
  _controlNum ++;
}

