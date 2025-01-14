#ifndef Display_h
#define Display_h

#include <Arduino.h>
#include <TFT_eSPI.h>

class Display {
  public:
    Display();
    void init();
    TFT_eSPI get_tft();
    void set_backlight(uint32_t level);
};

#endif