#ifndef __SOUNDCAPTURE_H
#define __SOUNDCAPTURE_H


#define SC_MAX_BANDS        7    //Number of bands

#define SC_MAP_USE_MIN      0x01 //Use running min for lower boundary
#define SC_MAP_USE_MAX      0x02 //Use running max for upper boundary
#define SC_MAP_ABOVE_NOISE  0x04 //Make 0 everything below noise threshold
#define SC_MAP_LOG          0x10 //Logarithmic scale

#define SOUND_LOWER_MIN     0    //Deault minimum in scale from 0 to 255
#define SOUND_UPPER_MAX     255  //Default maximum in scale from 0 to 255


////////////////////////////
// Sound capture base interface
  
class SoundCapture{
  public:
    SoundCapture();
    ~SoundCapture();

    //Interface functions
    virtual void init()                              = 0;  //Initialization
    virtual void reset()                             = 0;  //Reset
    virtual void getData(uint8_t *bands, 
                         size_t numBands) const      = 0;  //Retrieve the data
    virtual void idle()                              = 0;  //Do something while there is no activity
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
    void getData(uint8_t *bands, 
                 size_t numBands) const;
    void idle();

  private:
    uint8_t _pinAnalog;
    uint8_t _pinStrobe:4;
    uint8_t _pinReset:4;
};



#endif //__SOUNDCAPTURE_H
