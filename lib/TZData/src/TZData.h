#ifndef TZData_h
#define TZData_h
#include <Arduino.h>


class TZData {
public:
  TZData();
  void init();
  String getZone(String zone);
private:
  static String tzdata;
};

#endif