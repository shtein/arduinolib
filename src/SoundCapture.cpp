#include <Arduino.h>

#include "DbgTool.h"
#include "SoundCapture.h"

////////////////////////////
//SoundCapture
SoundCapture::SoundCapture(){
}


SoundCapture::~SoundCapture(){
}



///////////////////////////////////////////
// SoundCaptureMSGEQ7

#define MSGEQ7_AUDI_MIN 8

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


void SoundCaptureMSGEQ7::getData(uint8_t *bands, 
                                 size_t numBands) const{
  //Reset
  digitalWrite(_pinReset, HIGH); 
  digitalWrite(_pinReset, LOW);

  numBands = min(numBands, (uint16_t)MSGEQ7_BANDS);

  for(uint8_t i = 0; i < numBands; i++){
    //Prepare
    digitalWrite(_pinStrobe, HIGH);
    digitalWrite(_pinStrobe, LOW);

    //Allow output to settle
    delayMicroseconds(36);

    //Read data    
    bands[i] = analogRead(_pinAnalog);
    bands[i] =  ((uint16_t)bands[i] * 255 + 511) / 1023;
    //DBG_OUT("%u ", data.bands[i]);
  }

  //DBG_OUTLN("");
}

void SoundCaptureMSGEQ7::idle(){
}