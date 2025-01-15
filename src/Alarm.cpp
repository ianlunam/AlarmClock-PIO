#include "Display.h"
#include <time.h>
#include "Alarm.h"

#include <SPI.h>
#include <XPT2046_Bitbang.h>
#include <TFT_eSPI.h>
#include <Preferences.h>

struct tm timeinfo3;
Display alarmDisplay;

XPT2046_Bitbang ts(XPT2046_MOSI, XPT2046_MISO, XPT2046_CLK, XPT2046_CS);

TFT_eSPI_Button stop_button;
TFT_eSPI_Button snooze_button;

Alarm::Alarm(){}

TFT_eSPI_Button drawButton(String text, uint32_t x, uint32_t y, uint32_t width = 100, uint32_t height = 50) {

    TFT_eSPI tft = alarmDisplay.get_tft();
    TFT_eSPI_Button button;

    button.initButton(&tft,
                    x,
                    y,
                    width,
                    height,
                    TFT_DARKCYAN, // Outline
                    TFT_BLUE, // Fill
                    TFT_DARKCYAN, // Text
                    "",
                    2);

    button.drawButton(false, text);
    return button;
}

void alarm_clock(void *pvParameters)
{
    TouchPoint p = ts.getTouch();
    for(;;) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        if (false) {
            snooze_button = drawButton("Snooze", 300, 60);
            stop_button = drawButton("Stop", 300, 120);
        }
    }
}

void Alarm::start(Display &indisp)
{
    alarmDisplay = indisp;
    xTaskCreate(alarm_clock, "Alarm Clock", 4096, NULL, 10, NULL);
}
