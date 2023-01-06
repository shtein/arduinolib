#ifndef __NOTIFICATION_H
#define __NOTIFICATION_H


/////////////////////////////
// Base interface to send notifications

class NtfBase {
public:
  //Actions
  virtual void reset() = 0;
  virtual void send() = 0;
 
  //Block management
  
  
  //Block management
  virtual void begin(const char *key = NULL) = 0;
  virtual void end(const char *key = NULL) = 0;
  void begin_F(const __FlashStringHelper *key = NULL);
  void end_F(const __FlashStringHelper *key = NULL);

  virtual void beginArray(const char *key = NULL) = 0;
  virtual void endArray(const char *key = NULL) = 0;
  void beginArray_F(const __FlashStringHelper *key = NULL);
  void endArray_F(const __FlashStringHelper *key = NULL);


  //Primitives
  virtual void put(const char *key, uint8_t v) = 0;
  virtual void put(const char *key, uint16_t v) = 0;
  virtual void put(const char *key, uint32_t v) = 0;
  virtual void put(const char *key, int8_t v) = 0;
  virtual void put(const char *key, int16_t v) = 0;  
  virtual void put(const char *key, int32_t v) = 0;  
  virtual void put(const char *key, const char *v) = 0;
  void put(const char *key, const __FlashStringHelper *v);

  template <class T>
  void put(const char *key, const T &t);

  template<class T>
  void put(const char *key, const T*, size_t size);

  template< class T>
  void put_F (const __FlashStringHelper *key, const T &t);

  template< class T>
  void put_F(const __FlashStringHelper *key, const T*, size_t size);
};


#define NTF_FUNC_F(name, key, ...) \
   if(key){ \
    char buf[16]; \
    strncpy_P(buf, (const char *)key, sizeof(buf) - 1); \
    buf[sizeof(buf) - 1] = 0; \
    name(buf, ##__VA_ARGS__); \
  } \
  else{ \
    name((const char *)NULL, ##__VA_ARGS__); \
  }

template< class T>
inline void NtfBase::put_F(const __FlashStringHelper *key, const T &t){
  NTF_FUNC_F(put, key, t);
}

template<class T>
inline void NtfBase::put_F(const __FlashStringHelper *key, const T *t, size_t size){
    NTF_FUNC_F(put, key, t, size);
}

inline void NtfBase::begin_F(const __FlashStringHelper *key){
    NTF_FUNC_F(begin, key);
}

inline void NtfBase::end_F(const __FlashStringHelper *key){
    NTF_FUNC_F(end, key);
}

inline void NtfBase::beginArray_F(const __FlashStringHelper *key){
  NTF_FUNC_F(beginArray, key);
}

inline void NtfBase::endArray_F(const __FlashStringHelper *key){
  NTF_FUNC_F(endArray, key);
}

template <class T>
void NtfBase::put(const char *key, const T &t){
  begin(key);

  putNtfObject(*this, t);

  end(key);
}

template<class T>
void NtfBase::put(const char *key, const T *t, size_t size){
  beginArray(key);

  for(size_t i = 0; i < size; i++){
    put(NULL, t[i]);
  }

  endArray(key);
}

inline void NtfBase::put(const char *key, const __FlashStringHelper *v){
  char buf[24]; 

  if(v){  
    strncpy_P(buf, (const char *)v, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
  }
  else{
    buf[0] = 0;
  }

  put(key, (const char *)buf);
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
  void put(const char *key, const T& t);

  template <class T>
  void put_F(const __FlashStringHelper *key, const T& t);

  template <class T>
  void put(const char * key, const T *t, size_t size);

  template <class T>
  void put_F(const __FlashStringHelper *key, const T* t, size_t size);

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
void NtfBaseSet<N>::put(const char *key, const T& t){

  for(size_t i = 0; i < N  && _ntf[i] != NULL; i++){
    _ntf[i]->reset();
    _ntf[i]->put(key, t);
    _ntf[i]->send();
  }  
}


template<size_t N>
template<class T>
void NtfBaseSet<N>::put_F(const __FlashStringHelper *key, const T& t){
  NTF_FUNC_F(put, key, t);
}


template<size_t N>
template<class T>
void NtfBaseSet<N>::put(const char *key, const T *t, size_t size){

  for(size_t i = 0; i < N  && _ntf[i] != NULL; i++){
    _ntf[i]->reset();
    _ntf[i]->put(key, t, size);
    _ntf[i]->send();
  }  
}


template<size_t N>
template<class T>
void NtfBaseSet<N>::put_F(const __FlashStringHelper *key, const T *t, size_t size){
  NTF_FUNC_F(put, key, t, size);
}

////////////////////////////////
// NtfSerial - notificatoins via serial port

class NtfSerial: public NtfBase {
public:    
  NtfSerial();

  void reset();
  void send();

  void begin(const char *key = NULL);
  void end(const char *key = NULL);
  void beginArray(const char *key = NULL);
  void endArray(const char *key = NULL);

  void put(const char *key, uint8_t v);
  void put(const char *key, uint16_t v);
  void put(const char *key, uint32_t v);
  void put(const char *key, int8_t v);
  void put(const char *key, int16_t v);  
  void put(const char *key, int32_t v);
  void put(const char *key, const char *v);

private:
  void print(const __FlashStringHelper *fmt, ...); 

private:
  uint8_t _ident;    
};





#endif //__NOTIFICATION_H