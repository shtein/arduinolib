#include "arduinolib.h"

#include <Arduino.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include "EEPROMCfg.h"
#include "DbgTool.h"


/////////////////////////////////
// EEPROMConf
EEPROMConf::EEPROMConf(){  
  _index = 0;
}

bool EEPROMConf::read(void *p, size_t size){  
  uint8_t *cur = (uint8_t *)p;

  //Read byte by byte
  for(size_t i = 0; i < size; i++, cur++, _index ++){
    *cur = eeprom_read_byte( (uint8_t*) _index );
    
  }

  return true;
}

bool EEPROMConf::write(const void *p, size_t size){
  const uint8_t *cur = (uint8_t *)p;

  //Write byte by byte if different from what is in EEProm cell
  for(size_t i = 0; i < size; i++, cur++, _index ++){
    if(eeprom_read_byte( (uint8_t*) _index ) != *cur){
      eeprom_write_byte( (uint8_t*) _index, *cur );
    }
  }


  return true;
}
