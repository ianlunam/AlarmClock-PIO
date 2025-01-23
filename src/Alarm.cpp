#include <time.h>
#include "Alarm.h"
#include "Screamer.h"

#include <TFT_eSPI.h>
#include <Preferences.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

struct tm timeinfo3;
Display alarmDisplay;
Ldr alarmLdr;
Screamer screamer = Screamer();
bool alarmHoliday = false;

TFT_eSPI_Button stop_button;
TFT_eSPI_Button snooze_button;

SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

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


bool getAlarm(char* name, AlarmEntry& newAlarm) {
    getAlarmList();

    bool inList = false;
    for (int x = 0; x < 6; x++) {
        if (strcmp(alarmList[x], name) == 0) {
            inList = true;
        }
    }

    if (!inList) {
        return false;
    }

    Preferences alarmStore;
    alarmStore.begin("alarmStore", true);

    if (alarmStore.isKey(name)) {
        int size = alarmStore.getBytesLength(name);
        if (size > 0) {
            char* buf[size + 1];
            int result = alarmStore.getBytes(name, &buf, size);
            memcpy(&newAlarm, buf, size);
            alarmStore.end();
            return true;
        }
    }
    alarmStore.end();
    return false;
}


bool alarmTriggerNow(bool isPhol) {
    struct tm currentTm;
    if (!getLocalTime(&currentTm)) {
        return false;
    }
    if (currentTm.tm_sec > 10) return false;  // Only check if in the first 10s of the minute

    time_t currentTime = mktime(&currentTm);
    if (lastAlarmCheck > (currentTime - 20)) return false;  // Only check if we last checked more than 20s ago (ie: greater than 10s)
    lastAlarmCheck = currentTime;

    currentTm.tm_sec = 0;  // reset to first second of minute to make comparison easier
    currentTime = mktime(&currentTm);

    getAlarmList();

    for (int x = 0; x < 6; x++) {
        if (strlen(alarmList[x]) == 0) {
            continue;
        }
        // Serial.print("Checking alarm ");
        // Serial.println(alarmList[x]);
        AlarmEntry nextAlarm;
        if (getAlarm(alarmList[x], nextAlarm)) {
            // Serial.println(toString(nextAlarm));

            // Skip the alarm if ...
            if (!nextAlarm.enabled) continue;
            if (isPhol and nextAlarm.skip_phols) continue;

            // Skip if today isn't enabled
            if (currentTm.tm_wday == 0 and !nextAlarm.sunday) continue;
            if (currentTm.tm_wday == 1 and !nextAlarm.monday) continue;
            if (currentTm.tm_wday == 2 and !nextAlarm.tuesday) continue;
            if (currentTm.tm_wday == 3 and !nextAlarm.wednesday) continue;
            if (currentTm.tm_wday == 4 and !nextAlarm.thursday) continue;
            if (currentTm.tm_wday == 5 and !nextAlarm.friday) continue;
            if (currentTm.tm_wday == 6 and !nextAlarm.saturday) continue;

            struct tm t = { 0 };
            t.tm_year = currentTm.tm_year;  // Construct tm as per today for alarm time at zero seconds
            t.tm_mon = currentTm.tm_mon;
            t.tm_mday = currentTm.tm_mday;
            t.tm_hour = nextAlarm.hour - currentTm.tm_isdst;
            t.tm_min = nextAlarm.minute;
            t.tm_sec = 0;
            time_t alarmTime = mktime(&t);  // convert to seconds
            Serial.print("Now: ");
            Serial.print(currentTime);
            Serial.print(" Alarm: ");
            Serial.print(alarmTime);
            Serial.print(" Lasttime: ");
            Serial.println(alarmedLast[x]);
            if (alarmTime == currentTime && alarmedLast[x] < (currentTime - 20)) {  // Using same 20s as above to debounce
                alarmedLast[x] = currentTime;
                if (nextAlarm.once) {
                    nextAlarm.enabled = false;
                    Preferences preferences;
                    preferences.begin("alarmStore", false);
                    preferences.putBytes(nextAlarm.name, &nextAlarm, sizeof(nextAlarm));
                    preferences.end();
                }
                return true;
            }
        }
    }
    return false;
}


bool alarming(bool isPhol) {
    if (alarmState == ALARM_ON) {
        return true;
    } else if (alarmState == ALARM_SNOOZE) {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            return false;
        }
        time_t now = mktime(&timeinfo);
        if (now > (alarmStarted + (60 * 7))) {
            Serial.println("Snooze over");
            alarmState = ALARM_ON;
            alarmStarted = now;
            return true;
        }
        return false;
    } else {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            return false;
        }
        time_t now = mktime(&timeinfo);
        if (alarmTriggerNow(isPhol)) {
            alarmState = ALARM_ON;
            alarmStarted = now;
            return true;
        }
        return false;
    }
}


void Alarm::turnOff() {
    Serial.println("Alarm off button");
    alarmState = ALARM_OFF;
}

void Alarm::turnOn() {
    Serial.println("Alarm on button");
    alarmState = ALARM_ON;
}


void Alarm::snooze() {
    Serial.println("Alarm snooze button");
    alarmState = ALARM_SNOOZE;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return;
    }
    snoozeTime = mktime(&timeinfo);
}

bool Alarm::isSnoozed() {
    if (alarmState == ALARM_SNOOZE) {
        return true;
    } else {
        return false;
    }
}

bool Alarm::isOn() {
    if (alarmState == ALARM_ON) {
        return true;
    } else {
        return false;
    }
}

void Alarm::set_public_holiday(bool state) {
    alarmHoliday = state;
}

void scream(){
    Serial.println("Alarmed");
    screamer.start();
    // Position is center of button.
    snooze_button = drawButton("Snooze", 100, 130, 100, 95);
    stop_button = drawButton("Stop", 260, 130, 100, 95);

    for (;;){
        if (ts.tirqTouched() && ts.touched()) {
            TS_Point p = ts.getPoint();
            Serial.print("Pressure = ");
            Serial.print(p.z);
            Serial.print(", x = ");
            Serial.print(p.x);
            Serial.print(", y = ");
            Serial.print(p.y);
            Serial.println();
            break;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    screamer.stop();
}

void alarm_clock(void *pvParameters)
{
    for(;;) {
        bool tmp = alarmTriggerNow(alarmHoliday);
        if (tmp == true) {
            Serial.println("Entering alarming state");
            scream();
            Serial.println("Exiting alarming state");
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void Alarm::start(Display &indisp, Ldr &ldr, bool &holiday)
{
    alarmDisplay = indisp;
    alarmLdr = ldr;

    mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin(mySpi);

    xTaskCreate(alarm_clock, "Alarm Clock", 4096, NULL, 10, &alarmTaskHandle);
}

void Alarm::restart()
{
    vTaskDelete(alarmTaskHandle);
    xTaskCreate(alarm_clock, "Alarm Clock", 4096, NULL, 10, &alarmTaskHandle);
}
