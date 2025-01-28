#ifndef Alarm_h
#define Alarm_h
#include <Arduino.h>
#include <Display.h>
#include "Ldr.h"

struct AlarmEntry
{
  char name[20];
  int hour;
  int minute;
  bool sunday;
  bool monday;
  bool tuesday;
  bool wednesday;
  bool thursday;
  bool friday;
  bool saturday;
  bool skip_phols;
  bool once;
  bool enabled;
};

class Alarm
{
public:
  Alarm();
  void start(Display &indisp, Ldr &ldr);
  void set_public_holiday(bool state);
};

#endif
