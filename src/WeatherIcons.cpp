#include <Arduino.h>
#include "WeatherIcons.h"

// All icons are drawn on a fixed 22x22 logical grid (icon-local coordinates,
// origin top-left) which is then centred inside whatever sprite is passed
// in. To move/resize an icon, adjust its coordinates below - they don't
// depend on anything else.
namespace
{
    const int ICON_SIZE = 22;

    const uint16_t SUN_COLOUR = TFT_YELLOW;
    const uint16_t CLOUD_COLOUR = TFT_LIGHTGREY;
    const uint16_t DARK_CLOUD_COLOUR = TFT_DARKGREY;
    const uint16_t RAIN_COLOUR = TFT_SKYBLUE;
    const uint16_t SNOW_COLOUR = TFT_WHITE;
    const uint16_t BOLT_COLOUR = TFT_YELLOW;
    const uint16_t MOON_COLOUR = TFT_LIGHTGREY;

    void drawSun(TFT_eSprite *s, int ox, int oy, int cx, int cy, int r)
    {
        s->fillCircle(ox + cx, oy + cy, r, SUN_COLOUR);
        for (int i = 0; i < 8; i++)
        {
            float angle = i * PI / 4.0;
            int x1 = ox + cx + (int)((r + 2) * cos(angle));
            int y1 = oy + cy + (int)((r + 2) * sin(angle));
            int x2 = ox + cx + (int)((r + 5) * cos(angle));
            int y2 = oy + cy + (int)((r + 5) * sin(angle));
            s->drawLine(x1, y1, x2, y2, SUN_COLOUR);
        }
    }

    void drawMoon(TFT_eSprite *s, int ox, int oy)
    {
        s->fillCircle(ox + 11, oy + 11, 8, MOON_COLOUR);
        s->fillCircle(ox + 15, oy + 8, 7, BACKGROUND_COLOUR);
    }

    void drawCloud(TFT_eSprite *s, int ox, int oy, int cy, uint16_t colour)
    {
        s->fillCircle(ox + 6, oy + cy, 4, colour);
        s->fillCircle(ox + 11, oy + cy - 3, 5, colour);
        s->fillCircle(ox + 16, oy + cy, 4, colour);
        s->fillRect(ox + 2, oy + cy, 18, 5, colour);
    }

    void drawRainDrops(TFT_eSprite *s, int ox, int oy, int y, int count)
    {
        int startX = ox + 6;
        for (int i = 0; i < count; i++)
        {
            int x = startX + i * 5;
            s->drawLine(x, oy + y, x - 2, oy + y + 5, RAIN_COLOUR);
        }
    }

    void drawSnowFlakes(TFT_eSprite *s, int ox, int oy, int y, int count)
    {
        int startX = ox + 6;
        for (int i = 0; i < count; i++)
        {
            int x = startX + i * 5;
            s->drawLine(x - 2, oy + y, x + 2, oy + y, SNOW_COLOUR);
            s->drawLine(x, oy + y - 2, x, oy + y + 2, SNOW_COLOUR);
        }
    }

    void drawHailStones(TFT_eSprite *s, int ox, int oy, int y, int count)
    {
        int startX = ox + 6;
        for (int i = 0; i < count; i++)
        {
            s->fillCircle(startX + i * 5, oy + y, 2, CLOUD_COLOUR);
        }
    }

    void drawBolt(TFT_eSprite *s, int ox, int oy)
    {
        s->fillTriangle(ox + 12, oy + 9, ox + 8, oy + 16, ox + 13, oy + 14, BOLT_COLOUR);
        s->fillTriangle(ox + 13, oy + 14, ox + 9, oy + 21, ox + 15, oy + 12, BOLT_COLOUR);
    }

    void drawFogLines(TFT_eSprite *s, int ox, int oy)
    {
        for (int i = 0; i < 4; i++)
        {
            s->drawFastHLine(ox + 2, oy + 4 + i * 5, 18, CLOUD_COLOUR);
        }
    }

