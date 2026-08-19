#include <time.h>
#include "Alarm.h"
#include "AlarmLogic.h"
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

// Raw XPT2046 touch ADC range for this panel, mapped below to the screen's
// pixel dimensions so touches can be hit-tested against a widget's actual
// on-screen position and size, instead of a hand-picked coordinate box. If
// touches don't land where expected, touch each screen corner, print
// ts.getPoint().x/.y over Serial, and adjust these four numbers to match.
const int16_t TOUCH_RAW_X_MIN = 200;
const int16_t TOUCH_RAW_X_MAX = 2700;
const int16_t TOUCH_RAW_Y_MIN = 300;
const int16_t TOUCH_RAW_Y_MAX = 1000;

ButtonWidget *stopButton;

char alarmList[][20] = {"", "", "", "", "", ""};
int lastAlarmCheck = 100;

SemaphoreHandle_t alarmStoreMutex = xSemaphoreCreateRecursiveMutex();

volatile bool alarmActive = false;

TaskHandle_t alarmTaskHandle = NULL;

char stop_text[] = "Stop";


Alarm::Alarm() {}

void getAlarmList()
{
    AlarmStoreLock lock;
    Preferences preferences;

    preferences.begin("alarmStore", false);
    if (preferences.isKey("alarms"))
    {
        int size = preferences.getBytesLength("alarms");
        if (size > 0)
        {
            char buf[size + 1];
            preferences.getBytes("alarms", &buf, size);
            // Never copy more than alarmList can hold, even if the stored
            // data is a different size than expected (e.g. after a firmware
            // change or downgrade).
            size_t copySize = (size_t)size;
            if (copySize > sizeof(alarmList))
                copySize = sizeof(alarmList);
            memcpy(&alarmList, buf, copySize);
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
    AlarmStoreLock lock;
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
            char buf[size + 1];
            int result = alarmStore.getBytes(name, &buf, size);
            size_t copySize = (size_t)size;
            if (copySize > sizeof(newAlarm))
                copySize = sizeof(newAlarm);
            memcpy(&newAlarm, buf, copySize);
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

    getAlarmList();

    for (int x = 0; x < 6; x++)
    {
        if (strlen(alarmList[x]) == 0)
        {
            continue;
        }
        AlarmEntry nextAlarm = {};
        if (getAlarm(alarmList[x], nextAlarm))
        {
            if (!alarmMatchesNow(nextAlarm, currentTm, alarmHoliday))
                continue;

            if (nextAlarm.once)
            {
                nextAlarm.enabled = false;
                AlarmStoreLock lock;
                Preferences preferences;
                preferences.begin("alarmStore", false);
                preferences.putBytes(nextAlarm.name, &nextAlarm, sizeof(nextAlarm));
                preferences.end();
            }
            return true;
        }
    }
    return false;
}

void Alarm::set_public_holiday(bool state)
{
    alarmHoliday = state;
}

bool touchOnButton(ButtonWidget *button)
{
    if (!(ts.tirqTouched() && ts.touched()))
    {
        return false;
    }
    TS_Point p = ts.getPoint();
    int16_t pixelX = map(p.x, TOUCH_RAW_X_MIN, TOUCH_RAW_X_MAX, 0, 320);
    int16_t pixelY = map(p.y, TOUCH_RAW_Y_MIN, TOUCH_RAW_Y_MAX, 0, 240);
    return button->contains(pixelX, pixelY);
}

void scream()
{
    alarmActive = true;
    screamer.start();

    stopButton->initButtonUL(10, 100, 150, 60, TFT_BLUE, TFT_RED, TFT_BLACK, stop_text, 2);
    stopButton->drawSmoothButton(false, 3, TFT_BLACK);

    for (;;)
    {
        if (touchOnButton(stopButton))
        {
            break;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    screamer.stop();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    alarmDisplay.set_backlight(BL_MAX);

    stopButton->initButtonUL(10, 100, 150, 60, TFT_BLACK, TFT_BLACK, TFT_BLACK, stop_text, 2);
    stopButton->drawSmoothButton(false, 3, TFT_BLACK);

    // Only let the idle-screen tasks resume drawing over this area once the
    // button is actually gone.
    alarmActive = false;
}

void alarm_clock(void *pvParameters)
{

    TFT_eSPI &tft = alarmDisplay.get_tft();

    // Black with black surround and black text, for now.
    stopButton = new ButtonWidget(&tft);
    stopButton->initButtonUL(10, 100, 150, 60, TFT_BLACK, TFT_BLACK, TFT_BLACK, stop_text, 2);
    stopButton->drawSmoothButton(false, 3, TFT_BLACK);

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
