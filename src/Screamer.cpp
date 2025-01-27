#include "Screamer.h"
#include "Pitches.h"
#include <Arduino.h>

int melody[] = {
  // NOTE_C6, NOTE_A5, NOTE_C6, NOTE_A5, NOTE_C6, NOTE_A5, NOTE_C6, NOTE_A5
  NOTE_C5, NOTE_A4, NOTE_C5, NOTE_A4, NOTE_C5, NOTE_A4, NOTE_C5, NOTE_A4
};

int noteDurations[] = {
  4, 4, 4, 4, 4, 4, 4, 4
};

TaskHandle_t beeperTaskHandle = NULL;

void sound_beeper(void *pvParameters) {

  for (;;) {
    for (int thisNote = 0; thisNote < 8; thisNote++) {
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(SPEAKER_PIN, melody[thisNote], noteDuration);

      int pauseBetweenNotes = noteDuration * 1.30;
      vTaskDelay(pauseBetweenNotes / portTICK_PERIOD_MS);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}


Screamer::Screamer(){}

void Screamer::start()
{
    Serial.println("Staring screamer");
    xTaskCreate(sound_beeper, "Alarm!", 4096, NULL, 20, &beeperTaskHandle);
}

void Screamer::stop()
{
    Serial.println("Stopping screamer");
    if (eTaskGetState(beeperTaskHandle) != eDeleted) {
      vTaskDelete(beeperTaskHandle);
    }
    noTone(SPEAKER_PIN);
}
