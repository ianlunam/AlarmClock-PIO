#include <Arduino.h>
#include "Ldr.h"
#include <Display.h>

Ldr::Ldr() {}

Display ldrDisplay;
bool backlightDown = true;
TaskHandle_t ldrTaskHandle = NULL;

void get_ldr(void *pvParameters)
{
    for (;;)
    {
        int sensorValue = analogRead(LDR_PIN);

        if (sensorValue == 0 && backlightDown)
        {
            backlightDown = false;
            ldrDisplay.set_backlight(BL_MAX);
        }
        else if (sensorValue != 0 && !backlightDown)
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
