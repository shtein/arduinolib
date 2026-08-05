#include "ambientlight.h"
#include "DbgTool.h"



////////////////////////////////////////
// AmbientLight - base class for ambient light protocols
AmbientLight::AmbientLight()
#ifdef AMBL_NEOPIXEL_BUS
  : _leds(AMBL_LEDS_COUNT, AMBL_LEDS_PIN)
#endif
{
  _ledsOn         = true; 
  _timeoutCounter = 0;
  _lastDataTime   = 0;
  _receiver       = NULL;
  _indexColor     = 0;
  _indexChannel   = 0;
}

void AmbientLight::init(AmbientLightReceiver *receiver, bool ledsOn){
  //Init leds
#ifdef AMBL_FASTLED
  FastLED.addLeds<AMBL_LEDS_TYPE, AMBL_LEDS_PIN, AMBL_COLOR_ORDER>(_leds, AMBL_LEDS_COUNT);
#elif defined(AMBL_NEOPIXEL_BUS)
  _leds.Begin();
#endif

  //Turn off leds
  clearLeds();
  showLeds();

  _ledsOn         = ledsOn;
  _receiver       = receiver;
  _lastDataTime   = millis();

  //Debug led
  AMBL_DEBUG_LED_INIT();
}

void AmbientLight::setReceiver(AmbientLightReceiver *receiver){
  _receiver = receiver;
  clearLeds();
  showLeds();
}

void AmbientLight::setLedsOn(bool ledsOn){
  _ledsOn = ledsOn;

  if(!_ledsOn){
    //Turn off leds
    clearLeds();
    showLeds();
  }
}

void AmbientLight::showLeds(){
#ifdef AMBL_FASTLED
  FastLED.show();
#elif defined(AMBL_NEOPIXEL_BUS)
  _leds.Show();
#endif
}
  

void AmbientLight::clearLeds(){
#ifdef AMBL_FASTLED
  fill_solid(_leds, AMBL_LEDS_COUNT, CRGB::Black);
#elif defined(AMBL_NEOPIXEL_BUS)
  _leds.ClearTo(RgbColor(0, 0, 0));
#endif
}

void AmbientLight::setLedColor(uint16_t ledIndex, uint8_t r, uint8_t g, uint8_t b){
  if(ledIndex < AMBL_LEDS_COUNT){
#ifdef AMBL_FASTLED  
    _leds[ledIndex] = CRGB(r, g, b);
#elif defined(AMBL_NEOPIXEL_BUS)
    _leds.SetPixelColor(ledIndex, RgbColor(r, g, b));
#endif
  }
}

bool AmbientLight::processData(){
  //Check if receiver is set
  if(!_receiver){    
    return false;
  }

  //Retrieve data from receiver
  uint16_t result    = AMBL_DATA_NONE;
  bool dataReceived = false;
  
  while((result = _receiver->getData())){
    //Data received
    dataReceived = true;

    //Skip data
    if(result & AMBL_DATA_SKIP){      
      continue;
    }

    //Process color data
    if(result & AMBL_DATA_COLOR){
      _rgb[_indexChannel] = (uint8_t)(result & 0x00FF);    
      _indexChannel++;
    }
    
    //Set color if all channels received
    if(_indexChannel == 3){      
      //Set color
      setLedColor(_indexColor, _rgb[0], _rgb[1], _rgb[2]);    

      //Reset channel index
      _indexChannel = 0;
      //Increment color index
      _indexColor++;            
    }  
      
    //Show leds if end of strip data
    if(result & AMBL_DATA_END){      
      if(_ledsOn){
        showLeds();        
        //DBG_OUTLN("Show leds %d", _indexColor);
      }
                    
      //Reset 
      _indexColor   = 0;
      _indexChannel = 0;
      _receiver->reset();
    }    
  }        

  return dataReceived;
}

#define TIMEOUT_DATA          1000
#define TIMEOUT_COUNT_MAX     10


