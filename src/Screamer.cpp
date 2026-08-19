#include "Screamer.h"
#include <Arduino.h>

namespace
{
    const note_t MELODY[] = {NOTE_A, NOTE_C, NOTE_A, NOTE_C, NOTE_A, NOTE_C, NOTE_A, NOTE_C};
    const int MELODY_LENGTH = sizeof(MELODY) / sizeof(MELODY[0]);
    const int NOTE_MS = 500;
    const int PAUSE_MS = 1000;
    // How often the beeper task checks for a stop request while holding a
    // note or pausing - this bounds how long stop() takes to notice.
    const int STOP_CHECK_INTERVAL_MS = 50;
}

// Set to request the beeper task stop; cleared again once it has actually
// detached the pin and exited, so stop() knows when it's safe to return.
volatile bool beeperShouldStop = false;
volatile bool beeperRunning = false;

void sound_beeper(void *pvParameters)
{
    beeperRunning = true;
    ledcAttachPin(SPEAKER_PIN, 0);

    for (;;)
    {
        for (int i = 0; i < MELODY_LENGTH && !beeperShouldStop; i++)
        {
            ledcWriteNote(SPEAKER_CHANNEL, MELODY[i], 4);
            for (int waited = 0; waited < NOTE_MS && !beeperShouldStop; waited += STOP_CHECK_INTERVAL_MS)
            {
                vTaskDelay(STOP_CHECK_INTERVAL_MS / portTICK_PERIOD_MS);
            }
        }
        if (beeperShouldStop)
        {
            break;
        }
        for (int waited = 0; waited < PAUSE_MS && !beeperShouldStop; waited += STOP_CHECK_INTERVAL_MS)
        {
            vTaskDelay(STOP_CHECK_INTERVAL_MS / portTICK_PERIOD_MS);
        }
    }

    ledcDetachPin(SPEAKER_PIN);
    beeperRunning = false;
    vTaskDelete(NULL);
}

Screamer::Screamer() {}

void Screamer::start()
{
    beeperShouldStop = false;
    xTaskCreate(sound_beeper, "Alarm!", 4096, NULL, 20, NULL);
}

void Screamer::stop()
{
    beeperShouldStop = true;
    // Wait for the task to notice, detach the pin, and delete itself,
    // instead of killing it wherever it happens to be mid-note.
    while (beeperRunning)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
