#ifndef HomeAssistant_h
#define HomeAssistant_h
#include "Alarm.h"

class HomeAssistant
{
public:
  HomeAssistant();
  void start(const Display &indisp, const Alarm &inalarm);
};

#endif
