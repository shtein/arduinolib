#include <Arduino.h>

#include "DbgTool.h"
#include "SoundCapture.h"




////////////////////////////
//SoundCapture
SoundCapture::SoundCapture():_max(15, 100){
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

  _bass = SOUND_LOWER_MIN;
  _soundBass  = true;
  _ssTimeBass = 0;

  _mid  = SOUND_LOWER_MIN;
  _soundTreble  = true;
  _ssTimeMid = 0; 

  _treble = SOUND_LOWER_MIN;
  _soundTreble  = true;
  _ssTimeTreble = 0;  

  _curMin  = SOUND_UPPER_MAX;
  _curMax  = SOUND_LOWER_MIN;
  _countMM = 0;

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


#define NOISE_FALL_ALPHA  25
#define NOISE_RAISE_ALPHA 15
#define NOISE_RAISE_HOLD  10

struct NoiseFloor{
  union 
  {
    struct {
      uint16_t value:10;       //Noise floor value
      uint16_t raiseCnt:6;     //Counter for noise raise
    };
    uint16_t raw;              //Raw value
  };

  void reCalculate(uint16_t newValue){

    if(newValue > value){

      //Increas raise counter
      raiseCnt += 1;

      //If hold time is reached, raise noise floor
      if(raiseCnt >= NOISE_RAISE_HOLD){
        
        raiseCnt = 0;
          
        uint16_t old = value;
        value = u16Smooth(value, newValue, NOISE_RAISE_ALPHA);

        //If no change, increase counter        
        if(old == value){                  
          value = value + 1;
        }    
      }
    }
    else{
      //Reset raise counter
      raiseCnt = 0;

      //Fall noise floor      
      value = u16Smooth(value, newValue, NOISE_FALL_ALPHA);  
    }

  }

};


bool detectSound(uint16_t min, const RunningStats &stat){

  uint16_t avg     = stat.getAverage();
  uint16_t stddev  = stat.getStdDev(); 

  // Auto-tune thresholds based on recent signal stats
  uint16_t dynamicMinLevel = min + U16_SCALE(stddev, 128);
  uint16_t dynamicMinGap   = U16_SCALE(stddev, 128);

  // Clamp minimums
  if (dynamicMinLevel < 3) dynamicMinLevel = 3;
  if (dynamicMinGap < 2)   dynamicMinGap   = 2;

  bool sound = ((avg >= dynamicMinLevel) && (avg > min + dynamicMinGap));

  return sound;
}


#define SOUNDSTAT_MAX_COUNT 70

void SoundCapture::getProcessedData(sc_band_t &bands){ 

  //Get data
  getData(bands);

  uint16_t avg = 0;

  //Process data
  for(size_t i = 0; i < SC_MAX_BANDS; i++){

    //Update noise floor and band
    
    NoiseFloor &noiseFloor = (NoiseFloor &)_noiseFloor[i];
    noiseFloor.reCalculate(bands[i]);
    bands[i] = noiseFloor.value > bands[i] ? 0 : bands[i] - noiseFloor.value;
    
    avg += bands[i];
  
    //Min
    if(_curMin > bands[i] ){
      _curMin = bands[i];
    }

    //Max
    if(_curMax < bands[i]){
      _curMax = bands[i];
    }   

    //Min and max collection counter
    _countMM++;    
  }     

  //Average
  _mean.add(avg/SC_MAX_BANDS); //Add mean, first
    
  //Bass
   uint16_t tmp = (bands[0] + bands[1]) / 2;
   _meanBass.add(tmp); //Add bass mean, first
   _bass = u16Smooth(_bass, tmp,  tmp > _bass ? 128 : 192); //Fast bass value
  
    //Mid
  tmp = (bands[2] + bands[3]) / 2;
  _meanMid.add(tmp);  //Add mid mean, second
  _mid = u16Smooth(_mid, tmp, tmp < _mid ? 128 : 192); //Fast mid value
    
    //Treble
  tmp = (bands[4] + bands[5] + bands[6]) / 3;
  _meanTreble.add(tmp); //Add treble mean, last three        
  _treble = u16Smooth(_treble, tmp, tmp < _treble ? 96 : 192); //Fast treble value    
  

  //Min and Max
  if(_countMM >= SOUNDSTAT_MAX_COUNT){

    _min.add(_curMin);
    _max.add(_curMax);    

    DBG_OUTLN("%d %d, %d %d, %d %d, %d %d - %d", _mean.getAverage(), _mean.getStdDev(), _meanBass.getAverage(), _meanBass.getStdDev(), _meanMid.getAverage(), _meanMid.getStdDev(), _meanTreble.getAverage(), _meanTreble.getStdDev(), isSound(true, 50));       
/*
    for(size_t i = 0; i < SC_MAX_BANDS; i++){
      DBG_OUT("%d ", bands[i]);  
    }

    DBG_OUTLN(" - %d", _sound);
*/    

    _curMin  = SOUND_UPPER_MAX;      
    _curMax  = SOUND_LOWER_MIN;

    _countMM = 0;
  }

  //Check if silence changed
  if(_soundBass != detectSound(_min.getAverage(), _meanBass )){
    _soundBass = !_soundBass;
    SET_MILLIS(_ssTimeBass);
  }

  if(_soundMid != detectSound(_min.getAverage(), _meanMid)){
    _soundMid = !_soundMid;
    SET_MILLIS(_ssTimeMid);
  }
  
  if(_soundTreble != detectSound(_min.getAverage(), _meanTreble)){
    _soundTreble = !_soundTreble;
    SET_MILLIS(_ssTimeTreble);
  }
}

bool SoundCapture::isPeak(SoundStatGet ssg, uint16_t value, uint8_t sensForBandAvg, uint8_t sensForAvg) const{

  const RunningStats &stat = getStats(ssg);

  //Band and overall statistics
  uint16_t avgBand    = stat.getAverage();
  uint16_t stddevBand = stat.getStdDev();      
  uint16_t avg        = _mean.getAverage();
  uint16_t stddev     = _mean.getStdDev();  
  uint16_t minBand    = SOUND_MAX(avgBand, U16_SCALE(stddevBand, sensForBandAvg));


  //Check if value is above thresholds
  if(value < minBand)
    return false; 
 
  //Check if value is above average - threashold
  if(value + U16_SCALE(stddev, sensForAvg) < avg )
    return false; 

  return true;
}


void SoundCapture::scaleSound(sc_band_t &bands, uint8_t flags, uint16_t lower, uint16_t upper) const{

  uint16_t mx  = (flags & SC_MAP_USE_MAX) ?  getMax() : upper;
  uint16_t mn  = (flags & SC_MAP_USE_MIN) ?  getMin() : lower < upper ? lower : upper;  

  for(size_t i = 0; i < SC_MAX_BANDS; i++){

    uint16_t &val = bands[i];

    bool sound = false;
    if(i < 2){
      sound = isBassSound(true, 50);
    }
    else if(i < 4){
      sound = isMidSound(true, 50);
    }
    else{
      sound = isTrebleSound(true, 50);
    }
    

    //if no signal, set to 0
    if((flags & SC_MAP_ABOVE_NOISE) && !sound){
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

  //DBG_OUTLN("%d %d, %d %d, %d %d", _mean.getAverage(), _mean.getStdDev(), _min.getAverage(), _min.getStdDev(), _max.getAverage(), _max.getStdDev()); 
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