#include "AlarmLogic.h"

bool alarmMatchesNow(const AlarmEntry &alarm, const struct tm &currentTm, bool isHoliday)
{
    if (!alarm.enabled)
    {
        return false;
    }
    if (isHoliday && alarm.skip_phols)
    {
        return false;
    }

    // tm_wday is 0=Sunday..6=Saturday, matching this array's order.
    bool dayEnabled[7] = {
        alarm.sunday, alarm.monday, alarm.tuesday,
        alarm.wednesday, alarm.thursday, alarm.friday,
        alarm.saturday};
    if (!dayEnabled[currentTm.tm_wday])
    {
        return false;
    }

    struct tm current = currentTm;
    current.tm_sec = 0; // reset to first second of minute to make comparison easier
    time_t currentTime = mktime(&current);

    struct tm t = {0};
    t.tm_year = currentTm.tm_year; // Construct tm as per today for alarm time at zero seconds
    t.tm_mon = currentTm.tm_mon;
    t.tm_mday = currentTm.tm_mday;
    t.tm_hour = alarm.hour - currentTm.tm_isdst;
    t.tm_min = alarm.minute;
    t.tm_sec = 0;
    time_t alarmTime = mktime(&t); // convert to seconds

    return alarmTime == currentTime;
}
