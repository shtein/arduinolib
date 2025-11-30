#ifndef __EEPROMCFG_H
#define __EEPROMCFG_H

#include "arduinolib.h"

//A workaround for including EEPROM Library 
#if defined(ESP8266) || defined(ESP32)
  #include <EEPROM.h>
#endif 

////////////////////////////////
// EEPROMConf

class EEPROMCfg{
public:
  EEPROMCfg(size_t index = 0);
  ~EEPROMCfg();

  bool read(void *p, size_t size);        //read block
  bool write(const void *p, size_t size); //write block

  template <typename T>
  bool read(T &t){ return read(&t, sizeof(T)); }

  template <typename T>
  bool write(const T &t){ return write(&t, sizeof(T));}

  template<typename T>
  EEPROMCfg & operator >> (T &t){ read(t); return *this; }

  template<typename T>
  EEPROMCfg & operator << (const T &t){ write(t); return *this; }

  void moveTo(size_t index){ _index = index; }

private:
  size_t _index;  //EEPROM cell index
};




#endif //__EEPROMCFG_H