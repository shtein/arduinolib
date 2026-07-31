#include <arduinolib.h>
#include <adalight.h>


AdaLight adalight;

void setup(){
  adalight.init(115200);
}

void loop(){
  adalight.loop();
}