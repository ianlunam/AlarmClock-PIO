#ifndef Display_h
#define Display_h

#include <TFT_eSPI.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class Display
{
public:
  Display();
  void start();
  TFT_eSPI &get_tft();
  void set_backlight(uint32_t level);
};

// The single global TFT_eSPI object (and its SPI bus) is shared by every
// task that draws to the screen (clock, weather/MQTT data, alarm button,
// WiFi status icon). TFT_eSPI has no internal locking of its own, so two
// tasks drawing at the same instant can corrupt each other's SPI
// transaction and crash the device unpredictably. Every draw-and-push
// sequence must hold this lock for its duration.
extern SemaphoreHandle_t displayMutex;

class DisplayLock
{
public:
  DisplayLock() { xSemaphoreTake(displayMutex, portMAX_DELAY); }
  ~DisplayLock() { xSemaphoreGive(displayMutex); }
};

#endif
