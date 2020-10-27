#ifndef __SOUNDCAPTURE_H
#define __SOUNDCAPTURE_H


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
                         uint16_t numBands) const    = 0;  //Retrieve the data
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
                 uint16_t numBands) const;
    void idle();

  private:
    uint8_t _pinAnalog;
    uint8_t _pinStrobe:4;
    uint8_t _pinReset:4;
};



#endif //__SOUNDCAPTURE_H
