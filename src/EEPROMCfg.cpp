#include "arduinolib.h"

#include <Arduino.h>

#ifdef __AVR__
#include <avr/io.h>
#endif 

#include "EEPROMCfg.h"
#include "DbgTool.h"

//EEPROM init/flushread/write

#if defined (__AVR__)
//EEPROM routines for ATMEGAXXX
  #define EEPROM_INIT()
  #define EEPROM_FLUSH()
  #define EEPROM_READ_BYTE(addr, byte) byte = eeprom_read_byte( (uint8_t*) _index );
  #define EEPROM_WRITE_BYTE(addr, byte) \
    if(eeprom_read_byte( (uint8_t*) addr ) != byte){ \
      eeprom_write_byte( (uint8_t*) addr, byte); \
    }    

#elif defined(ESP8266) ||  defined(ESP32) 
//EEPROM routines ESP8266
  #define EEPROM_INIT() EEPROM.begin(512);
  #define EEPROM_FLUSH() EEPROM.commit();  
  #define EEPROM_READ_BYTE(addr, byte) byte = EEPROM.read(addr);
  #define EEPROM_WRITE_BYTE(addr, byte) EEPROM.write(addr, byte);
#endif


/////////////////////////////////
// EEPROMConf
EEPROMCfg::EEPROMCfg(size_t index){  
  EEPROM_INIT();

  _index = index;  
}

EEPROMCfg::~EEPROMCfg(){  
  EEPROM_FLUSH();
}

bool EEPROMCfg::read(void *p, size_t size){  
  uint8_t *cur = (uint8_t *)p;

  //Read byte by byte
  for(size_t i = 0; i < size; i++, cur++, _index ++){
    EEPROM_READ_BYTE(_index, *cur);
  }

  
  return true;
}

bool EEPROMCfg::write(const void *p, size_t size){
  const uint8_t *cur = (uint8_t *)p;

  //Write byte by byte if different from what is in EEProm cell
  for(size_t i = 0; i < size; i++, cur++, _index ++){
    EEPROM_WRITE_BYTE(_index, *cur);    
  }

  return true;
}
