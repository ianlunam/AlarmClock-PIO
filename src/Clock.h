#ifndef Clock_h
#define Clock_h
#include <Arduino.h>
#include <Display.h>

class Clock {
  public:
    Clock();
    void start(Display indisp);
};

#endif