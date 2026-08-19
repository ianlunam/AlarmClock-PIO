#ifndef Alarm_h
#define Alarm_h
#include <Display.h>
#include "Ldr.h"
#include "AlarmEntry.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

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

// True while the alarm is ringing (from the moment the stop button is drawn
// until it's cleared again). Every background task that redraws part of the
// idle screen (clock, weather, temperature, humidity, day/date) must check
// this and skip its redraw while it's true, since the alarm screen is
// allowed to use that same screen space. Set only from Alarm.cpp.
extern volatile bool alarmActive;

class Alarm
{
public:
  Alarm();
  void start(const Display &indisp, const Ldr &ldr);
  void set_public_holiday(bool state);
};

#endif
