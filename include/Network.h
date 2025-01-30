#ifndef Network_h
#define Network_h
#include <Arduino.h>
#include "Display.h"
#include "TFT_eSPI.h"

class Network
{
public:
  Network();
  void start(Display &display);
};

#endif
