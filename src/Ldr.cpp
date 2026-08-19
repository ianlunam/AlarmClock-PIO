#include "Ldr.h"
#include "LdrHysteresis.h"
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

        LdrDecision decision = ldrUpdate(sensorValue, backlightDown, darkCount, lightCount,
                                          LDR_DARK_THRESHOLD, LDR_LIGHT_THRESHOLD, LDR_DEBOUNCE_COUNT);

        if (decision == LDR_GO_BRIGHT)
        {
            backlightDown = false;
            ldrDisplay.set_backlight(BL_MAX);
        }
        else if (decision == LDR_GO_DIM)
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
