#include "Ldr.h"
#include <Display.h>

Ldr::Ldr() {}

Display ldrDisplay;
bool backlightDown = true;
TaskHandle_t ldrTaskHandle = NULL;

void get_ldr(void *pvParameters)
{
    int darkCount = 0;
    int lightCount = 0;

    for (;;)
    {
        int sensorValue = analogRead(LDR_PIN);

        // Hysteresis: "dark" and "light" use different thresholds with a dead
        // zone between them, and a reading must persist for several polls
        // before it's acted on. Without this, a value sitting right on the
        // boundary flickers the backlight between min and max every 100ms.
        if (sensorValue <= LDR_DARK_THRESHOLD)
        {
            darkCount++;
            lightCount = 0;
        }
        else if (sensorValue >= LDR_LIGHT_THRESHOLD)
        {
            lightCount++;
            darkCount = 0;
        }
        else
        {
            darkCount = 0;
            lightCount = 0;
        }

        if (darkCount >= LDR_DEBOUNCE_COUNT && backlightDown)
        {
            backlightDown = false;
            ldrDisplay.set_backlight(BL_MAX);
        }
        else if (lightCount >= LDR_DEBOUNCE_COUNT && !backlightDown)
        {
            backlightDown = true;
            ldrDisplay.set_backlight(BL_MIN);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
void Ldr::start(Display &display)
{
    Serial.println("LDR started");
    ldrDisplay = display;
    analogSetAttenuation(ADC_0db);
    pinMode(LDR_PIN, INPUT);
    xTaskCreate(get_ldr, "Display LDR", 4096, NULL, 10, &ldrTaskHandle);
}

void Ldr::stop()
{
    vTaskDelete(ldrTaskHandle);
}
