#include "arduinolib.h"

#include <Arduino.h>


#if defined(ESP8266) || defined(ESP32)
#include <flash_hal.h>
#else
#include <avr/eeprom.h>
#include <avr/io.h>
#endif 

#include "EEPROMCfg.h"
#include "DbgTool.h"


#if defined(ESP8266) || defined(ESP32)
uint8_t eeprom_read_byte (const uint8_t *__p){
  return 0; 
}

void eeprom_write_byte (uint8_t *__p, uint8_t __value){

}

#endif


/////////////////////////////////
// EEPROMConf
EEPROMCfg::EEPROMCfg(size_t index){  
  _index = index;
}

bool EEPROMCfg::read(void *p, size_t size){  
  uint8_t *cur = (uint8_t *)p;

  //Read byte by byte
  for(size_t i = 0; i < size; i++, cur++, _index ++){
    *cur = eeprom_read_byte( (uint8_t*) _index );
  }

  
  return true;
}

bool EEPROMCfg::write(const void *p, size_t size){
  const uint8_t *cur = (uint8_t *)p;

  //Write byte by byte if different from what is in EEProm cell
  for(size_t i = 0; i < size; i++, cur++, _index ++){
    if(eeprom_read_byte( (uint8_t*) _index ) != *cur){
      eeprom_write_byte( (uint8_t*) _index, *cur );
    }
  }


  return true;
}
