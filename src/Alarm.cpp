#include <time.h>
#include "Alarm.h"
#include "Screamer.h"

#include <SPI.h>
#include <XPT2046_Bitbang.h>
#include <TFT_eSPI.h>
#include <Preferences.h>

struct tm timeinfo3;
Display alarmDisplay;
Ldr alarmLdr;
Screamer screamer = Screamer();
bool alarmHoliday = false;

XPT2046_Bitbang ts(XPT2046_MOSI, XPT2046_MISO, XPT2046_CLK, XPT2046_CS);

TFT_eSPI_Button stop_button;
TFT_eSPI_Button snooze_button;

#define ALARM_OFF 0
#define ALARM_ON 1
#define ALARM_SNOOZE 2

int alarmState = ALARM_OFF;

char alarmList[][20] = { "", "", "", "", "", "" };
time_t alarmedLast[6];
time_t snoozeTime = 0;
time_t lastAlarmCheck = 0;
time_t alarmStarted = 0;

TaskHandle_t alarmTaskHandle = NULL;

Alarm::Alarm(){}

TFT_eSPI_Button drawButton(String text, uint32_t x, uint32_t y, uint32_t width = 100, uint32_t height = 50) {

    TFT_eSPI tft = alarmDisplay.get_tft();
    TFT_eSPI_Button button;

    char a[] = "";
    button.initButton(&tft, x, y, width, height, TFT_DARKCYAN, TFT_BLUE, TFT_DARKCYAN, a, 2);

    button.drawButton(false, text);
    return button;
}

void getAlarmList() {
  Preferences preferences;

  preferences.begin("alarmStore", false);
  if (preferences.isKey("alarms")) {
    int size = preferences.getBytesLength("alarms");
    if (size > 0) {
      char* buf[size + 1];
      int result = preferences.getBytes("alarms", &buf, size);
      memcpy(&alarmList, buf, size);
      preferences.end();
      return;
    }
  }
  for (int x = 0; x < 6; x++) {
    strcpy(alarmList[x], "");
  }
  preferences.putBytes("alarms", &alarmList, sizeof(alarmList));
  preferences.end();
  return;
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
        TouchPoint tp = ts.getTouch();
        if (tp.x != 0 && tp.y != 0) {
             Serial.print("I've been touched at ");
             Serial.print(tp.x);
             Serial.print("x");
             Serial.print(tp.y);
             Serial.println("!");
        }
    }
}

void Alarm::start(Display &indisp, Ldr &ldr, bool &holiday)
{
    alarmDisplay = indisp;
    alarmLdr = ldr;
    xTaskCreate(alarm_clock, "Alarm Clock", 4096, NULL, 10, &alarmTaskHandle);
}

void Alarm::restart()
{
    vTaskDelete(alarmTaskHandle);
    xTaskCreate(alarm_clock, "Alarm Clock", 4096, NULL, 10, &alarmTaskHandle);
}
