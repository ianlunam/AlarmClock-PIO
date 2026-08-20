#include "Clock.h"
#include "Display.h"
#include "Alarm.h"
#include <time.h>

struct tm clockTimeInfo;
Display clockDisplay;

char lastValue[20] = "nope";
int lastDoW = -1;

void display_time(void *pvParameters)
{
    Serial.println("Clock started");
    TFT_eSPI &tft = clockDisplay.get_tft();
    TFT_eSprite spr = TFT_eSprite(&tft);

    spr.setColorDepth(8);

    spr.setFreeFont(&FreeSansBold24pt7b);
    spr.setTextSize(2);

    uint32_t width = 232;
    uint32_t height = 75;

    uint32_t x = (320 / 2) - (width / 2);

    {
        DisplayLock lock;
        spr.createSprite(width, height);
        spr.fillSprite(BACKGROUND_COLOUR);
        spr.setTextColor((TEXT_R << (5 + 6)) | (TEXT_G << 5) | TEXT_B);
        spr.pushSprite(x, 15);
    }

    bool forceRedraw = false;
    for (;;)
    {
        if (alarmActive)
        {
            // The alarm screen owns the display right now - don't draw over
            // it. Redraw unconditionally once it's done.
            forceRedraw = true;
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        getLocalTime(&clockTimeInfo);
        char ptr[20];
        int rc = strftime(ptr, 20, "%H:%M", &clockTimeInfo);

        if (strcmp(lastValue, ptr) != 0 || forceRedraw)
        {
            forceRedraw = false;
            strncpy(lastValue, ptr, 20);
            DisplayLock lock;
            spr.fillSprite(BACKGROUND_COLOUR);
            uint32_t len = spr.drawString(ptr, 0, 0);
            uint32_t start = (width / 2) - (len / 2);
            spr.fillSprite(BACKGROUND_COLOUR);
            spr.drawString(ptr, start, 0);
            spr.pushSprite(x, 15);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void display_dow(void *pvParameters)
{
    Serial.println("Clock started");
    TFT_eSPI &tft = clockDisplay.get_tft();
    TFT_eSprite dowSprite = TFT_eSprite(&tft);
    TFT_eSprite dateSprite = TFT_eSprite(&tft);

    dowSprite.setColorDepth(8);
    dowSprite.setFreeFont(&FreeSansBold12pt7b);
    dowSprite.setTextSize(1);
    dateSprite.setColorDepth(8);
    dateSprite.setFreeFont(&FreeSansBold12pt7b);
    dateSprite.setTextSize(1);

    uint32_t width = 110;
    uint32_t height = 30;
    uint32_t dowX = 100;
    uint32_t dowY = 98;
    uint32_t dateX = 100;
    uint32_t dateY = 132;

    {
        DisplayLock lock;
        dowSprite.createSprite(width, height);
        dowSprite.fillSprite(BACKGROUND_COLOUR);
        dowSprite.setTextColor((TEXT_R << (5 + 6)) | (TEXT_G << 5) | TEXT_B);
        dowSprite.pushSprite(dowX, dowY);

        dateSprite.createSprite(width, height);
        dateSprite.fillSprite(BACKGROUND_COLOUR);
        dateSprite.setTextColor((TEXT_R << (5 + 6)) | (TEXT_G << 5) | TEXT_B);
        dateSprite.pushSprite(dateX, dateY);
    }

    bool forceRedraw = false;
    for (;;)
    {
        if (alarmActive)
        {
            forceRedraw = true;
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        getLocalTime(&clockTimeInfo);

        if (lastDoW != clockTimeInfo.tm_wday || forceRedraw)
        {
            forceRedraw = false;
            lastDoW = clockTimeInfo.tm_wday;

            DisplayLock lock;
            char dowPtr[20];
            strftime(dowPtr, 20, "%a", &clockTimeInfo);
            dowSprite.fillSprite(BACKGROUND_COLOUR);
            uint32_t len = dowSprite.drawString(dowPtr, 0, 0);
            uint32_t start = (width / 2) - (len / 2);
            dowSprite.fillSprite(BACKGROUND_COLOUR);
            dowSprite.drawString(dowPtr, start, 0);
            dowSprite.pushSprite(dowX, dowY);

            char datePtr[20];
            strftime(datePtr, 20, "%d %b", &clockTimeInfo);
            dateSprite.fillSprite(BACKGROUND_COLOUR);
            uint32_t dateLen = dateSprite.drawString(datePtr, 0, 0);
            uint32_t dateStart = (width / 2) - (dateLen / 2);
            dateSprite.fillSprite(BACKGROUND_COLOUR);
            dateSprite.drawString(datePtr, dateStart, 0);
            dateSprite.pushSprite(dateX, dateY);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

Clock::Clock() {}

void Clock::start(Display &indisp)
{
    clockDisplay = indisp;
    xTaskCreate(display_time, "Display Time", 4096, NULL, 10, NULL);
    xTaskCreate(display_dow, "Display Day of Week", 4096, NULL, 10, NULL);
}
