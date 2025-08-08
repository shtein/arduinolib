#ifndef __NOTIFICATION_H
#define __NOTIFICATION_H

#include "arduinolib.h"

/////////////////////////////
// Base interface to send notifications

//Context flags
#define NTF_CTX_NONE   0x00 
#define NTF_CTX_ARRAY  0x01


class NtfBase {
public:
  NtfBase();

  //Actions
  virtual void reset() = 0;
  virtual void send() = 0; 
  
  //Block management
  virtual void begin(const char *key = NULL) = 0;
  void begin_F(const char *key = NULL);
  virtual void end() = 0;
  

  virtual void beginArray(const char *key = NULL) = 0;
  void beginArray_F(const char *key = NULL);
  virtual void endArray() = 0;
  


  //Primitives
  virtual void put(const char *key, bool v) = 0;
  virtual void put(const char *key, uint8_t v) = 0;
  virtual void put(const char *key, uint16_t v) = 0;
  virtual void put(const char *key, uint32_t v) = 0;
  virtual void put(const char *key, int8_t v) = 0;
  virtual void put(const char *key, int16_t v) = 0;  
  virtual void put(const char *key, int32_t v) = 0;  
  virtual void put(const char *key, const char *v) = 0;
  void put_F(const char *key, const char *v);
  void put_P(const char *key, const char *v);
  void put(const char *key, const __FlashStringHelper *v);
  void put_FP(const char *key, const char *v);


  
  template <class T>
  void put(const char *key, const T &t);

  template<class T>
  void put(const char *key, const T*, size_t size);

  template<class T>
  void put_F (const char *key, const T &t);

  template<class T>
  void put_F(const char *key, const T*, size_t size);

  
  //Context manupulation
  struct CONTEXT{
    uint8_t flags;
    size_t  arrayIndex;
  }; 

  const CONTEXT& getContext() const; 
  void setContext(const CONTEXT &context);

private:
  struct CONTEXT _context;
};

inline NtfBase::NtfBase(){
  _context.flags      = NTF_CTX_NONE;
  _context.arrayIndex = 0;
}

template< class T>
inline void NtfBase::put_F(const char *key, const T &t){
  put((const char *)Progmem2Str24(key), t);
}

inline void NtfBase::put_F(const char *key, const char *v){
  put((const char *)Progmem2Str24(key), v);
}

template<class T>
inline void NtfBase::put_F(const char *key, const T *t, size_t size){
  put((const char *)Progmem2Str24(key), t, size);
}

inline void NtfBase::begin_F(const char *key){
  begin((const char *)Progmem2Str24(key));
}

inline void NtfBase::beginArray_F(const char *key){
  beginArray((const char *)Progmem2Str24(key));
}

template <class T>
void NtfBase::put(const char *key, const T &t){
  begin(key);
  
  putNtfObject(*this, t);

  end();
}

template<class T>
void NtfBase::put(const char *key, const T *t, size_t size){
  beginArray(key);

  //Remeber current context
  CONTEXT ctx = getContext();

  //Init context for array;
  _context.flags      = NTF_CTX_ARRAY;
  _context.arrayIndex = 0;

  for(size_t i = 0; i < size; i++){    
    //Serialize
    put(NULL, t[i]);

    //Increment array index
    _context.arrayIndex ++;
  }

  //Restore context
  setContext(ctx);

  endArray();
}

inline void NtfBase::put_P(const char *key, const char *v ){
  put(key, (const char *)Progmem2Str24(v));
}

inline void NtfBase::put_FP(const char *key, const char *v){
  put_P((const char *)Progmem2Str24(key), v);
}

inline void NtfBase::put(const char *key, const __FlashStringHelper *v ){
  put_P(key, (const char *)v);
}

inline const NtfBase::CONTEXT& NtfBase::getContext() const{
  return _context;
}

inline void NtfBase::setContext(const NtfBase::CONTEXT &context){
  _context = context;
}

///////////////////////////////
// For custom struct to be able to use with NtfBase::put(NtfBase &resp, T), 
// implementation of the following function is required

// void putNtfObject(NtfBase &resp, const T &data){
//   resp.put_F(F("key1"), data.member1);
//   resp.put_F(F("key2"), data.member2);
//   ...
// }


////////////////////////////////////////
// Set of notifiers to call all at the same time
template <size_t N>
class NtfBaseSet{
public:
  NtfBaseSet();

  void addNtf(NtfBase *p);

  template <class T>
  void put(const T& t);

  template <class T>
  void put(const T* t, size_t size);

private:
  NtfBase *_ntf[N]; 
}; 

template <size_t N>
NtfBaseSet<N>::NtfBaseSet(){
  memset(&_ntf, 0, sizeof(_ntf)); 
}

template <size_t N>
void NtfBaseSet<N>::addNtf(NtfBase *p){
  for(size_t i = 0; i < N; i++){
    //Find first available slot
    if(!_ntf[i]){
      _ntf[i] = p;
      return;
    }      
  }    
}


template<size_t N>
template<class T>
void NtfBaseSet<N>::put(const T& t){

  for(size_t i = 0; i < N  && _ntf[i] != NULL; i++){
    _ntf[i]->reset();
    _ntf[i]->put(NULL, t);
    _ntf[i]->send();
  }  
}



template<size_t N>
template<class T>
void NtfBaseSet<N>::put(const T *t, size_t size){

  for(size_t i = 0; i < N  && _ntf[i] != NULL; i++){
    _ntf[i]->reset();
    _ntf[i]->put(NULL, t, size);
    _ntf[i]->send();
  }  
}

#ifndef MAX_NTF
  #define MAX_NTF 2
#endif //MAX_NTF

typedef NtfBaseSet<MAX_NTF> NtfSet;

//////////////////////////////////
// Structure for command response
DECLARE_STR_PROGMEM(rs_Cmd)
DECLARE_STR_PROGMEM(rs_Data)
DECLARE_STR_PROGMEM(rs_Error)


template <typename ...Ts>
struct CmdResponse{};

template<>
struct CmdResponse<>{
  uint8_t cmd;
  uint8_t error;
};

template<typename T> 
struct CmdResponse<T>{
  uint8_t cmd;
  T data;
}; 

inline void putNtfObject(NtfBase &resp, const CmdResponse<> &r){
  resp.put_F(rs_Cmd, r.cmd);
  resp.put_F(rs_Error, r.error);
}

//Default serialization
template<typename T>
void putNtfObject(NtfBase &resp, const CmdResponse<T> &r){
  resp.put_F(rs_Cmd, r.cmd);
  resp.put_F(rs_Data, r.data);
}



#endif //__NOTIFICATION_H