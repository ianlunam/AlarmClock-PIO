#ifndef WeatherIconShape_h
#define WeatherIconShape_h

#include <string>

enum WeatherIconShape
{
    SHAPE_SUN,
    SHAPE_CLEAR_NIGHT,
    SHAPE_PARTLY_CLOUDY,
    SHAPE_CLOUDY,
    SHAPE_FOG,
    SHAPE_WIND,
    SHAPE_RAIN,
    SHAPE_POURING,
    SHAPE_LIGHTNING,
    SHAPE_LIGHTNING_RAIN,
    SHAPE_SNOW,
    SHAPE_SNOW_RAIN,
    SHAPE_HAIL,
    SHAPE_UNKNOWN // condition not recognised - caller should show fallback text
};

// Maps a Home Assistant weather condition string (case-insensitive) to the
// icon shape to draw. Pure string matching, no drawing - kept separate so
// it can be unit tested without a display (see test/test_weather_icon_shape).
WeatherIconShape mapConditionToShape(const std::string &condition);

#endif
