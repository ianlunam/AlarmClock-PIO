#ifndef HomeAssistant_h
#define HomeAssistant_h
#include <Arduino.h>
#include <Display.h>

class HomeAssistant {
  public:
    HomeAssistant();
    void start(Display &indisp);
};

#endif