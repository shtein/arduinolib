#include "arduinolib.h"
#include "alutils.h"
#include "DBGTool.h"


long mapEx( long x, long in_min, long in_max, long out_min, long out_max ){
  long out_range;
  long in_range;

  out_range = out_max - out_min;
  if (out_range > 0)
    ++out_range;
  else if (out_range < 0)
    --out_range;

  in_range = in_max - in_min;
  if ( in_range > 0 )
    ++in_range;
  else if ( in_range < 0 )
    --in_range;
  else
    return 0x7FFFFFFF;

  return (x - in_min) * out_range / in_range + out_min;
}


int powInt(int x, int y, int limit){
  int n = 1;

  if(y > limit)
    y = limit;
  
  for(int i = 0; i < y; i++){
    n = n * x;
  }

  return n; 
}


///////////////////////////////////////////
//Mapping to logarith scale for 8bit value
uint8_t u8Log2(uint8_t val){
  uint8_t r = 0;
  while(val >>= 1)
    r++;
  return r;
}

uint8_t u8MapLog2(uint8_t val){
  if(val == 0)
    return 0;

  uint16_t log = u8Log2(val) * 255;
  return (uint8_t)(log / 8);
}

///////////////////////////////////////////
// Square root for 16 bit value
uint8_t u8Sqrt(uint16_t val) {

    if (val == 0 || val == 1) {
        return (uint8_t)val; // Square root of 0 is 0, and square root of 1 is 1
    }

    uint16_t r = val; // Initial guess
    uint16_t prevR = 0; // Previous guess to check for convergence

    // Iterate until convergence or a maximum of 6 iterations
    for (uint8_t i = 0; i < 6; i++) {
        prevR = r;
        r = (r + val / r) / 2;

        // Break early if there's no change between iterations
        if (r == prevR) {
            break;
        }
    }

    // Ensure the result fits within uint8_t bounds
    return (r > 255) ? 255 : (uint8_t)r;
}

///////////////////////////////////////////
// Square root for 32 bit value
uint16_t u16Sqrt(uint32_t val){
    
    if (val == 0 || val == 1) {
        return (uint16_t)val; // Square root of 0 is 0, and square root of 1 is 1
    }

    uint32_t r = val; // Initial guess
    uint32_t prevR = 0; // Previous guess to check for convergence

    // Iterate until convergence or a maximum of 6 iterations
    for (uint8_t i = 0; i < 6; i++) {
        prevR = r;
        r = (r + val / r) / 2;

        // Break early if there's no change between iterations
        if (r == prevR) {
            break;
        }
    }

    // Ensure the result fits within uint16_t bounds
    return (r > 65535) ? 65535 : (uint16_t)r;
}

////////////////////////////////////////
// Smooth value for 16 bit value
uint16_t u16Smooth(uint16_t old, uint16_t val, uint8_t smoothFactor){
  
     // Convert old and val to Q8.8 fixed-point
    uint32_t old_q8 = ((uint32_t)old) << 8;
    uint32_t val_q8 = ((uint32_t)val) << 8;

    int32_t delta = (int32_t)val_q8 - (int32_t)old_q8;

    // Apply smoothing factor in Q8.8 domain with rounding
    old_q8 += ((delta * smoothFactor + 127) / 255);

    // Convert back to uint16_t by discarding fractional part
    return (uint16_t)(old_q8 >> 8);
}



/////////////////////////////////////
// RunningStats
RunningStats::RunningStats(uint8_t smoothFactor){
  _smoothFactorRise = smoothFactor;
  _smoothFactorFall = smoothFactor;

  reset();
} 

RunningStats::RunningStats(uint8_t smoothFactorFall, uint8_t smoothFactorRise ){
  _smoothFactorFall = smoothFactorFall;
  _smoothFactorRise = smoothFactorRise;

  reset();
} 


void RunningStats::reset(){
  _mean     = 0;
  _variance = 0;
}


void RunningStats::add(uint16_t val){
  //Mean
  uint8_t smoothFactor = (_mean > val) ? _smoothFactorFall : _smoothFactorRise;
  _mean = u16Smooth(_mean, val, smoothFactor);

  //Varience
  int32_t delta = (int32_t)val - (int32_t)_mean;
  // Calculate the square of the delta
  uint32_t deltaSq = (uint32_t)(delta * delta);

  
  _variance = ((_variance * (255 - smoothFactor)) + (deltaSq * smoothFactor) + 127) >> 8;
}

uint16_t RunningStats::getAverage() const{
  return _mean;
}

uint16_t RunningStats::getStdDev() const{
  return u16Sqrt(_variance); // Return the square root of variance as standard deviation
}

