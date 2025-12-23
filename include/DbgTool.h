#ifndef __DBGTOOL_H
#define __DBGTOOL_H

#include "arduinolib.h"


#if defined(DBG_TO_SERIAL) || defined(DBG_TO_SW_SERIAL)
  void _dbgInit();
  void _dbgOut(const __FlashStringHelper *fmt, ...);

  #define DBG_INIT() _dbgInit();
  
  #define DBG_OUT(FORMAT, ...)  _dbgOut(F(FORMAT), ##  __VA_ARGS__);     
  #define DBG_OUTLN(FORMAT, ...)  DBG_OUT(FORMAT "\n", ##  __VA_ARGS__); 
  
#else
  #define DBG_INIT() 
  #define DBG_OUT(...)
  #define DBG_OUTLN(...)  
#endif




#endif //__DBGTOOL_H
