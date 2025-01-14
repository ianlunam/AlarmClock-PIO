#ifndef Weather_h
#define Weather_h
#include <Arduino.h>
#include <Display.h>

class Weather {
  public:
    Weather();
    void start(Display indisp);
};

#endif