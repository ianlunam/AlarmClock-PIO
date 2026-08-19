#include "WeatherIconShape.h"
#include <algorithm>
#include <cctype>

namespace
{
    std::string toLower(const std::string &s)
    {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
                        [](unsigned char c) { return std::tolower(c); });
        return result;
    }
}

WeatherIconShape mapConditionToShape(const std::string &conditionIn)
{
    std::string condition = toLower(conditionIn);

    if (condition == "sunny")
        return SHAPE_SUN;
    if (condition == "clear-night")
        return SHAPE_CLEAR_NIGHT;
    if (condition == "partlycloudy")
        return SHAPE_PARTLY_CLOUDY;
    if (condition == "cloudy")
        return SHAPE_CLOUDY;
    if (condition == "fog")
        return SHAPE_FOG;
    if (condition == "windy" || condition == "windy-variant")
        return SHAPE_WIND;
    if (condition == "rainy")
        return SHAPE_RAIN;
    if (condition == "pouring")
        return SHAPE_POURING;
    if (condition == "lightning")
        return SHAPE_LIGHTNING;
    if (condition == "lightning-rainy")
        return SHAPE_LIGHTNING_RAIN;
    if (condition == "snowy")
        return SHAPE_SNOW;
    if (condition == "snowy-rainy")
        return SHAPE_SNOW_RAIN;
    if (condition == "hail")
        return SHAPE_HAIL;

    return SHAPE_UNKNOWN;
}
