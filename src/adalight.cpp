#include "adalight.h"

//Mode
#define ADA_MODE_HEADER 0
#define ADA_MODE_DATA   1


//Debug LED
#ifdef ADA_DEBUG_LED
  #define ADA_DEBUG_LED_INIT() \
      pinMode(LED_BUILTIN, OUTPUT); \
      digitalWrite(LED_BUILTIN, LOW);

  #define ADA_DEBUG_LED_ON()  \
    digitalWrite(LED_BUILTIN, HIGH);

  #define ADA_DEBUG_LED_OFF()  \
    digitalWrite(LED_BUILTIN, LOW);

#else
  #define ADA_DEBUG_LED_INIT()
  #define ADA_DEBUG_LED_ON()
  #define ADA_DEBUG_LED_OFF()
#endif



#if defined(ADA_FASTLED) || defined(ADA_NEOPIXEL_BUS)
    
  //LEDs count`
  #ifndef ADA_LEDS_COUNT
    #error "ADA_LEDS_COUNT is not defined."
  #endif

  //PIN
  #ifndef ADA_LEDS_PIN
    #error "ADA_LEDS_PIN is not defined."
  #endif

  //LED TYPE
  #ifndef ADA_LEDS_TYPE
    #error "ADA_LEDS_TYPE is not defined."
  #endif

#endif // defined(ADA_FASTLED) || defined(ADA_NEOPIXEL_BUS)

//Library selection
#if defined(ADA_FASTLED)
  #include <FastLED.h>

CRGB leds[ADA_LEDS_COUNT];

  #ifndef ADA_COLOR_ORDER
    #define ADA_COLOR_ORDER GRB
  #endif

  #define ADA_LEDS_INIT()  \
    FastLED.addLeds<ADA_LEDS_TYPE, ADA_LEDS_PIN, ADA_COLOR_ORDER>(leds, ADA_LEDS_COUNT);

  #define ADA_LEDS_SHOW()  \
    FastLED.show();

  #define ADA_LEDS_CLEAR()  \
    fill_solid(leds, ADA_LEDS_COUNT, CRGB::Black);

  #define ADA_LEDS_OFF()  \
    ADA_LEDS_CLEAR(); \
    ADA_LEDS_SHOW();
  
  #define ADA_LEDS_SET_COLOR_BY_INDEX(ledIndex, colorIndex, x)  \
      if(ledIndex < ADA_LEDS_COUNT) \
        leds[ledIndex].raw[colorIndex] = x;


#elif defined(ADA_NEOPIXEL_BUS)
  #include <NeoPixelBus.h>
  
  #ifndef ADA_COLOR_ORDER
    #define ADA_COLOR_ORDER NeoRgbFeature
  #endif

  NeoPixelBus<ADA_COLOR_ORDER, ADA_LEDS_TYPE> leds(ADA_LEDS_COUNT, ADA_LEDS_PIN);

  #define ADA_LEDS_INIT() \
    leds.Begin();

  #define ADA_LEDS_SHOW()  \
    leds.Show();

  #define ADA_LEDS_CLEAR()  \
    leds.ClearTo(RgbColor(0, 0, 0));

  #define ADA_LEDS_OFF()  \
    ADA_LEDS_CLEAR(); \
    ADA_LEDS_SHOW();
  
  #define ADA_LEDS_SET_COLOR_BY_INDEX(ledIndex, colorIndex, x)  \
      if(ledIndex < ADA_LEDS_COUNT) {\
        RgbColor clr = leds.GetPixelColor(ledIndex); \
        if(colorIndex == 0) clr.R = (x); \
        else if(colorIndex == 1) clr.G = (x); \
        else if(colorIndex == 2) clr.B = (x); \
        leds.SetPixelColor(ledIndex, clr); \
      } 
      

#else
  #pragma message "LED library is not defined."
  #define ADA_LEDS_INIT()
  #define ADA_LEDS_SHOW()
  #define ADA_LEDS_OFF()
  #define ADA_LEDS_SET_COLOR_BY_INDEX(ledIndex, colorIndex, x)
  #define ADA_LEDS_CLEAR()

  #ifdef ADA_LEDS_COUNT
    #undef ADA_LEDS_COUNT
  #endif
  #define ADA_LEDS_COUNT 0
#endif


///////////////////////////////////
// Adalight 
AdaLight::AdaLight(){
  _ledsOn         = false; 
  _timeoutCounter = 0;
  _lastByteTime   = 0;
  _mode           = ADA_MODE_HEADER;
  _dataIndex      = 0;
  _ledsRemaining  = 0;
  _ledIndex       = 0;
}

