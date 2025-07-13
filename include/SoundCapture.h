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


inline uint16_t soundMax(uint16_t a, uint16_t b) {
  return a + b > SOUND_UPPER_MAX ? SOUND_UPPER_MAX : a + b; 
}

#define SOUND_MAX(a, b) soundMax(a, b) //Maximum value for sound


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
    virtual void getData(sc_band_t &bands) const     = 0;  //Retrieve the data

    //Main processing function
    void getProcessedData(sc_band_t &bands);
    
    //Statistics functions
    void resetStats();                                    //Reset statistics              
    const RunningStats& getStats(SoundStatGet ssg) const; //Get statistics

    //State functions
    bool isBassSound(bool sound, uint16_t delta) const { return _soundBass == sound && DELTA_MILLS(_ssTimeBass) > delta; };              //Is sound detected for bass
    bool isMidSound(bool sound, uint16_t delta) const { return _soundMid == sound && DELTA_MILLS(_ssTimeMid) > delta; };                 //Is sound detected for mid
    bool isTrebleSound(bool sound, uint16_t delta) const { return _soundTreble == sound && DELTA_MILLS(_ssTimeTreble) > delta; };        //Is sound detected for treble

    bool isSound(bool sound, uint16_t delta) const{ return isBassSound(sound, delta) || isMidSound(sound, delta) || isTrebleSound(sound, delta);}            //Is sound detected in any band

    void scaleSound(sc_band_t &bands, uint8_t flags, uint16_t lower = SOUND_LOWER_MIN, uint16_t upper = SOUND_UPPER_MAX) const;

    bool isPeak(SoundStatGet ssg, uint16_t value, uint8_t sensForBanAg = 128, uint8_t sensForAvg = 255) const;   //Generic peak detection
    bool isBassPeak() const {return isBassSound(true, 50) && isPeak(ssgAverageBass, _bass, 64, 158); }           //Base peak detection
    bool isMidPeak() const {return isMidSound(true, 50) && isPeak(ssgAverageMid, _mid, 158, 176); }              //Medium peak detection
    bool isTreblePeak() const {return isTrebleSound(true, 50) && isPeak(ssgAverageTreble, _treble, 192, 192); }  //Treble peak detection

    uint16_t getBass() const { return _bass; }     //Get current bass value
    uint16_t getMid() const { return _mid; }       //Get current mid value
    uint16_t getTreble() const { return _treble; } //Get current treble value
    
    uint16_t getMax() const { return SOUND_MAX(_max.getAverage(), 3 * _max.getStdDev()) ; } //Get current maximum value
    uint16_t getMin() const { return SOUND_MAX(_min.getAverage(), _min.getStdDev()) ; } //Get current minimum value

  private:
    //Min and Max
    RunningStats _min;        //Running statistics for min
    RunningStats _max;        //Running statistics for max
    uint16_t _curMin;         //Current minimum value
    uint16_t _curMax;         //Current maximum value
    uint8_t _countMM;         //Counter for min/max calculations

    //Average
    RunningStats _mean;       //Running statistics for mean

    //Bass
    RunningStats _meanBass;    //Running statistics for bass mean, first 2 bands
    uint16_t     _bass;        //Fast bass value
    bool         _soundBass;   //Bass silence or sound
    uint16_t     _ssTimeBass;  //Bass silence or sound time

    //Mid
    RunningStats _meanMid;    //Running statistics for mid mean, second 2 bands
    uint16_t     _mid;        //Fast mid value
    bool         _soundMid;   //Mid silence or sound
    uint16_t     _ssTimeMid;  //Mid silence or sound time

    //Treble
    RunningStats _meanTreble;   //Running statistics for treble mean, last 3 bands
    uint16_t     _treble;       //Fast treble value
    bool         _soundTreble;  //Treble silence or sound
    uint16_t     _ssTimeTreble; //Treble silence or sound time

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
