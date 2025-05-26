#ifndef __UTILS_H
#define __UTILS_H

//Map function
long mapEx( long x, long in_min, long in_max, long out_min, long out_max ); 

//Use with millis function and to save on unsigned long 
template<typename T>
bool cmpWithOverflow(T t, uint32_t c, T delta){  
  T t1 = (T)c;

  if(t1 > t)
    return t1 - t >= delta;
  
  return t1 + (T)(1-2) - t >= delta;
}

//Pow for integers
int powInt(int x, int y, int limit);


//Macro for handndling variable number of arguments
#define TOKEN_CONCAT(x, y) TOKEN_CONCAT_(x, y)
#define TOKEN_CONCAT_(x, y) x ## y

#define GET_NTH_ARG(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, N,...) N
#define NUM_ARGS(...) GET_NTH_ARG(, ##__VA_ARGS__ ,9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define ARG_NUM_1(P1, ...) P1
#define ARG_NUM_2(P1, P2, ...) P2
#define ARG_NUM_3(P1, P2, P3, ...) P3
#define ARG_NUM_4(P1, P2, P3, P4, ...) P4
#define ARG_NUM_5(P1, P2, P3, P4, P5, ...) P5
#define ARG_NUM_6(P1, P2, P3, P4, P5, P6, ...) P6
#define ARG_NUM_7(P1, P2, P3, P4, P5, P6, P7, ...) P7
#define ARG_NUM_8(P1, P2, P3, P4, P5, P6, P7, P8, ...) P8
#define ARG_NUM_9(P1, P2, P3, P4, P5, P6, P7, P8, P9, ...) P9
#define ARG_NUM_10(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, ...) P10

#define ARG_NUM(N, ...) TOKEN_CONCAT(ARG_NUM_, N)(__VA_ARGS__)
#define ARG_LAST(...) ARG_NUM( NUM_ARGS(__VA_ARGS__),  __VA_ARGS__)


////////////////////////////////////////////////
// const char * PROGMEM to const char converter
// usage:
// const char *p = (const char *)(Progmem2Str<>(pProgmemStr))

#define PROGMEMSTR_SIZE_MAX 24

template<size_t SIZE = PROGMEMSTR_SIZE_MAX>
class Progmem2Str{
public:
  Progmem2Str(const char *s){
    if(s){
      strncpy_P(_buf, s, sizeof(_buf));
    }
    else{
      _buf[0] = 0;
    }
  }

  operator const char *() const{
    return _buf;
  }

private:
  char _buf[SIZE];  
};


typedef Progmem2Str<> Progmem2Str24;

//String resources
#define DEFINE_STR_PROGMEM(k, v) const char k[] PROGMEM = v;
#define DECLARE_STR_PROGMEM(k) extern const char k[] PROGMEM;


//Time checks
#define SET_MILLIS(var) \
  var = (uint16_t)millis(); \
  if(var == 0) var = 1;

#define DELTA_MILLS(var) ((uint16_t)millis() - var )


//Log2 and sqrt
uint8_t u8Log2(uint8_t val);
uint8_t u8MapLog2(uint8_t val);
uint8_t u8Sqrt(uint16_t val);
uint16_t u16Sqrt(uint32_t val);

// Smooth value for 16 bit value
uint16_t u16Smooth(uint16_t old, uint32_t val, uint8_t smoothFactor);


/////////////////////////////////////////////
// Running statistics
class RunningStats{ 
  public:
    RunningStats(uint8_t smoothFactor = 25);

    void reset();
    void add(uint16_t val);

    uint16_t getAverage() const;
    uint16_t getStdDev() const;
  
  private:
    uint16_t  _mean;
    uint16_t  _variance;
    
    uint8_t   _smoothFactor;
};



#endif //_UTILS_H