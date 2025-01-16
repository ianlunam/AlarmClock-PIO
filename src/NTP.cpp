#include <Arduino.h>
#include "NTP.h"
#include <time.h>

struct tm timeinfo;

NTP::NTP(){}

void NTP::start()
{
    configTzTime(TIMEZONE, "nz.pool.ntp.org");
    if (!getLocalTime(&timeinfo)) {
        return;
    }
    Serial.println("NTP updated");
}
