#ifndef __DBGTOOL_H
#define __DBGTOOL_H

#include "arduinolib.h"


#if defined(DBG_TO_SERIAL) || defined(DBG_TO_SW_SERIAL)
  void _dbgInit();
  void _dbgOut(const __FlashStringHelper *fmt, ...);

  #define DBG_INIT() _dbgInit();
  
  #define DBG_OUT(FORMAT, ...)  _dbgOut(F(FORMAT), ##  __VA_ARGS__);     
  #define DBG_OUTLN(FORMAT, ...)  DBG_OUT(FORMAT "\n", ##  __VA_ARGS__); 

#elif defined(DBG_TO_ESPHOME)
//For this we use ESPHome logging macros for debug output
//Make sure your yaml has the following line to enable debug output:
// esphome  
//   platformio_options:  
//     lib_ldf_mode: "chain"    
//     lib_deps: 
//       - https://github.com/shtein/arduinolib.git
//     build_flags: 
//       build_flags: 
//         - -Isrc

#include "esphome/core/log.h"

#define DBG_INIT()
#define DBG_OUT(FORMAT, ...)    ESP_LOGD("AL", FORMAT, ## __VA_ARGS__)
#define DBG_OUTLN(FORMAT, ...)  ESP_LOGD("AL", FORMAT, ## __VA_ARGS__)
  
#else
  #define DBG_INIT() 
  #define DBG_OUT(...)
  #define DBG_OUTLN(...)  
#endif




#endif //__DBGTOOL_H
