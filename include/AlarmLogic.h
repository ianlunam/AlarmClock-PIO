#ifndef AlarmLogic_h
#define AlarmLogic_h

#include "AlarmEntry.h"
#include <time.h>

// Pure decision: does this alarm fire at currentTm? No I/O, no NVS, no
// hardware - takes plain data so it can be unit tested on its own
// (see test/test_alarm_logic).
bool alarmMatchesNow(const AlarmEntry &alarm, const struct tm &currentTm, bool isHoliday);

#endif
