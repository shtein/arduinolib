#ifndef __SOUNDCAPTURE_H
#define __SOUNDCAPTURE_H

////////////////////////////
// Sound capture base interface

#define MAX_BANDS 7

typedef struct {
  uint8_t bands[MAX_BANDS]; //8-bit data for each band
} SoundCaptureData;

class SoundCapture{
  public:
    SoundCapture();
    ~SoundCapture();

    //Single instance operations
    static void initInstance(SoundCapture *instance); 
    static SoundCapture* getInstance(); 

    //Interface functions
    virtual void init()                                = 0;  //Initialization
    virtual void reset()                               = 0;  //Reset
    virtual void idle()                                = 0;  //Do something while there is no activity
    virtual void getData(SoundCaptureData &data) const = 0;  //Retrieve the data

  private:
    static SoundCapture *_instance;
};


////////////////////////////
// MSGEQ7 sound captupre
class SoundCaptureMSGEQ7: public SoundCapture{
  public:
    SoundCaptureMSGEQ7(unsigned char pinAnalog, uint8_t pinStrobe, uint8_t pinReset);
    ~SoundCaptureMSGEQ7();

    void init();
    void reset();
    void idle();
    void getData(SoundCaptureData &data) const;


  private:
    uint8_t _pinAnalog;
    uint8_t _pinStrobe:4;
    uint8_t _pinReset:4;
};



#endif //__SOUNDCAPTURE_H
