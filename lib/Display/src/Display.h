#ifndef Display_h
#define Display_h
#include <Arduino.h>

class Display {
public:
  Display();
  void init();
  void updateTime();
  void setString(int zone, String value);
};

#endif
