#include <lvgl.h>
#include <TFT_eSPI.h>

#include "Display.h"

TFT_eSPI tft = TFT_eSPI();

#define LEDC_CHANNEL_0 0
#define LEDC_TIMER_12_BIT 12
#define LEDC_BASE_FREQ 5000

Display::Display() {}

void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255)
{
    // calculate duty, 4095 from 2 ^ 12 - 1
    uint32_t duty = (4095 / valueMax) * min(value, valueMax);

    // write duty to LEDC
    ledcWrite(channel, duty);
}

void Display::start()
{
    Serial.println("Starting display");
    // Start the tft display and set it to black
    tft.init();
    tft.setRotation(3); // This is the display in landscape

#if ESP_IDF_VERSION_MAJOR == 5
    ledcAttach(TFT_BL, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
#else
    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcAttachPin(TFT_BL, LEDC_CHANNEL_0);
#endif
    ledcAnalogWrite(LEDC_CHANNEL_0, 0);

    // Clear the screen before writing to it
    tft.fillScreen(BACKGROUND_COLOUR);
}

TFT_eSPI &Display::get_tft()
{
    return tft;
}

void Display::set_backlight(uint32_t level)
{
    if (level > BL_MAX)
    {
        level = BL_MAX;
    }
    if (level < BL_MIN)
    {
        level = BL_MIN;
    }
    ledcAnalogWrite(LEDC_CHANNEL_0, level);
}
