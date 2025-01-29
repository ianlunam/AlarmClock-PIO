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

SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSprite *spr;

char alarmList[][20] = {"", "", "", "", "", ""};
int lastAlarmCheck = 100;

TaskHandle_t alarmTaskHandle = NULL;
int snooze_colour = (TEXT_R << (5 + 6)) | (TEXT_G << 5) | TEXT_B;

Alarm::Alarm() {}

void getAlarmList()
{
    Preferences preferences;

    preferences.begin("alarmStore", false);
    if (preferences.isKey("alarms"))
    {
        int size = preferences.getBytesLength("alarms");
        if (size > 0)
        {
            char *buf[size + 1];
            int result = preferences.getBytes("alarms", &buf, size);
            memcpy(&alarmList, buf, size);
            preferences.end();
            return;
        }
    }
    for (int x = 0; x < 6; x++)
    {
        strcpy(alarmList[x], "");
    }
    preferences.putBytes("alarms", &alarmList, sizeof(alarmList));
    preferences.end();
    return;
}

bool getAlarm(char *name, AlarmEntry &newAlarm)
{
    getAlarmList();

    bool inList = false;
    for (int x = 0; x < 6; x++)
    {
        if (strcmp(alarmList[x], name) == 0)
        {
            inList = true;
        }
    }

    if (!inList)
    {
        return false;
    }

    Preferences alarmStore;
    alarmStore.begin("alarmStore", true);

    if (alarmStore.isKey(name))
    {
        int size = alarmStore.getBytesLength(name);
        if (size > 0)
        {
            char *buf[size + 1];
            int result = alarmStore.getBytes(name, &buf, size);
            memcpy(&newAlarm, buf, size);
            alarmStore.end();
            return true;
        }
    }
    alarmStore.end();
    return false;
}

bool alarmTriggerNow()
{
    struct tm currentTm;
    if (!getLocalTime(&currentTm))
    {
        return false;
    }
    if (currentTm.tm_min == lastAlarmCheck)
        return false; // Have we checked this minute?
    lastAlarmCheck = currentTm.tm_min;

    currentTm.tm_sec = 0; // reset to first second of minute to make comparison easier
    time_t currentTime = mktime(&currentTm);

    getAlarmList();

    for (int x = 0; x < 6; x++)
    {
        if (strlen(alarmList[x]) == 0)
        {
            continue;
        }
        // Serial.print("Checking alarm ");
        // Serial.println(alarmList[x]);
        AlarmEntry nextAlarm;
        if (getAlarm(alarmList[x], nextAlarm))
        {
            // Serial.println(toString(nextAlarm));

            // Skip the alarm if ...
            if (!nextAlarm.enabled)
                continue;
            if (alarmHoliday and nextAlarm.skip_phols)
                continue;

            // Skip if today isn't enabled
            if (currentTm.tm_wday == 0 and !nextAlarm.sunday)
                continue;
            if (currentTm.tm_wday == 1 and !nextAlarm.monday)
                continue;
            if (currentTm.tm_wday == 2 and !nextAlarm.tuesday)
                continue;
            if (currentTm.tm_wday == 3 and !nextAlarm.wednesday)
                continue;
            if (currentTm.tm_wday == 4 and !nextAlarm.thursday)
                continue;
            if (currentTm.tm_wday == 5 and !nextAlarm.friday)
                continue;
            if (currentTm.tm_wday == 6 and !nextAlarm.saturday)
                continue;

            struct tm t = {0};
            t.tm_year = currentTm.tm_year; // Construct tm as per today for alarm time at zero seconds
            t.tm_mon = currentTm.tm_mon;
            t.tm_mday = currentTm.tm_mday;
            t.tm_hour = nextAlarm.hour - currentTm.tm_isdst;
            t.tm_min = nextAlarm.minute;
            t.tm_sec = 0;
            time_t alarmTime = mktime(&t); // convert to seconds
            Serial.print("Now: ");
            Serial.print(currentTime);
            Serial.print(" Alarm: ");
            Serial.print(alarmTime);
            if (alarmTime == currentTime)
            {
                if (nextAlarm.once)
                {
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

void Alarm::set_public_holiday(bool state)
{
    alarmHoliday = state;
}

bool snooze()
{
    Serial.println("Entering snooze state");

    screamer.stop();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    alarmDisplay.set_backlight(BL_MAX);

    spr->fillSprite(BACKGROUND_COLOUR);
    spr->setTextColor(snooze_colour);
    spr->drawString("Zzzz", 0, 0);
    spr->pushSprite(250, 205);

    struct tm nowTm;
    getLocalTime(&nowTm);
    time_t snoozeStartTimestamp = mktime(&nowTm);

    getLocalTime(&nowTm);
    time_t nowTimestamp = mktime(&nowTm);

    bool stop = false;
    while (nowTimestamp < (snoozeStartTimestamp + (SNOOZE_PERIOD * 60)))
    {
        if (ts.tirqTouched() && ts.touched())
        {
            TS_Point p = ts.getPoint();
            if (p.x < 2000)
            {
                stop = true;
                break;
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);

        getLocalTime(&nowTm);
        nowTimestamp = mktime(&nowTm);
        Serial.print("Now: ");
        Serial.print(nowTimestamp);
        Serial.print(" Unsnooze: ");
        Serial.println((snoozeStartTimestamp + (SNOOZE_PERIOD * 60)));
    }
    Serial.println("Leaving snooze loop");
    spr->fillSprite(BACKGROUND_COLOUR);
    spr->pushSprite(250, 205);
    spr->deleteSprite();
    Serial.println("Deleted sprite");
    if (stop == false)
    {
        screamer.start();
    }
    return stop;
}

void scream()
{
    Serial.println("Entering alarming state");
    screamer.start();

    for (;;)
    {
        if (ts.tirqTouched() && ts.touched())
        {
            TS_Point p = ts.getPoint();
            if (p.x < 2000)
            {
                break;
            }
            if (snooze())
            {
                break;
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    screamer.stop();
    Serial.println("Exiting alarming state");
    vTaskDelay(500 / portTICK_PERIOD_MS);
    alarmDisplay.set_backlight(BL_MAX);
}

void alarm_clock(void *pvParameters)
{

    TFT_eSPI tft = alarmDisplay.get_tft();
    spr = new TFT_eSprite(&tft);

    spr->createSprite(80, 25);
    spr->setColorDepth(8);
    spr->setFreeFont(&FreeSansBold12pt7b);
    spr->setTextSize(1);
    spr->fillSprite(BACKGROUND_COLOUR);
    spr->pushSprite(250, 205);

    Serial.println("Alarm started");
    for (;;)
    {
        if (alarmTriggerNow() == true)
        {
            scream();
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void Alarm::start(Display &indisp, Ldr &ldr)
{
    alarmDisplay = indisp;
    alarmLdr = ldr;

    mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin(mySpi);

    xTaskCreate(alarm_clock, "Alarm Clock", 4096, NULL, 10, &alarmTaskHandle);
}
