#include <Arduino.h>
#include "NTP.h"
#include <time.h>

const char* tz = "NZST-12NZDT,M9.5.0,M4.1.0/3";
struct tm timeinfo;

NTP::NTP(){}

void NTP::start()
{
    configTzTime(tz, "nz.pool.ntp.org");
    if (!getLocalTime(&timeinfo)) {
        return;
    }
    Serial.println("NTP updated");
}
