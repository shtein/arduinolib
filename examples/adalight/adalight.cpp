#include <arduinolib.h>
#include <adalight.h>


AdaLight theAdaLight;

void setup(){
  theAdaLight.init(true, 115200);
}

void loop(){
  theAdaLight.loop();
}