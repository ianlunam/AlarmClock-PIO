#ifndef WebServer_h
#define WebServer_h
#include <Arduino.h>
#include <Alarm.h>

class WebServer {
  public:
    WebServer();
    void start(Alarm inalarm);
};

#endif