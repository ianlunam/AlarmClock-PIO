#include "Clock.h"
#include <Display.h>
#include <time.h>

struct tm timeinfo2;
Display clockDisplay;

char lastValue[20] = "nope";

void display_time(void *pvParameters)
{
    TFT_eSPI tft = clockDisplay.get_tft();
    for(;;){
        getLocalTime(&timeinfo2);
        char ptr[20];
        int rc = strftime(ptr, 20, "%H:%M", &timeinfo2);

        if (strcmp(lastValue, ptr) != 0){
            Serial.println("Updating time");
            strncpy(lastValue, ptr, 20);
            int x = 5;
            int y = 10;
            tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);

            tft.setFreeFont(&FreeSansBold24pt7b);
            tft.setTextSize(2);
            tft.fillRect(x, y, 240, 100, TFT_BLACK);
            tft.drawString(ptr, x, y);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

}

Clock::Clock()
{
}

void Clock::start(Display &indisp)
{
    clockDisplay = indisp;
    xTaskCreate(display_time, "Display Time", 4096, NULL, 10, NULL);
}
