#ifndef __EEPROMCFG_H
#define __EEPROMCFG_H

////////////////////////////////
// EEPROMConf

class EEPROMConf{
public:
  EEPROMConf();

  bool read(void *p, size_t size);        //read block
  bool write(const void *p, size_t size); //write block

  template <typename T>
  bool read(T &t){ return read(&t, sizeof(T)); }

  template <typename T>
  bool write(const T &t){ return write(&t, sizeof(T));}

  template<typename T>
  EEPROMConf & operator >> (T &t){ read(t); return *this; }

  template<typename T>
  EEPROMConf & operator << (const T &t){ write(t); return *this; }

private:
  size_t _index;  //EEPROM cell index
};




#endif //__EEPROMCFG_H