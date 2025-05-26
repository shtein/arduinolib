#ifndef __SOUNDCAPTURE_H
#define __SOUNDCAPTURE_H

#include "Utils.h"

//Constants and flags
#define SC_MAX_BANDS        7    //Number of bands

#define SOUND_LOWER_MIN     0     //Deault minimum in scale from 0 to 255
#define SOUND_UPPER_MAX     1023  //Default maximum in scale from 0 to 255

#define SC_MAP_USE_MIN      0x01  //Use running min for lower boundary
#define SC_MAP_USE_MAX      0x02  //Use running max for upper boundary
#define SC_MAP_ABOVE_NOISE  0x04  //Make 0 everything below noise threshold
#define SC_MAP_LOG          0x10  //Logarithmic scale


//Get statistics from 
enum SoundStatGet {
  ssgMin = 0,
  ssgMax,  
  ssgAverage,
  ssgAverageBass,
  ssgAverageMid,
  ssgAverageTreble,
};


////////////////////////////
// Sound capture base interface

typedef uint16_t sc_band_t[SC_MAX_BANDS];

  
class SoundCapture{
  public:
    SoundCapture();
    ~SoundCapture();

    //Interface functions
    virtual void init()                              = 0;  //Initialization
    virtual void reset()                             = 0;  //Reset    
    virtual void idle()                              = 0;  //Do something while there is no activity
    virtual void getData(sc_band_t &bands) const    = 0;  //Retrieve the data

    //Main processing function
    void getProcessedData(sc_band_t &bands);
    
    //Statistics functions
    void resetStats();                                    //Reset statistics              
    const RunningStats& getStats(SoundStatGet ssg) const; //Get statistics

    //State functions
    bool isSound() const; //Is sound detected
    void scaleSound(sc_band_t &bands, uint8_t flags, uint16_t lower = SOUND_LOWER_MIN, uint16_t upper = SOUND_UPPER_MAX) const;

    bool isSoundBass() const;    //Is sound in bass range
    bool isSoundMid() const ;    //Is sound in mid range
    bool isSoundTreble() const;  //Is sound in treble range

    uint16_t getMax() const { return _max.getAverage() + _max.getStdDev() * 2; } //Get current maximum value
    uint16_t getMin() const { return _min.getAverage() + _min.getStdDev() * 2; } //Get current minimum value


  private:
    RunningStats _min;        //Running statistics for min
    RunningStats _max;        //Running statistics for max
    RunningStats _mean;       //Running statistics for mean
    RunningStats _meanBass;   //Running statistics for bass mean, first 2 bands
    RunningStats _meanMid;    //Running statistics for mid mean, second 2 bands
    RunningStats _meanTreble; //Running statistics for treble mean, last 3 bands
  
    uint16_t _curMin;         //Current minimum value
    uint16_t _curMax;         //Current maximum value
    uint8_t  _count;          //Counter for min/max calculations

    uint16_t _noiseFloor[SC_MAX_BANDS]; //Noise floor for each band
};



////////////////////////////
// MSGEQ7 sound captupre

#define MSGEQ7_BANDS 7

class SoundCaptureMSGEQ7: public SoundCapture{
  public:
    SoundCaptureMSGEQ7(unsigned char pinAnalog, uint8_t pinStrobe, uint8_t pinReset);
    ~SoundCaptureMSGEQ7();

    void init();
    void reset();    
    void idle();
    void getData(sc_band_t &bands) const;

    //Getters
    uint8_t getPinAnalog() const { return _pinAnalog; }
    uint8_t getPinStrobe() const { return _pinStrobe; }
    uint8_t getPinReset() const { return _pinReset; }
  
  private:
    uint8_t _pinAnalog;
    uint8_t _pinStrobe:4;
    uint8_t _pinReset:4;
};





#endif //__SOUNDCAPTURE_H
