#ifndef __CTRL_SERIAL_H
#define __CTRL_SERIAL_H

#include "Utils.h"
#include "AnalogInput.h"
#include "Controls.h"
#include "Notification.h"

//////////////////////////////
//Input from serial port
#define SI_CR          '\r'
#define SI_LF          '\n'
#define SI_BS          '\b'
#define SI_NULLCHAR    '\0'
#define SI_SPACE       ' '

#if defined(ESP8266) || defined(ESP32)
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
    char *getCommandLine();
    void reset();

  protected:   
    char    _bufRead[SI_BUFF_LEN + 1]; 
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
  void end(const char *key = NULL);
  void beginArray(const char *key = NULL);
  void endArray(const char *key = NULL);

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
template <uint8_t (*PARSER) (char *cmdLine, CtrlQueueData &data)>
using CtrlItemSerial = CtrlItemMultiCommand<SerialInput, NtfSerial, PARSER>;

#endif //__CTRL_SERIAL_H