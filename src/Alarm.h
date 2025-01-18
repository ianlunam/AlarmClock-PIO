#ifndef Alarm_h
#define Alarm_h
#include <Arduino.h>
#include <Display.h>
#include "Ldr.h"

struct AlarmEntry {
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

class Alarm {
  public:
    Alarm();
    void start(Display &indisp, Ldr &ldr, bool &holiday);
    void restart();
    bool getAlarm(char *name, AlarmEntry &newAlarm);
    bool alarming(bool isPhol);
    bool alarmTriggerNow(bool isPhol);
    void turnOff();
    void turnOn();
    void snooze();
    String toString(AlarmEntry &thisAlarm);
    bool isSnoozed();
    bool isOn();
    void set_public_holiday(bool state);
};

#endif