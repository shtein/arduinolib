#ifndef __ALUTILS_H
#define __ALUTILS_H

#include "arduinolib.h"

//////////////////////////////
// Swap
template <typename T>
inline constexpr void swapIf(T &a, T &b) {
    if (a > b) {
        T c = a;
        a = b;
        b = c;
    }
}

#define SWAPIF(a, b) swapIf(a, b)

template <typename T>
inline uint8_t random8_ab(T a, T b){
  swapIf(a, b);
  return random8(a, b + 1);
}


#define RANDOM8_AB(a, b) random8_ab((int8_t)(a), (int8_t)(b))

/////////////////////////////////
//Integers types
template<typename T>
struct upper_type;

template<> struct upper_type<int8_t>  { using type = int16_t; };
template<> struct upper_type<int16_t> { using type = int32_t; };
template<> struct upper_type<int32_t> { using type = int64_t; };
template<> struct upper_type<uint8_t>  { using type = uint16_t; };
template<> struct upper_type<uint16_t> { using type = uint32_t; };
template<> struct upper_type<uint32_t> { using type = uint64_t; };

template<typename T>
using upper_type_t = typename upper_type<T>::type;

template<typename T>
struct unsigned_type;

template<> struct unsigned_type<int8_t>  { using type = uint8_t; };
template<> struct unsigned_type<int16_t> { using type = uint16_t; };
template<> struct unsigned_type<int32_t> { using type = uint32_t; };
template<> struct unsigned_type<int64_t> { using type = uint64_t; };

template<typename T>
using unsigned_type_t = typename unsigned_type<T>::type;


template<typename T>
struct int_limits;

// int8_t
template <> struct int_limits<int8_t> {
    static constexpr int8_t  min = INT8_MIN;
    static constexpr int8_t  max = INT8_MAX;
};

// int16_t
template<> struct int_limits<int16_t> {
    static constexpr int16_t min = INT16_MIN;
    static constexpr int16_t max = INT16_MAX;
};

// int32_t
template<> struct int_limits<int32_t> {
    static constexpr int32_t min = INT32_MIN;
    static constexpr int32_t max = INT32_MAX;
};

// uint8_t
template <> struct int_limits<uint8_t> {
    static constexpr uint8_t  min = 0;
    static constexpr uint8_t  max = UINT8_MAX;
};

// uint16_t
template<> struct int_limits<uint16_t> {
    static constexpr uint16_t min = 0;
    static constexpr uint16_t max = UINT16_MAX;
};

// uint32_t
template<> struct int_limits<uint32_t> {
    static constexpr uint32_t min = 0;
    static constexpr uint32_t max = UINT32_MAX;
};


template<typename T>
T divRound(T a, T b) {
  return (a >= 0) == (b >= 0)? (a + b / 2) / b : (a - b / 2) / b;
}

#define DIV_ROUND(a, b) divRound((a), (b))

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
int16_t powInt(int16_t x, int16_t y, int16_t limit);


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

#ifdef ARDUINO

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

#define PROGMEM_TO_STR(s) (const char *)Progmem2Str24(s)

//String resources
#define DEFINE_STR_PROGMEM(k, v) const char k[] PROGMEM = v;
#define DECLARE_STR_PROGMEM(k) extern const char k[] PROGMEM;



//Time checks
#define SET_MILLIS(var) \
  var = (uint16_t)millis(); \
  if(var == 0) var = 1;

#define DELTA_MILLS(var) ((uint16_t)millis() - var )

#endif //ARDUINO


///////////////////////////////////////////
// Square root 
template<typename T> 
T uSqrt(upper_type_t<T> val){
    
    if (val == 0 || val == 1) {
        return (T)val; // Square root of 0 is 0, and square root of 1 is 1
    }

    upper_type_t<T> r = val;   // Initial guess
    upper_type_t<T> prevR = 0; // Previous guess to check for convergence

    //Initial guess
    uint8_t idx = 0;
    while (r >>= 1) 
      ++idx; // small loop: 0..(bits-1)  

    r = (upper_type_t<T> )1 << ((idx + 1) / 2);

    // Iterate until convergence or a maximum of 6 iterations
    for (uint8_t i = 0; i < 6; i++) {
        prevR = r;
        r = (r + val / r) / 2;

        // Break early if there's no change between iterations
        if (r == prevR) {
            break;
        }
    }

    // Correct to floor(sqrt(val)) 
    for (int i = 0; i < 2 && r > val / r; i++) 
      r--;

    for (int i = 0; i < 2 && (r + 1) != 0 && (r + 1) <= val / (r + 1); i++) 
      r++;

    T mx = int_limits<T>::max;

    // Ensure the result fits within  bounds
    return (r > mx ) ? mx : (T)r;
}

//Log2 and sqrt
uint8_t u8Log2(uint8_t val);
uint8_t u8MapLog2(uint8_t val);

// Smooth value for 16 bit value
uint16_t u16Smooth(uint16_t old, uint16_t val, uint8_t smoothFactor);

#define U16_SCALE(val, sens) ((uint16_t)(val * sens + 127) / 255) //Scale value by sensitivity, with rounding

/////////////////////////////////////////////
// Running statistics
class RunningStats{ 
  public:
    RunningStats(uint8_t smoothFactor = 25);
    RunningStats(uint8_t smoothFactorFall, uint8_t smoothFactorRise);

    void reset();
    void add(uint16_t val);

    uint16_t getAverage() const;
    uint16_t getStdDev() const;
  
  private:
    uint16_t  _mean;     //Mean value
    uint16_t  _variance; //Variance value
    
    uint8_t   _smoothFactorFall; //Smoothing factor for falling values
    uint8_t   _smoothFactorRise; //Smoothing factor for rising values
};



#endif //_ALUTILS_H