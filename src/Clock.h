#ifndef Clock_h
#define Clock_h
#include <Arduino.h>

class Clock {
public:
  Clock();
  void init();
  void update();
  void connect(const char* ssid, const char* wifipw);
  void setTime();
  void setTime(int hh, int mm);
  String getTimeString();
  String getDateString();
  String getYearString();
  int getHour();
  int getMinute();
};

#endif
