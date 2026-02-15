#include "Controls.h"
#include "AnalogInput.h"
#include "DbgTool.h"


////////////////////////////
// CtrlQueue

CtrlCmdQueue::CtrlCmdQueue(){
  clear();
}

void CtrlCmdQueue::clear(){
  _size  = 0; 
  _index = 0;
}

bool CtrlCmdQueue::add(const CtrlQueueItem &itm){
  //Check size
  if(_size == MAX_CTRL_QUEUE_SIZE)
    return false;

  //Find position
  uint8_t pos = (_index + _size) % MAX_CTRL_QUEUE_SIZE;

  //Copy item
  _queue[pos] = itm;

  //Increase size
  _size ++;  

  return true;
}

bool CtrlCmdQueue::pop(CtrlQueueItem &itm){
  //Check if there are items in a queue
  if(_size == 0)
    return false;

  //Retrieve item
  itm = _queue[_index];

  //Shrink size and move to the next index
  _size --;
  _index = (_index + 1) %  MAX_CTRL_QUEUE_SIZE;

  return true;
}

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

