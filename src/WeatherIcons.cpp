#include <Arduino.h>
#include "WeatherIcons.h"
#include "WeatherIconShape.h"
#include "Display.h"

// All icons are drawn on a fixed 44x44 logical grid (icon-local coordinates,
// origin top-left) which is then centred inside whatever sprite is passed
// in. To move/resize an icon, adjust its coordinates below - they don't
// depend on anything else.
namespace
{
    const int ICON_SIZE = 44;

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
            int x1 = ox + cx + (int)((r + 4) * cos(angle));
            int y1 = oy + cy + (int)((r + 4) * sin(angle));
            int x2 = ox + cx + (int)((r + 10) * cos(angle));
            int y2 = oy + cy + (int)((r + 10) * sin(angle));
            s->drawLine(x1, y1, x2, y2, SUN_COLOUR);
        }
    }

    void drawMoon(TFT_eSprite *s, int ox, int oy)
    {
        s->fillCircle(ox + 22, oy + 22, 16, MOON_COLOUR);
        s->fillCircle(ox + 30, oy + 16, 14, BACKGROUND_COLOUR);
    }

    void drawCloud(TFT_eSprite *s, int ox, int oy, int cy, uint16_t colour)
    {
        s->fillCircle(ox + 12, oy + cy, 8, colour);
        s->fillCircle(ox + 22, oy + cy - 6, 10, colour);
        s->fillCircle(ox + 32, oy + cy, 8, colour);
        s->fillRect(ox + 4, oy + cy, 36, 10, colour);
    }

    void drawRainDrops(TFT_eSprite *s, int ox, int oy, int y, int count)
    {
        int startX = ox + 12;
        for (int i = 0; i < count; i++)
        {
            int x = startX + i * 10;
            s->drawLine(x, oy + y, x - 4, oy + y + 10, RAIN_COLOUR);
        }
    }

    void drawSnowFlakes(TFT_eSprite *s, int ox, int oy, int y, int count)
    {
        int startX = ox + 12;
        for (int i = 0; i < count; i++)
        {
            int x = startX + i * 10;
            s->drawLine(x - 4, oy + y, x + 4, oy + y, SNOW_COLOUR);
            s->drawLine(x, oy + y - 4, x, oy + y + 4, SNOW_COLOUR);
        }
    }

    void drawHailStones(TFT_eSprite *s, int ox, int oy, int y, int count)
    {
        int startX = ox + 12;
        for (int i = 0; i < count; i++)
        {
            s->fillCircle(startX + i * 10, oy + y, 4, CLOUD_COLOUR);
        }
    }

    void drawBolt(TFT_eSprite *s, int ox, int oy)
    {
        s->fillTriangle(ox + 24, oy + 18, ox + 16, oy + 32, ox + 26, oy + 28, BOLT_COLOUR);
        s->fillTriangle(ox + 26, oy + 28, ox + 18, oy + 42, ox + 30, oy + 24, BOLT_COLOUR);
    }

    void drawFogLines(TFT_eSprite *s, int ox, int oy)
    {
        for (int i = 0; i < 4; i++)
        {
            s->drawFastHLine(ox + 4, oy + 8 + i * 10, 36, CLOUD_COLOUR);
        }
    }

    void drawWindLines(TFT_eSprite *s, int ox, int oy)
    {
        for (int i = 0; i < 3; i++)
        {
            int y = oy + 10 + i * 12;
            int len = 36 - i * 6;
            s->drawFastHLine(ox + 4, y, len, CLOUD_COLOUR);
            s->drawLine(ox + 4 + len - 2, y, ox + 4 + len - 8, y - 4, CLOUD_COLOUR);
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
    DisplayLock lock;
    sprite->fillSprite(BACKGROUND_COLOUR);

    WeatherIconShape shape = mapConditionToShape(conditionIn.c_str());

    int ox = ((int)sprite->width() - ICON_SIZE) / 2;
    int oy = ((int)sprite->height() - ICON_SIZE) / 2;
    if (ox < 0)
        ox = 0;
    if (oy < 0)
        oy = 0;

    switch (shape)
    {
    case SHAPE_SUN:
        drawSun(sprite, ox, oy, 22, 22, 12);
        break;
    case SHAPE_CLEAR_NIGHT:
        drawMoon(sprite, ox, oy);
        break;
    case SHAPE_PARTLY_CLOUDY:
        drawSun(sprite, ox, oy, 14, 14, 8);
        drawCloud(sprite, ox, oy, 28, CLOUD_COLOUR);
        break;
    case SHAPE_CLOUDY:
        drawCloud(sprite, ox, oy, 22, CLOUD_COLOUR);
        break;
    case SHAPE_FOG:
        drawFogLines(sprite, ox, oy);
        break;
    case SHAPE_WIND:
        drawWindLines(sprite, ox, oy);
        break;
    case SHAPE_RAIN:
        drawCloud(sprite, ox, oy, 16, CLOUD_COLOUR);
        drawRainDrops(sprite, ox, oy, 32, 3);
        break;
    case SHAPE_POURING:
        drawCloud(sprite, ox, oy, 16, DARK_CLOUD_COLOUR);
        drawRainDrops(sprite, ox, oy, 32, 4);
        break;
    case SHAPE_LIGHTNING:
        drawCloud(sprite, ox, oy, 12, DARK_CLOUD_COLOUR);
        drawBolt(sprite, ox, oy);
        break;
    case SHAPE_LIGHTNING_RAIN:
        drawCloud(sprite, ox, oy, 12, DARK_CLOUD_COLOUR);
        drawBolt(sprite, ox, oy);
        drawRainDrops(sprite, ox, oy, 36, 2);
        break;
    case SHAPE_SNOW:
        drawCloud(sprite, ox, oy, 16, CLOUD_COLOUR);
        drawSnowFlakes(sprite, ox, oy, 34, 3);
        break;
    case SHAPE_SNOW_RAIN:
        drawCloud(sprite, ox, oy, 16, CLOUD_COLOUR);
        drawRainDrops(sprite, ox, oy, 32, 2);
        drawSnowFlakes(sprite, ox, oy, 34, 2);
        break;
    case SHAPE_HAIL:
        drawCloud(sprite, ox, oy, 16, CLOUD_COLOUR);
        drawHailStones(sprite, ox, oy, 34, 3);
        break;
    case SHAPE_UNKNOWN:
    default:
        // Unknown/unmapped condition (e.g. "exceptional") - show the text
        // instead of nothing, so it's never a blank space.
        drawFallbackText(sprite, conditionIn);
        sprite->pushSprite(x, y);
        return;
    }

    sprite->pushSprite(x, y);
}
