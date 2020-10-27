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