    void drawWindLines(TFT_eSprite *s, int ox, int oy)
    {
        for (int i = 0; i < 3; i++)
        {
            int y = oy + 5 + i * 6;
            int len = 18 - i * 3;
            s->drawFastHLine(ox + 2, y, len, CLOUD_COLOUR);
            s->drawLine(ox + 2 + len - 1, y, ox + 2 + len - 4, y - 2, CLOUD_COLOUR);
        }
    }

    void drawFallbackText(TFT_eSprite *sprite, const String &conditionIn)
    {
        String label = conditionIn;
        if (label.length() > 0 && isalpha(label[0]))
        {
            label[0] = toupper(label[0]);
        }
        sprite->setTextColor((TEXT_R << (5 + 6)) | (TEXT_G << 5) | TEXT_B);
        uint32_t len = sprite->drawString(label, 0, 0);
        sprite->fillSprite(BACKGROUND_COLOUR);
        int startX = ((int)sprite->width() - (int)len) / 2;
        sprite->drawString(label, startX < 0 ? 0 : startX, 0);
    }
}

void updateWeatherIcon(TFT_eSprite *sprite, const String &conditionIn, uint16_t x, uint16_t y)
{
    sprite->fillSprite(BACKGROUND_COLOUR);

    String condition = conditionIn;
    condition.toLowerCase();

    int ox = ((int)sprite->width() - ICON_SIZE) / 2;
    int oy = ((int)sprite->height() - ICON_SIZE) / 2;
    if (ox < 0)
        ox = 0;
    if (oy < 0)
        oy = 0;

    if (condition == "sunny")
    {
        drawSun(sprite, ox, oy, 11, 11, 6);
    }
    else if (condition == "clear-night")
    {
        drawMoon(sprite, ox, oy);
    }
    else if (condition == "partlycloudy")
    {
        drawSun(sprite, ox, oy, 7, 7, 4);
        drawCloud(sprite, ox, oy, 14, CLOUD_COLOUR);
    }
    else if (condition == "cloudy")
    {
        drawCloud(sprite, ox, oy, 11, CLOUD_COLOUR);
    }
    else if (condition == "fog")
    {
        drawFogLines(sprite, ox, oy);
    }
    else if (condition == "windy" || condition == "windy-variant")
    {
        drawWindLines(sprite, ox, oy);
    }
    else if (condition == "rainy")
    {
        drawCloud(sprite, ox, oy, 8, CLOUD_COLOUR);
        drawRainDrops(sprite, ox, oy, 16, 3);
    }
    else if (condition == "pouring")
    {
        drawCloud(sprite, ox, oy, 8, DARK_CLOUD_COLOUR);
        drawRainDrops(sprite, ox, oy, 16, 4);
    }
    else if (condition == "lightning")
    {
        drawCloud(sprite, ox, oy, 6, DARK_CLOUD_COLOUR);
        drawBolt(sprite, ox, oy);
    }
    else if (condition == "lightning-rainy")
    {
        drawCloud(sprite, ox, oy, 6, DARK_CLOUD_COLOUR);
        drawBolt(sprite, ox, oy);
        drawRainDrops(sprite, ox, oy, 18, 2);
    }
    else if (condition == "snowy")
    {
        drawCloud(sprite, ox, oy, 8, CLOUD_COLOUR);
        drawSnowFlakes(sprite, ox, oy, 17, 3);
    }
    else if (condition == "snowy-rainy")
    {
        drawCloud(sprite, ox, oy, 8, CLOUD_COLOUR);
        drawRainDrops(sprite, ox, oy, 16, 2);
        drawSnowFlakes(sprite, ox, oy, 17, 2);
    }
    else if (condition == "hail")
    {
        drawCloud(sprite, ox, oy, 8, CLOUD_COLOUR);
        drawHailStones(sprite, ox, oy, 17, 3);
    }
    else
    {
        // Unknown/unmapped condition (e.g. "exceptional") - show the text
        // instead of nothing, so it's never a blank space.
        drawFallbackText(sprite, conditionIn);
        sprite->pushSprite(x, y);
        return;
    }

    sprite->pushSprite(x, y);
}
