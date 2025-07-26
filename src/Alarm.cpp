#include <time.h>
#include "Alarm.h"
#include "Screamer.h"

#include <TFT_eSPI.h>
#include <Preferences.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <TFT_eWidget.h>
#include <sstream>

struct tm timeinfo3;
Display alarmDisplay;
Ldr alarmLdr;
Screamer screamer = Screamer();
bool alarmHoliday = false;

SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSprite *snoozeSprite;

ButtonWidget *stopButton;
ButtonWidget *snoozeButton;

char alarmList[][20] = {"", "", "", "", "", ""};
int lastAlarmCheck = 100;

TaskHandle_t alarmTaskHandle = NULL;

char stop_text[] = "Stop";
char snooze_text[] = "Snooze";


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
            preferences.getBytes("alarms", &buf, size);
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
            Serial.print(" Offset: ");
            Serial.print(currentTm.tm_isdst);
            Serial.print("\n");
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

    snoozeSprite->fillSprite(BACKGROUND_COLOUR);
    snoozeSprite->setTextColor((TEXT_R << (5 + 6)) | (TEXT_G << 5) | TEXT_B);
    snoozeSprite->drawString("Zzzz", 0, 0);
    snoozeSprite->pushSprite(250, 205);

    struct tm nowTm;
    getLocalTime(&nowTm);
    time_t snoozeStartTimestamp = mktime(&nowTm);

    getLocalTime(&nowTm);
    time_t nowTimestamp = mktime(&nowTm);

    bool stop = false;
    long countdown = 0;
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
        vTaskDelay(200 / portTICK_PERIOD_MS);

        getLocalTime(&nowTm);
        nowTimestamp = mktime(&nowTm);

        long remaining = (snoozeStartTimestamp + (SNOOZE_PERIOD * 60)) - nowTimestamp;
        if (countdown != remaining) {
            char buffer[30];
            int ret = snprintf(buffer, sizeof(buffer), "%ld", remaining);
            char * str = buffer;

            snoozeButton->initButtonUL(170, 100, 150, 60, TFT_BLUE, TFT_RED, TFT_BLACK, str, 2);
            snoozeButton->drawSmoothButton(false, 3, TFT_BLACK);
            snoozeButton->flush();
            countdown = remaining;
        }
    }
    Serial.println("Leaving snooze loop");
    snoozeSprite->fillSprite(BACKGROUND_COLOUR);
    snoozeSprite->pushSprite(250, 205);
    snoozeButton->initButtonUL(170, 100, 150, 60, TFT_BLUE, TFT_RED, TFT_BLACK, snooze_text, 2);
    snoozeButton->drawSmoothButton(false, 3, TFT_BLACK);
    snoozeButton->flush();
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

    stopButton->initButtonUL(10, 100, 150, 60, TFT_BLUE, TFT_RED, TFT_BLACK, stop_text, 2);
    stopButton->drawSmoothButton(false, 3, TFT_BLACK);
    snoozeButton->initButtonUL(170, 100, 150, 60, TFT_BLUE, TFT_RED, TFT_BLACK, snooze_text, 2);
    snoozeButton->drawSmoothButton(false, 3, TFT_BLACK);

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

    stopButton->initButtonUL(10, 100, 150, 60, TFT_BLACK, TFT_BLACK, TFT_BLACK, stop_text, 2);
    stopButton->drawSmoothButton(false, 3, TFT_BLACK);
    snoozeButton->initButtonUL(170, 100, 150, 60, TFT_BLACK, TFT_BLACK, TFT_BLACK, snooze_text, 2);
    snoozeButton->drawSmoothButton(false, 3, TFT_BLACK);
}

void alarm_clock(void *pvParameters)
{

    TFT_eSPI tft = alarmDisplay.get_tft();
    snoozeSprite = new TFT_eSprite(&tft);

    // Black with black surround and black text, for now.
    stopButton = new ButtonWidget(&tft);
    stopButton->initButtonUL(10, 100, 150, 60, TFT_BLACK, TFT_BLACK, TFT_BLACK, stop_text, 2);
    stopButton->drawSmoothButton(false, 3, TFT_BLACK);

    // Black with black surround and black text, for now.
    snoozeButton = new ButtonWidget(&tft);
    snoozeButton->initButtonUL(170, 100, 150, 60, TFT_BLACK, TFT_BLACK, TFT_BLACK, snooze_text, 2);
    snoozeButton->drawSmoothButton(false, 3, TFT_BLACK);

    snoozeSprite->createSprite(80, 25);
    snoozeSprite->setColorDepth(8);
    snoozeSprite->setFreeFont(&FreeSansBold12pt7b);
    snoozeSprite->setTextSize(1);
    snoozeSprite->fillSprite(BACKGROUND_COLOUR);
    snoozeSprite->pushSprite(250, 205);

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

void Alarm::start(const Display &indisp, const Ldr &ldr)
{
    alarmDisplay = indisp;
    alarmLdr = ldr;

    mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin(mySpi);

    xTaskCreate(alarm_clock, "Alarm Clock", 4096, NULL, 10, &alarmTaskHandle);
}
