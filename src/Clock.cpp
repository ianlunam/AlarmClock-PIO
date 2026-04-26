#include "Clock.h"
#include "Display.h"
#include <time.h>

struct tm clockTimeInfo;
Display clockDisplay;

char lastValue[20] = "nope";
int lastDoW = -1;

void display_time(void *pvParameters)
{
    Serial.println("Clock started");
    TFT_eSPI tft = clockDisplay.get_tft();
    TFT_eSprite spr = TFT_eSprite(&tft);

    spr.setColorDepth(8);

    spr.setFreeFont(&FreeSansBold24pt7b);
    spr.setTextSize(2);

    uint32_t width = 232;
    uint32_t height = 75;

    uint32_t x = (320 / 2) - (width / 2);

    spr.createSprite(width, height);
    spr.fillSprite(BACKGROUND_COLOUR);
    spr.setTextColor((TEXT_R << (5 + 6)) | (TEXT_G << 5) | TEXT_B);
    spr.pushSprite(x, 15);

    for (;;)
    {
        getLocalTime(&clockTimeInfo);
        char ptr[20];
        int rc = strftime(ptr, 20, "%H:%M", &clockTimeInfo);

        if (strcmp(lastValue, ptr) != 0)
        {
            strncpy(lastValue, ptr, 20);
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
    TFT_eSPI tft = clockDisplay.get_tft();
    TFT_eSprite dowSprite = TFT_eSprite(&tft);
    TFT_eSprite dateSprite = TFT_eSprite(&tft);

    dowSprite.setColorDepth(8);
    dowSprite.setFreeFont(&FreeSansBold12pt7b);
    dowSprite.setTextSize(1);
    dateSprite.setColorDepth(8);
    dateSprite.setFreeFont(&FreeSansBold12pt7b);
    dateSprite.setTextSize(1);

    uint32_t width = 80;
    uint32_t height = 25;

    dowSprite.createSprite(width, height);
    dowSprite.fillSprite(BACKGROUND_COLOUR);
    dowSprite.setTextColor((TEXT_R << (5 + 6)) | (TEXT_G << 5) | TEXT_B);
    dowSprite.pushSprite(200, 175);

    dateSprite.createSprite(width, height);
    dateSprite.fillSprite(BACKGROUND_COLOUR);
    dateSprite.setTextColor((TEXT_R << (5 + 6)) | (TEXT_G << 5) | TEXT_B);
    dateSprite.pushSprite(200, 205);

    for (;;)
    {
        getLocalTime(&clockTimeInfo);

        if (lastDoW != clockTimeInfo.tm_wday)
        {
            lastDoW = clockTimeInfo.tm_wday;

            char dowPtr[20];
            strftime(dowPtr, 20, "%a", &clockTimeInfo);
            dowSprite.fillSprite(BACKGROUND_COLOUR);
            uint32_t len = dowSprite.drawString(dowPtr, 0, 0);
            uint32_t start = (width / 2) - (len / 2);
            dowSprite.fillSprite(BACKGROUND_COLOUR);
            dowSprite.drawString(dowPtr, start, 0);
            dowSprite.pushSprite(200, 175);

            char datePtr[20];
            strftime(datePtr, 20, "%d %b", &clockTimeInfo);
            dateSprite.fillSprite(BACKGROUND_COLOUR);
            uint32_t dateLen = dateSprite.drawString(datePtr, 0, 0);
            uint32_t dateStart = (width / 2) - (dateLen / 2);
            dateSprite.fillSprite(BACKGROUND_COLOUR);
            dateSprite.drawString(datePtr, dateStart, 0);
            dateSprite.pushSprite(200, 205);
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
