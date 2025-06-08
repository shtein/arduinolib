#include <Arduino.h>

#include "DbgTool.h"
#include "SoundCapture.h"




////////////////////////////
//SoundCapture
SoundCapture::SoundCapture() :_meanBass(25){
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


#define NOISE_FALL_ALPHA  35
#define NOISE_RAISE_ALPHA 10
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

#define SOUNDSTAT_MAX_COUNT 70

void SoundCapture::getProcessedData(sc_band_t &bands){ 

  //Get data
  getData(bands);

  uint16_t average = 0;
  uint16_t averageBass = 0;
  uint16_t averageMid = 0;
  uint16_t averageTreble = 0;


  //Process data
  for(size_t i = 0; i < SC_MAX_BANDS; i++){

    //Update noise floor and band
    
    NoiseFloor &noiseFloor = (NoiseFloor &)_noiseFloor[i];
    noiseFloor.reCalculate(bands[i]);
    bands[i] = noiseFloor.value > bands[i] ? 0 : bands[i] - noiseFloor.value;
    
    //Averages
    average += bands[i];
    if(i < 2) averageBass += bands[i]; //First two bands are bass
    else if(i < 4) averageMid += bands[i]; //Next two bands are mid
    else averageTreble += bands[i]; //Last three bands are treble


    //Min
    if(_curMin > bands[i] ){
      //_curMin = bands[i] != 0 ? bands[i] : _curMin; //u16Smooth(_curMin, 0, NOISE_FALL_ALPHA);      
      _curMin = bands[i];
    }

    //Max
    if(_curMax < bands[i]){
      _curMax = bands[i];
    }   

    //Min and max collection counter
    _count++;    

  }     

  //DBG_OUTLN("");
  
  //Mean
  _mean.add(average / SC_MAX_BANDS); //Add mean, first
  //Bass
  _meanBass.add(averageBass / 2); //Add bass mean, first
  //Mid 
  _meanMid.add(averageMid / 2);  //Add mid mean, second
  //Treble
  _meanTreble.add(averageTreble / 3); //Add treble mean, last three

  //Min and Max
  if(_count >= SOUNDSTAT_MAX_COUNT){

    _min.add(_curMin);
    _max.add(_curMax);    

    _curMin  = SOUND_UPPER_MAX;      
    _curMax  = SOUND_LOWER_MIN;

    _count = 0;

    //DBG_OUTLN("%d %d, %d %d, %d %d, %d %d, %d %d", _min.getAverage(), _min.getStdDev(), _mean.getAverage(), _mean.getStdDev(), _meanBass.getAverage(), _meanBass.getStdDev(), _meanMid.getAverage(), _meanMid.getStdDev(), _meanTreble.getAverage(), _meanTreble.getStdDev());       
  }

  
}


bool SoundCapture::isSound() const{
   //return _mean.getAverage() > getMin();

  uint16_t avg    = _mean.getAverage();
  uint16_t stddev = _mean.getStdDev();
  uint16_t min    = _min.getAverage();
  

  // Auto-tune thresholds based on recent signal stats
  uint16_t dynamicMinLevel = min + stddev / 2;     // floor rises if noise increases
  uint16_t dynamicMinGap   = stddev / 3;           // larger variance demands a bigger gap

  // Clamp minimums
  if (dynamicMinLevel < 3) dynamicMinLevel = 3;
  if (dynamicMinGap < 2)   dynamicMinGap   = 2;

  return (avg >= dynamicMinLevel) && (avg > min + dynamicMinGap);
}


#define U16_SCALE(val, sens) ((uint16_t)(val * sens + 127) / 255) //Scale value by sensitivity, with rounding

bool SoundCapture::isPeak(SoundStatGet ssg, uint16_t value, uint16_t sensForBandAvg, uint16_t sensForOvrlAvg) const{
 
  if (!isSound())
    return false;

 
  const RunningStats &stat = getStats(ssg);

  //Band and overall statistics
  uint16_t avgBand    = stat.getAverage();
  uint16_t stddevBand = stat.getStdDev();      
  uint16_t avg        = _mean.getAverage();
  uint16_t stddev     = _mean.getStdDev();  //Scale stddev by sensitivity, with rounding

  

  //Is value a peak for this band?
  if(value < max(SOUND_MAX(avgBand, U16_SCALE(stddevBand, sensForBandAvg)), getMin()))
    return false;


  //See if there enough signal to consider a peak, avgBand < avg - stddev  
  if (avgBand + U16_SCALE(stddev, sensForOvrlAvg) < avg)
    return false; 
  
/*
  //Is value a peak for the overall sound?
  dynThreshold = max(SOUND_MAX(avg, stddev), getMin());
  if(value < dynThreshold)
    return false;
*/    

  return true;
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