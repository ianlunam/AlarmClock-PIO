#ifndef HomeAssistant_h
#define HomeAssistant_h
#include <Arduino.h>
#include <Display.h>
#include "Alarm.h"

class HomeAssistant
{
public:
  HomeAssistant();
  void start(Display &indisp, Alarm &inalarm);
};

#endif
