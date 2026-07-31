#include <arduinolib.h>
#include <adalight.h>




void setup(){
  theAdaLight.init(true, 115200);
}

void loop(){
  theAdaLight.loop();
}