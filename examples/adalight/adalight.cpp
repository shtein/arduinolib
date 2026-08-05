#include <arduinolib.h>
#include <ambientlight.h>
#include <DbgTool.h>


AmbientLight theLight;
AdaLightReceiver theReceiver;

void setup(){
  
  theReceiver.init(115200);
  theLight.init(&theReceiver, true);
}

void loop(){
  theLight.loop();
}