void AdaLight::setLedsOn(bool ledsOn){
  _ledsOn = ledsOn;

  if(!_ledsOn){
    //Turn off leds
    ADA_LEDS_OFF();
  }
}

void AdaLight::init(bool ledsOn, uint32_t serialBaudrate){  
  //Set leds on/off
  _ledsOn = ledsOn;

  ADA_DEBUG_LED_INIT();

  //Init leds
  ADA_LEDS_INIT();
  ADA_LEDS_OFF();

  //Init serial port
  if(serialBaudrate > 0)
    Serial.begin(serialBaudrate);

  //Send acknowledge
  Serial.print("Ada\n");

  //Initial time
  _lastByteTime   = millis();
  _timeoutCounter = 0;
}


void AdaLight::loop(){

  bool receivedData = false;

  while (Serial.available() > 0){    
    //Reset timers
    _lastByteTime = millis();
    _timeoutCounter = 0;

    //Read byte
    int c = Serial.read();
    receivedData = true;

    //Process
    if(_mode == ADA_MODE_HEADER){
      onHeader(c);
    }
    else {
      onData(c);
    }
  }

  if(!receivedData){
    //No data - check timeout
    onTimeout();
  }
}


const uint8_t magic[] = {'A','d','a'};
#define MAGICSIZE  sizeof(magic)
#define HICHECK    (MAGICSIZE)
#define LOCHECK    (MAGICSIZE + 1)
#define CHECKSUM   (MAGICSIZE + 2)

void AdaLight::onHeader(uint8_t c){
  if(_dataIndex < MAGICSIZE){
    // Check if magic word matches
    if(c == magic[_dataIndex]) {_dataIndex++;}
    else {_dataIndex = 0;}
  }
  else{

    switch(_dataIndex){
      case HICHECK:
        _high = c;
        _dataIndex++;
      break;
        
      case LOCHECK:
        _low = c;
        _dataIndex++;
      break;

      case CHECKSUM:
        //Check checksum
        if((_high ^ _low ^ 0x55) == c){                  
          //Remainign leds
          _ledIndex      = 0;
          _ledsRemaining = 256UL * (uint32_t)_high + (uint32_t)_low + 1UL;
          _mode          = ADA_MODE_DATA;    

          //Clear leds
          ADA_LEDS_CLEAR();

          //Debug led
          ADA_DEBUG_LED_ON();
        }
        _dataIndex = 0;      
      break;
    }
  }
}


void AdaLight::onData(uint8_t c){
  //Save led channel value - only if leds are on
  if(_ledsOn){
    ADA_LEDS_SET_COLOR_BY_INDEX(_ledIndex, _dataIndex, c);
  }

  //Increment channel
  _dataIndex++;

  if(_dataIndex >= 3){
    _dataIndex = 0;
    
    //Increment led index
    if(_ledIndex < ADA_LEDS_COUNT){
      _ledIndex++;
    }    

    //Decrement remaining leds
    _ledsRemaining--;
  }

  if(_ledsRemaining == 0){
    
    //Turn on leds
    if(_ledsOn){
      ADA_LEDS_SHOW();
    }

    //Debug led    
    ADA_DEBUG_LED_OFF();

    //Reset data index
    _dataIndex = 0;

    //Back to header mode
    _mode = ADA_MODE_HEADER;
  }
}

#define TIMEOUT_SERIAL        1000
#define TIMEOUT_RESET_COUNTER 10

void AdaLight::onTimeout(){
  //Get current time
  unsigned long now = millis();

  if((uint32_t)(now - _lastByteTime) >= TIMEOUT_SERIAL){
    //If no data received for a while, send ping-acknowledge
    Serial.print("Ada\n");

    //Reset timer
    _lastByteTime = now;

    //Increment timeout counter
    if (_timeoutCounter < TIMEOUT_RESET_COUNTER) {
      _timeoutCounter++;
    }

    //If no data for longer time turn off leds, turn on header mode - do it once
    if(_timeoutCounter == TIMEOUT_RESET_COUNTER){
      _mode          = ADA_MODE_HEADER;
      _dataIndex     = 0;
      _ledsRemaining = 0;
      _ledIndex      = 0;

      //Turn off leds
      ADA_LEDS_OFF();
      ADA_DEBUG_LED_OFF();
    }
  }
}