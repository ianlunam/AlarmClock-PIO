#include "Screamer.h"
#include <Arduino.h>

TaskHandle_t beeperTaskHandle = NULL;

void sound_beeper(void *pvParameters)
{

  ledcAttachPin(SPEAKER_PIN, 0);
  for (;;)
  {
    ledcAttachPin(SPEAKER_PIN, 0);
    ledcWriteNote(SPEAKER_CHANNEL, NOTE_A, 4);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ledcWriteNote(SPEAKER_CHANNEL, NOTE_C, 4);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ledcWriteNote(SPEAKER_CHANNEL, NOTE_A, 4);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ledcWriteNote(SPEAKER_CHANNEL, NOTE_C, 4);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ledcWriteNote(SPEAKER_CHANNEL, NOTE_A, 4);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ledcWriteNote(SPEAKER_CHANNEL, NOTE_C, 4);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ledcWriteNote(SPEAKER_CHANNEL, NOTE_A, 4);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ledcWriteNote(SPEAKER_CHANNEL, NOTE_C, 4);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ledcDetachPin(SPEAKER_PIN);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

Screamer::Screamer() {}

void Screamer::start()
{
  xTaskCreate(sound_beeper, "Alarm!", 4096, NULL, 20, &beeperTaskHandle);
}

void Screamer::stop()
{
  if (eTaskGetState(beeperTaskHandle) != eDeleted)
  {
    vTaskDelete(beeperTaskHandle);
  }
  ledcDetachPin(SPEAKER_PIN);
}
