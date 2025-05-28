#include <Arduino.h>

#include "DbgTool.h"
#include "SoundCapture.h"




////////////////////////////
//SoundCapture
SoundCapture::SoundCapture(){
}


SoundCapture::~SoundCapture(){
}


void SoundCapture::resetStats(){

  _min.reset();
  _max.reset();
  _mean.reset();
  _meanBass.reset();
  _meanMid.reset();
  _meanTreble.reset();

  _curMin = SOUND_UPPER_MAX;
  _curMax = SOUND_LOWER_MIN;
  _count  = 0;

  for(size_t i = 0; i < SC_MAX_BANDS; i++){
    _noiseFloor[i] = SOUND_UPPER_MAX;
  }
}

const RunningStats& SoundCapture::getStats(SoundStatGet ssg) const{
  
  switch(ssg){
    case ssgMin:
    return _min;

    case ssgMax:
    return _max;

    case ssgAverage:
    return _mean;

    case ssgAverageBass:
    return _meanBass;

    case ssgAverageMid:
    return _meanMid;

    case ssgAverageTreble:
    return _meanTreble;
  }

  //Return average by default
  return _mean;
}

#define SOUNDSTAT_MAX_COUNT 70
#define NOISE_FALL_ALPHA 25
#define NOISE_RAISE_ALPHA 6


void SoundCapture::getProcessedData(sc_band_t &bands){ 

  //Get data
  getData(bands);

  //Process data
  for(size_t i = 0; i < SC_MAX_BANDS; i++){

    //Check for noise floor    
    _noiseFloor[i] = u16Smooth(_noiseFloor[i], bands[i], bands[i] > _noiseFloor[i]  ? NOISE_RAISE_ALPHA : NOISE_FALL_ALPHA);
    //Check for noise floor  
    
    
    //Check for noise floor
    bands[i] = _noiseFloor[i] > bands[i] ? 0 : bands[i] - _noiseFloor[i];

    //Bass, mid and treble

    if(i < 2){ //add bass
      _meanBass.add(bands[i]);  
    }
    else if(i < 4){ //Add mid
      _meanMid.add(bands[i]);
    }
    else{ //Add treble
      _meanTreble.add(bands[i]);
    }
    

    //Mean
    _mean.add(bands[i]);
    
    
    //Min
    if(_curMin > bands[i] && bands[i] != 0){
      _curMin = bands[i];
    }

    //Max
    if(_curMax < bands[i]){
      _curMax = bands[i];
    }   

    //Min and max collection counter
    _count++;

  }     

  //Update min and max statistics if its time
  if(_count >= SOUNDSTAT_MAX_COUNT){
    _min.add(_curMin);
    _max.add(_curMax);

    //DBG_OUTLN("%d %d %d %d", _min.getAverage(), _min.getStdDev(), _mean.getAverage(), _mean.getStdDev());

    _curMin  = SOUND_UPPER_MAX;      
    _curMax  = 0;

    _count = 0;
  }

}


#define SC_AUDIO_MIN 4

bool SoundCapture::isSound() const{
   return _mean.getAverage()  > getMin();
}

bool SoundCapture::isPeak(SoundStatGet ssg, uint16_t value, uint8_t sens) const{
  const RunningStats &stat = getStats(ssg);
  
  uint16_t threshold = SOUND_MAX(stat.getAverage(), stat.getStdDev() * sens / 255);
  
  if(threshold <= getMin())
    return false;
  
  return value > threshold;
}


void SoundCapture::scaleSound(sc_band_t &bands, uint8_t flags, uint16_t lower, uint16_t upper) const{

  uint16_t mx  = (flags & SC_MAP_USE_MAX) ?  getMax() : upper;
  uint16_t mn  = (flags & SC_MAP_USE_MIN) ?  getMin() : lower < upper ? lower : upper;  

  for(size_t i = 0; i < SC_MAX_BANDS; i++){

    uint16_t &val = bands[i];
    

    //if no signal, set to 0
    if((flags & SC_MAP_ABOVE_NOISE) && !isSound()){
      val = 0;
    }
/*
    if(flags & SC_MAP_LOG){
      //TODO - implement logarithmic scale

      //mx  = (uint8_t)((uint16_t)mx * mx / 255);
      //mn  = (uint8_t)((uint16_t)mn * mn / 255);
      //val = (uint8_t)((uint16_t)val * val / 255);

      //mx = log10(mx) / log10(255) * 255;
      //mn = log10(mn) / log10(255) * 255;
      //val = val == 0 ? 0 : log10(val) / log10(255) * 255;
    }  
  */

    val =  mapEx(val > mx ? mx : val < mn ? mn : val, mn, mx, SOUND_LOWER_MIN, SOUND_UPPER_MAX);
  }
} 


///////////////////////////////////////////
// SoundCaptureMSGEQ7

SoundCaptureMSGEQ7::SoundCaptureMSGEQ7(uint8_t pinAnalog, uint8_t pinStrobe, uint8_t pinReset){
  _pinAnalog = pinAnalog;
  _pinStrobe = pinStrobe;
  _pinReset  = pinReset;

  //analogReference(EXTERNAL);
}

SoundCaptureMSGEQ7::~SoundCaptureMSGEQ7(){
}

void SoundCaptureMSGEQ7::init(){
  //Setup pins
  pinMode(_pinAnalog, INPUT);
  pinMode(_pinStrobe, OUTPUT);
  pinMode(_pinReset, OUTPUT);

  reset();
}


void SoundCaptureMSGEQ7::reset(){  
  //Initialize digital pins
  digitalWrite(_pinReset, LOW);
  digitalWrite(_pinStrobe, HIGH); 
}


void SoundCaptureMSGEQ7::getData(sc_band_t &bands) const{
  //Reset
  digitalWrite(_pinReset, HIGH); 
  digitalWrite(_pinReset, LOW);

  //Get each band value
  for(size_t i = 0; i < SC_MAX_BANDS; i++){
    //Prepare
    digitalWrite(_pinStrobe, HIGH);
    digitalWrite(_pinStrobe, LOW);

    //Allow output to settle
    delayMicroseconds(36);

    //Read data and convert to 8-bit value
    //bands[i] =  ((uint16_t)analogRead(_pinAnalog) * 255 + 511) / 1023;
    bands[i] =  (uint16_t)analogRead(_pinAnalog);

    //DBG_OUT("%u ", data.bands[i]);
  }

  //DBG_OUTLN("");
}

void SoundCaptureMSGEQ7::idle(){
}