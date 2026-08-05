#ifndef __AMBIENTLIGHT_H
#define __AMBIENTLIGHT_H

#include "arduinolib.h"


///////////////////////////////////////
//AmbientLight LED Strip

//Debug LED
#ifdef AMBL_DEBUG_LED
  #define AMBL_DEBUG_LED_INIT() \
      pinMode(LED_BUILTIN, OUTPUT); \
      digitalWrite(LED_BUILTIN, LOW);

  #define AMBL_DEBUG_LED_ON()  \
    digitalWrite(LED_BUILTIN, HIGH);

  #define AMBL_DEBUG_LED_OFF()  \
    digitalWrite(LED_BUILTIN, LOW);

#else
  #define AMBL_DEBUG_LED_INIT()
  #define AMBL_DEBUG_LED_ON()
  #define AMBL_DEBUG_LED_OFF()
#endif

#if defined(AMBL_FASTLED) || defined(AMBL_NEOPIXEL_BUS)
    
  //LEDs count`
  #ifndef AMBL_LEDS_COUNT
    #error "AMBL_LEDS_COUNT is not defined."
  #endif

  //PIN
  #ifndef AMBL_LEDS_PIN
    #error "AMBL_LEDS_PIN is not defined."
  #endif

  //LED TYPE
  #ifndef AMBL_LEDS_TYPE
    #error "AMBL_LEDS_TYPE is not defined."
  #endif

#endif // defined(AMBL_FASTLED) || defined(AMBL_NEOPIXEL_BUS)

//Library selection
#if defined(AMBL_FASTLED)
  #include <FastLED.h>  

  #ifndef AMBL_COLOR_ORDER
    #define AMBL_COLOR_ORDER GRB
  #endif


#elif defined(AMBL_NEOPIXEL_BUS)
  #include <NeoPixelBus.h>
  
  #ifndef AMBL_COLOR_ORDER
    #define AMBL_COLOR_ORDER NeoGrbFeature
  #endif

#else
  #pragma message "LED library is not defined."

  #ifdef AMBL_LEDS_COUNT
    #undef AMBL_LEDS_COUNT
  #endif
  #define AMBL_LEDS_COUNT 0
  
#endif


///////////////////////////////////////
// AmbientLightRececiiver - base class for Ambient light data receiver

#define AMBL_DATA_NONE  0x0000
#define AMBL_DATA_COLOR 0x0100
#define AMBL_DATA_SKIP  0x0200
#define AMBL_DATA_END   0x0400

class AmbientLightReceiver{
public:
  // Reset internal state (e.g. init or after timeout)
  virtual void reset() = 0;                                  
  // Called when no data has been received for a while
  virtual void onTimeout() = 0;                              
  // Get next data byte. Low byte is color 0-255, high byte is a bitmask of AMBL_DATA_* flags.
  virtual uint16_t getData() = 0; 
};


///////////////////////////////////////
// AmbientLight 
class AmbientLight{
public:
  AmbientLight();

  void loop();
  void init(AmbientLightReceiver *receiver, bool ledsOn);
  void setLedsOn(bool ledsOn);  
  void setReceiver(AmbientLightReceiver *receiver);

private:
   bool processData();     //Process data and return true if data was received

   //Led functions
   void showLeds();
   void clearLeds();
   void setLedColor(uint16_t ledIndex, uint8_t r, uint8_t g, uint8_t b);

private:
  bool                  _ledsOn;          //Turning on/off leds
  uint32_t              _lastDataTime;    //Last time data was received
  uint8_t               _timeoutCounter;  //Timeout counter

  AmbientLightReceiver *_receiver;        //Data receiver
  uint16_t              _indexColor;      //Index of the current color
  uint8_t               _indexChannel;    //Index of the current channel (0-2)
  uint8_t               _rgb[3];          //Current color data

#ifdef AMBL_FASTLED
  CRGB _leds[AMBL_LEDS_COUNT];             //Led strip object
#elif defined(AMBL_NEOPIXEL_BUS)
  NeoPixelBus<AMBL_COLOR_ORDER, AMBL_LEDS_TYPE> _leds; //Led strip object
#endif
};


///////////////////////////////////////////
// AdaLightReceiver - AdaLight protocol receiver
class AdaLightReceiver : public AmbientLightReceiver{
public:
  AdaLightReceiver();

  void init(uint32_t serialBaudrate = 115200);

  virtual void reset() override;
  virtual void onTimeout() override;
  virtual uint16_t getData() override;

protected:
  void processHeader(uint8_t c);
  
protected:
  //Mode
  uint8_t _mode;

  //Checksum
  uint8_t _low;
  uint8_t _high;

  //Leds
  uint32_t _remaining;
};

#if (defined(ESP8266) || defined(ESP32) ) 

#ifdef ESP8266
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

#include <WiFiUdp.h>

#define DDP_BUFFER_SIZE 256

/////////////////////////////////////////
// DPPReceiver - DDP protocol receiver
class DDPLightReceiver : public AmbientLightReceiver{
public:
  DDPLightReceiver();

  void init(uint16_t port = 4048);

  virtual void reset() override;
  virtual void onTimeout() override;
  virtual uint16_t getData() override;

private:
  void processPacket();
  

private:
  WiFiUDP _udp;                    //UDP socket
  bool    _processing;             //True if a packet is being processed

  uint8_t  _data[DDP_BUFFER_SIZE]; //Receive buffer
  uint16_t _dataIndex;             //Index of the current byte in the buffer
  uint16_t _dataLength;            //Length of the current buffer
  uint16_t _frameLength;           //Length of the current frame 
  uint32_t _offset;                //Offset of the current frame in the stream
  bool     _stripEnd;              //End of strip marker received
};

#endif // ESP8266 || ESP32

#endif // __AMBIENTLIGHT_H