void AmbientLight::loop(){

  //Process data
  if(processData()){
     //Reset timeout counter
    _lastDataTime = millis();
    _timeoutCounter = 0;

  } else {
    //Check timeout
    uint32_t now = millis();
    if((now - _lastDataTime) > TIMEOUT_DATA){
      _lastDataTime = now;
      //Call timeout handler      
      if(_receiver){
        _receiver->onTimeout();
      }

      //Increment timeout counter
      if(_timeoutCounter < TIMEOUT_COUNT_MAX){
        _timeoutCounter++;

        //Check if timeout counter reached max
        if(_timeoutCounter == TIMEOUT_COUNT_MAX){            
          //Turn off leds
          clearLeds();
          showLeds();          

          //Reset internal state
          _indexColor   = 0;
          _indexChannel = 0;
          
          if(_receiver){
            _receiver->reset();
          }
        }      
      }
    }
  }
}


///////////////////////////////////////////////
// AdaLightReceiver - AdaLight protocol receiver

const uint8_t magic[] = {'A','d','a'};
#define MAGICSIZE           sizeof(magic)
#define ADA_MODE_HEADER     0x00
#define ADA_MODE_HIGH_CHECK (MAGICSIZE)
#define ADA_MODE_LOW_CHECK  (MAGICSIZE + 1)
#define ADA_MODE_CHECKSUM   (MAGICSIZE + 2)
#define ADA_MODE_DATA       (ADA_MODE_CHECKSUM + 1)


AdaLightReceiver::AdaLightReceiver(){
  _mode       = ADA_MODE_HEADER;  
  _remaining  = 0;
  _low        = 0;
  _high       = 0;
}


void AdaLightReceiver::init(uint32_t serialBaudrate){
  //Init serial
  if(serialBaudrate > 0)
    Serial.begin(serialBaudrate);

  //Send acknowledge
  Serial.print("Ada\n");
      
  //Reset internal state
  reset();
}


void AdaLightReceiver::reset(){
  _mode       = ADA_MODE_HEADER;  
  _remaining  = 0;
  _low        = 0;
  _high       = 0;

  AMBL_DEBUG_LED_OFF();
}

void AdaLightReceiver::onTimeout(){
  //Send acknowledge
  Serial.print("Ada\n");
}



void AdaLightReceiver::processHeader(uint8_t c){

  switch(_mode){
    case ADA_MODE_HIGH_CHECK:
      _high = c;
      _mode ++;
    break;
      
    case ADA_MODE_LOW_CHECK:
      _low = c;
      _mode ++;
    break;

    case ADA_MODE_CHECKSUM:
      //Check checksum
      if((_high ^ _low ^ 0x55) == c){                  
        //Remainign leds          
        _remaining = (256UL * (uint32_t)_high + (uint32_t)_low + 1UL) * 3UL; //3 channels per led
        _mode      = ADA_MODE_DATA; 
                
       AMBL_DEBUG_LED_ON();
      }
      else{ //Something wrong - reset
        _mode = ADA_MODE_HEADER;
      }
    break;

    default:
      if(c == magic[_mode]) {_mode ++;}
      else {_mode = ADA_MODE_HEADER;}
    break;
  }
  
}



uint16_t AdaLightReceiver::getData(){
  //Read byte
  int c = Serial.read();
  

  //Check if data is available
  if(c < 0){    
    return AMBL_DATA_NONE;
  }

  if(_mode == ADA_MODE_DATA){

    //Decrement remaining leds
    _remaining --;    

    //Prepare result. Low byte is color 0-255, high byte is a bitmask of AMBL_DATA_* flags.
    uint16_t result = (uint16_t)(c) | AMBL_DATA_COLOR;    

    //Check end of strip data
    if(_remaining == 0){
      result |= AMBL_DATA_END;
      _mode = ADA_MODE_HEADER;
      AMBL_DEBUG_LED_OFF();
    }

    return result;
  }

  //Otehrwise process header
  processHeader(c);

  return AMBL_DATA_SKIP;
}


