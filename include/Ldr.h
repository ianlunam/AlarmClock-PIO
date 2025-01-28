#ifndef Ldr_h
#define Ldr_h
#include <Arduino.h>
#include <Display.h>

class Ldr
{
public:
  Ldr();
  void start(Display &display);
  void stop();
};

#endif
