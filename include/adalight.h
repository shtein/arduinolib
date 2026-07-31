#ifndef __ADALIGHT_H
#define __ADALIGHT_H

#include "arduinolib.h"


//////////////////////////////////////
// AdaLight - base class for Adalight protocol



class AdaLight{
public:
  AdaLight();

  void init(bool ledsOn = true, uint32_t serialBaudrate = 115200);
  void loop();
  void setLedsOn(bool ledsOn);

protected:  
  void onHeader(uint8_t c);
  void onData(uint8_t c);
  void onTimeout();

protected:
  uint8_t _mode;

  //Timers
  unsigned long _lastByteTime;
  uint8_t       _timeoutCounter;

  //Data countres
  uint8_t _dataIndex;

  //Checksum
  uint8_t _low;
  uint8_t _high;

  //Leds
  uint32_t _ledsRemaining;
  uint32_t _ledIndex;

  //Turning on/off leds
  bool _ledsOn;
};


#endif // __ADALIGHT_H