#if (defined(ESP8266) || defined(ESP32) ) 
/////////////////////////////////////////
// DDPLightReceiver - DDP protocol receiver

DDPLightReceiver::DDPLightReceiver(){
  _processing = false;
  _dataIndex  = 0;
  _dataLength = 0;
  _frameLength = 0;
  _stripEnd   = false;
}

void DDPLightReceiver::init(uint16_t port){
  //Init UDP
  _udp.begin(port);

  //Reset internal state
  reset();
}

void DDPLightReceiver::reset(){
  _processing = false;
  _dataIndex  = 0;
  _dataLength = 0;
  _frameLength = 0;
  _stripEnd   = false;
}

void DDPLightReceiver::onTimeout(){
}


//////////////////////////////////////////////////////////////////////////////
// DDP protocol
//
// 10-byte header (14 if the TIMECODE flag is set):
//   byte 0   : flags   bit0 PUSH, bit4 TIMECODE, bits6-7 version
//   byte 1   : sequence number
//   byte 2   : data type
//   byte 3   : destination / output id
//   byte 4-7 : data offset in bytes (big-endian)
//   byte 8-9 : data length in bytes (big-endian)
//   [10-13]  : timecode (only present if TIMECODE flag set)
//   payload  : RGB triplets
//////////////////////////////////////////////////////////////////////////////


#define DDP_FLAG_PUSH      0x01
#define DDP_FLAG_TIMECODE  0x10

#define DDP_HEADER_LEN     10
#define DDP_HEADER_LEN_TC  14

uint16_t DDPLightReceiver::getData(){

  //If currently processeing a packet return next byte from the payload
  if(_processing){

    //No more data available - reset processing state
    if(_dataIndex == _dataLength){
            
      //Try to read next portion of data from the UDP socket
      _dataIndex = 0;
      int bytesRead = _udp.read(_data, sizeof(_data));
      _dataLength = (bytesRead > 0) ? bytesRead : 0;

      if(_dataLength == 0){
        _processing = false;

        //If previsous packet had the end of strip marker then return end of strip
        if(_stripEnd){
          _stripEnd = false;
          return AMBL_DATA_END;
        }
        else{
          return AMBL_DATA_SKIP;
        }
      }      
    }
    
    //Return next byte from payload
    uint16_t result = _data[_dataIndex] | AMBL_DATA_COLOR;
    //Increment data index
    _dataIndex++;

    return result;
  }

  //Check if packet is available
  int packetSize = _udp.parsePacket();

  //do nothing if no packet available
  if(packetSize <= 0){
    return AMBL_DATA_NONE;
  }
    
  //Read header
  uint8_t header[DDP_HEADER_LEN_TC];

  //Read header
  int len = _udp.read(header, DDP_HEADER_LEN);
  if(len < DDP_HEADER_LEN){
    //Not enough data - skip packet
    return AMBL_DATA_SKIP;
  }

  //Parse packet 
  uint8_t flags = header[0];

  //Advance to the payload. The header is longer when a timecode field is present
  if(flags & DDP_FLAG_TIMECODE){
    //Read timecode
    len = _udp.read(header + DDP_HEADER_LEN, DDP_HEADER_LEN_TC - DDP_HEADER_LEN);
  }

  //Remenber if the end of strip marker
  _stripEnd   = (flags & DDP_FLAG_PUSH) ? true : false;

  //Data lenght of strip in bytes
  _frameLength = (uint16_t)header[8] << 8 | (uint16_t)header[9];

  uint8_t seqNum = header[1];

  //Offest of the current frame in the stream
  _offset = (uint32_t)header[4] << 24 | (uint32_t)header[5] << 16 | (uint32_t)header[6] << 8 | (uint32_t)header[7];

  //DBG_OUTLN("DDP packet: flags=%02X seqnum = 0x%02X offset=%lu length=%u", flags, seqNum, _offset, _frameLength);

  //Start processing
  _processing = true;            

  return AMBL_DATA_SKIP;
}



#endif // (defined(ESP8266) || defined(ESP32) )