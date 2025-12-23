#ifndef __CTRL_SERIAL_H
#define __CTRL_SERIAL_H

#include "arduinolib.h"
#include "alutils.h"
#include "AnalogInput.h"
#include "Controls.h"
#include "Notification.h"



//////////////////////////////
//Input from serial port - reading from terminal
#define SI_CR          '\r'
#define SI_LF          '\n'
#define SI_BS          '\b'
#define SI_NULLCHAR    '\0'
#define SI_SPACE       ' '

#ifdef WIFI_ENABLED
  #define SI_BUFF_LEN     128
#else
  #define SI_BUFF_LEN     32
#endif


class SerialInput: public BaseInput{
  public:
    SerialInput();
    ~SerialInput();

    void read();
    bool isReady() const;
    bool getTokens(const char *tokens[], size_t maxTokens);
    void reset();

  protected:   
    char    _bufRead[SI_BUFF_LEN + 2]; 
    uint8_t _lenRead; 
};

////////////////////////////////
// NtfSerial - notificatoins via serial port
class NtfSerial: public NtfBase {
public:    
  NtfSerial();

  void reset();
  void send();

  void begin(const char *key = NULL);
  void end();
  void beginArray(const char *key = NULL);
  void endArray();

  void put(const char *key, bool v);
  void put(const char *key, uint8_t v);
  void put(const char *key, uint16_t v);
  void put(const char *key, uint32_t v);
  void put(const char *key, int8_t v);
  void put(const char *key, int16_t v);  
  void put(const char *key, int32_t v);
  void put(const char *key, const char *v);

private:
  void print(const __FlashStringHelper *fmt, ...); 

private:
  uint8_t _ident;    
};


/////////////////////////////////////////
// Control from serial buffer
// Multi-command interface
template <uint8_t (*PARSER) (const char *tokens[], CtrlQueueData &data)>
using CtrlItemSerial = CtrlItemMultiCommand<SerialInput, NtfSerial, PARSER>;

////////////////////////////////////////
// Input from serial port - binary protocol

class SerialInputBinary: public BaseInput{
  public:
    SerialInputBinary();  

    void read();
    bool isReady() const { return _lenRead != 0; }
    void reset();

    const uint8_t * getData() const{ return _bufRead; }
    uint8_t   getDataLen() const { return _lenRead; }

  protected:
    protected:   
    uint8_t  _bufRead[SI_BUFF_LEN]; 
    uint8_t _lenRead;
};


///////////////////////////////////////////////
// CtrlItemSerialBinary - control item for binary serial input
class CtrlItemSerialBinary: public CtrlItem{
  public:
    CtrlItemSerialBinary(SerialInputBinary *input):
      CtrlItem(EEMC_NONE, input){
    }

  protected:
    bool triggered() const{
      return ((SerialInputBinary *)_input)->isReady();
    }

    void getData(CtrlQueueData &data){
      SerialInputBinary *in = (SerialInputBinary *)_input;

      if(in->getDataLen() != sizeof(CtrlQueueItem)){
        _cmd = EEMC_ERROR;
        return;
      }

      const CtrlQueueItem * ctrl = (const CtrlQueueItem *)in->getData();
      _cmd = ctrl->cmd;
      data = ctrl->data;
      
    }
};

#endif //__CTRL_SERIAL_H