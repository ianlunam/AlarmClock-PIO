#ifndef Alarm_h
#define Alarm_h
#include <Display.h>
#include "Ldr.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

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

// The "alarmStore" Preferences (NVS) namespace is read and written from both
// the web server task and the alarm-checking task. This guards it so the two
// can never touch it at the same instant. It's a recursive mutex because
// some of the alarm-reading functions call each other while already holding
// the lock.
extern SemaphoreHandle_t alarmStoreMutex;

class AlarmStoreLock
{
public:
  AlarmStoreLock() { xSemaphoreTakeRecursive(alarmStoreMutex, portMAX_DELAY); }
  ~AlarmStoreLock() { xSemaphoreGiveRecursive(alarmStoreMutex); }
};

class Alarm
{
public:
  Alarm();
  void start(const Display &indisp, const Ldr &ldr);
  void set_public_holiday(bool state);
};

#endif
