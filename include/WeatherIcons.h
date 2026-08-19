#ifndef WeatherIcons_h
#define WeatherIcons_h

#include <TFT_eSPI.h>

// Clears the sprite, draws an icon for the given Home Assistant weather
// condition string (e.g. "sunny", "partlycloudy", "rainy"), and pushes it
// to the screen at (x, y). Falls back to showing the raw text, centred,
// for any condition string it doesn't recognise.
void updateWeatherIcon(TFT_eSprite *sprite, const String &condition, uint16_t x, uint16_t y);

#endif
