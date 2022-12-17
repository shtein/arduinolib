#include <Arduino.h>
#include <DbgTool.h>
#include <AnalogInput.h>
#include <CtrlSerial.h>
#include <EEPROMCfg.h>



BEGIN_PARSE_ROUTINE(TestParse)

  BEGIN_GROUP_TOKEN("mode|m")  
    //mode set next[default]|prev|number
    BEGIN_GROUP_TOKEN("set|s")
        TOKEN_IS_TEXT("next|n|", 26, 1)
        TOKEN_IS_TEXT("prev|p", 27, 2)
        TOKEN_IS_NUMBER(25)
    END_GROUP_TOKEN()
    //mode get[default]      
    TOKEN_IS_TEXT("get|", 28)   
  END_GROUP_TOKEN()

  BEGIN_GROUP_TOKEN("effect|e")
    BEGIN_GROUP_TOKEN("set|s|")
        TOKEN_IS_TEXT("next|n|", 16, 1)
        TOKEN_IS_TEXT("prev|p", 17, 2)
        TOKEN_IS_NUMBER(15)
        TOKEN_IS_PAIR("color|c", 30)
    END_GROUP_TOKEN()
  END_GROUP_TOKEN();
  
END_PARSE_ROUTINE()



void putNtfObject(NtfBase &resp, const CtrlQueueData &data){
  resp.put_F(F("value"), data.value);
  resp.put_F(F("flag"), data.flag);
  resp.put_F(F("min"), data.min);
  resp.put_F(F("max"), data.min);
}

void putNtfObject(NtfBase &resp, const CtrlQueueItem &data){
  resp.put_F(F("cmd"), data.cmd);
  resp.put_F(F("data"), data.data);
}
  


template <typename ... ARGS>
void test(NtfBase *p, const ARGS&... args){
  CtrlQueueData data;
  p->put(NULL, args...);

}

#define MAX_NTF 3
typedef NtfBaseSet<MAX_NTF> NtfSet;


//////////////////////////////////////////
// ProcessControl - base class'

class CtrlItem1{
  public:
    void loop(CtrlQueueItem &itm){
      //Check if triggered
      if(triggered()){

        //Retrieve data    
        getData(itm.data);

        //Set command    
        itm.cmd = getCmd();   
      } 
      
    }

    virtual BaseInput *getInput() const = 0;

  protected:
    virtual bool triggered() const = 0;
    virtual void getData(CtrlQueueData &data) = 0;
    virtual uint8_t getCmd() const;
};

template<const uint8_t CMD>
class CtrlItemSC: public CtrlItem1{
  public:
    inline uint8_t getCmd() const __attribute__((always_inline)) { return CMD; }  
};



template <uint8_t CMD, const uint8_t CTRL = PB_CONTROL_CLICK,  const uint8_t FLAG = CTF_VAL_NEXT>
class CtrlItem1Pb: public CtrlItemSC<CMD>{
  public:
    CtrlItem1Pb(PushButton *input, int8_t value = 0){
      this->_ctrl  = input;
      _value = value;
    }

  BaseInput *getInput() const { return this->_ctrl; };

  protected:

    bool triggered() const{
      return _ctrl->value(CTRL);
    }

    void getData(CtrlQueueData &data){
      data.flag  = FLAG;
      data.value = _value;
      data.min   = 0;
      data.max   = 0;
    }

  protected:
    int8_t  _value;    
    PushButton *_ctrl;
};

/*
template <const uint16_t CMD,
         const uint16_t NOISE_THRESHOLD = POT_NOISE_THRESHOLD, 
         const uint16_t LOWER_MARGIN = POT_LOWER_MARGIN,
         const uint16_t UPPER_MARGIN = POT_UPPER_MARGIN >
class CtrlItemPtmtr1: public CtrlItemSC<CMD, AnalogInput>{
  public:
    CtrlItemPtmtr1(AnalogInput *ptn){
      this->_ctrl = ptn;
     _value  = POT_MAX; //just to make sure it is different from what we read
    }

  protected:
    bool triggered() const{
      int16_t value = (int16_t)getValue(); 
      return (abs(value - (int16_t)_value) >  min(NOISE_THRESHOLD, 
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
    }

    uint16_t getValue() const{
      uint16_t value = ( _value + this->_ctrl->value() ) / 2;

      return value < POT_MIN + LOWER_MARGIN ? POT_MIN + LOWER_MARGIN : value > POT_MAX - UPPER_MARGIN ? POT_MAX - UPPER_MARGIN : value;
    }

  protected:
   uint16_t  _value;
};


*/


void setup() {

  DBG_INIT();

  DBG_OUTLN("Started");

/*
  static SerialInput serial;
  static CtrlItemSerial<TestParse> ctrlSr(&serial);
  

  static CtrlPanel panel;
  panel.addControl(&ctrlSr);

  static NtfSet set;
  set.addNtf(&ctrlSr);

  static PushButton btn(10);  
  static CtrlItemPb<PB_CONTROL_CLICK_LONG> ctrlPb(33, &btn);
  panel.addControl(&ctrlPb);

  AnalogInput ai(5);
  CtrlItemPtmtr<> ptmr(10, &ai);



  static CtrlQueueItem itm;

  for(;;){
    
    panel.loop(itm);    

    if(itm.cmd != EEMC_NONE){      
            
      set.put_F(F("response"), itm.cmd);
    }  
  }
*/

  static PushButton pb(10);

  static CtrlItem1Pb<10, PB_CONTROL_CLICK_SHORT> ctl(&pb);
  static CtrlItem1Pb<20, PB_CONTROL_CLICK_LONG> ctl1(&pb);

  //static CtrlItemPb<PB_CONTROL_CLICK_SHORT> ctl(10, &pb);
  //static CtrlItemPb<PB_CONTROL_CLICK_LONG> ctl1(20, &pb);


  static CtrlQueueItem itm;

  while(1){
    pb.read();
    
    ctl.loop(itm);
    ctl1.loop(itm);
  }

  
  




  //static AnalogInput ai(2);
  //static CtrlItemPtmtr<10, 7, 1000> ptr(20, &ai);
  //static CtrlItemPtmtr<21, 66, 1000> ptr1(21, &ai);
  //static CtrlItemPtmtr1<22, 10, 7, 1000> ptr(&ai);
  //static CtrlItemPtmtr1<23, 11, 7, 1000> ptr1(&ai);

}



void loop(){
}
