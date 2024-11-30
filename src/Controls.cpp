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
// CtrlItemIR
CtrlItemIR::CtrlItemIR(uint8_t (*IRCmdMap) (unsigned long, CtrlQueueData &), IRRemoteRecv *ir):
      CtrlItem(EEMC_NONE, ir){
        _IRCmdMap = IRCmdMap;
    }

bool CtrlItemIR::triggered() const{
  
  //See if anything is pushed
  return ((IRRemoteRecv *)getInput())->pushed() ? true : false;


/*
  //Not defined button
  IR_REMOTE_CMD itm;
  if(!_IRCmdMap(((IRRemoteRecv *)getInput())->pushedBtn(), itm)) {    
    return false;
  }

  //Single click button
  if(n > 1 && itm.repeat == 0){
    return false;
  }
*/
}

void CtrlItemIR::getData(CtrlQueueData &data){
  //Get pushed button  
  unsigned long btn = ((IRRemoteRecv *)getInput())->pushedBtn();

  //Retrieve cmd and data
  _cmd = _IRCmdMap(btn, data);
  //Do nothing if command is not defined
  if(_cmd == EEMC_NONE || _cmd == EEMC_ERROR){
    return;
  }

  //Get repeats counts
  uint8_t pushedCnt = ((IRRemoteRecv *)getInput())->pushed(btn);

  if(data.value == 0){ 
    //Don't process repeats for single click
    if(pushedCnt > 1){
      _cmd == EEMC_NONE;
    }
  }
  else if(data.flag == CTF_VAL_DELTA){
  // If it is positive or negative then it is power of 2^repeat 
  // where repeat  is limited by absolute value of data.avlue
    data.value = (data.value > 0 ? 1 : -1) * powInt(2, pushedCnt - 1, abs(data.value));
  }
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

