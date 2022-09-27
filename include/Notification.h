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
  virtual void put(const char *key, int8_t v) = 0;
  virtual void put(const char *key, int16_t v) = 0;
  virtual void put(const char *key, const char *v) = 0;

  template <class T>
  void put(const char *key, const T &t);

  template<class T>
  void put(const char *key, const T*, size_t size);

  template< class T>
  void put_F (const __FlashStringHelper *key, const T &t);

  template< class T>
  void put_F(const __FlashStringHelper *key, const T*, size_t size);
};

template< class T>
void NtfBase::put_F(const __FlashStringHelper *key, const T &t){
  if(key){
    //Conver key to char *
    char buf[16];
    strncpy_P(buf, (const char *)key, sizeof(buf) - 1);

    //Serialize
    put(buf, t);
  }
  else{
    put((const char *)NULL, t);
  }
  
}

template<class T>
void NtfBase::put_F(const __FlashStringHelper *key, const T *t, size_t size){
  if(key){
    //Conver key to char *
    char buf[16];
    strncpy_P(buf, (const char *)key, sizeof(buf) - 1);

    //Serialize
    put(buf, t, size);
  }
  else{
    put(NULL, t, size);
  }
}

inline void NtfBase::begin_F(const __FlashStringHelper *key){
  if(key){
    //Conver key to char *
    char buf[16];
    strncpy_P(buf, (const char *)key, sizeof(buf) - 1);

    //Serialize
    begin(buf);
  }
  else{
    begin(NULL);
  }
}

inline void NtfBase::end_F(const __FlashStringHelper *key){
  if(key){
    //Conver key to char *
    char buf[16];
    strncpy_P(buf, (const char *)key, sizeof(buf) - 1);

    //Serialize
    end(buf);
  }
  else{
    end(NULL);
  }
}

inline void NtfBase::beginArray_F(const __FlashStringHelper *key){
  if(key){
    //Conver key to char *
    char buf[16];
    strncpy_P(buf, (const char *)key, sizeof(buf) - 1);

    //Serialize
    beginArray(buf);
  }
  else{
    beginArray(NULL);
  }
}

inline void NtfBase::endArray_F(const __FlashStringHelper *key){
 if(key){
    //Conver key to char *
    char buf[16];
    strncpy_P(buf, (const char *)key, sizeof(buf) - 1);

    //Serialize
    endArray(buf);
  }
  else{
    endArray(NULL);
  }
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



///////////////////////////////
// For custom struct to be able to use with NtfBase::put(NtfBase &resp, T), 
// implementation of the following function is required

// void putNtfObject(NtfBase &resp, const T &data){
//   resp.put_F(F("key1"), data.member1);
//   resp.put_F(F("key2"), data.member2);
//   ...
// }


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
  void put(const char *key, int8_t v);
  void put(const char *key, int16_t v);
  void put(const char *key, const char *v);

private:
  void print(const __FlashStringHelper *fmt, ...); 

private:
  uint8_t _ident;    
};

inline NtfSerial::NtfSerial(){
  reset();
}

inline void NtfSerial::reset(){
  _ident = 0;
}

inline void NtfSerial::send(){
}

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
  void put(const T *t, size_t size);

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


#endif //__NOTIFICATION_H