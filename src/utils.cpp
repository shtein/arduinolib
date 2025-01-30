#include "arduinolib.h"


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
