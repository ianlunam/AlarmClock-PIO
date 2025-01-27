#include "Clock.h"
#include <Display.h>
#include <time.h>

struct tm clockTimeInfo;
Display clockDisplay;

char lastValue[20] = "nope";

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

    uint32_t x = (320/2) - (width/2);

    spr.createSprite(width, height);
    spr.fillSprite(BACKGROUND_COLOUR);
    spr.setTextColor((TEXT_R<<(5+6))|(TEXT_G<<5)|TEXT_B);
    spr.pushSprite(x, 10);

    for(;;){
        getLocalTime(&clockTimeInfo);
        char ptr[20];
        int rc = strftime(ptr, 20, "%H:%M", &clockTimeInfo);

        if (strcmp(lastValue, ptr) != 0){
            strncpy(lastValue, ptr, 20);
            spr.fillSprite(BACKGROUND_COLOUR);
            uint32_t len = spr.drawString(ptr, 0, 0);
            uint32_t start = (width/2) - (len/2);
            spr.fillSprite(BACKGROUND_COLOUR);
            spr.drawString(ptr, start, 0);
            spr.pushSprite(x, 10);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

}

Clock::Clock(){}

void Clock::start(Display &indisp)
{
    clockDisplay = indisp;
    xTaskCreate(display_time, "Display Time", 4096, NULL, 10, NULL);
}
