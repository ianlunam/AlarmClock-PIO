#include <unity.h>
#include <cstring>
#include <cstdlib>
#include "AlarmLogic.h"

namespace
{
    // Builds a normalized struct tm for a given wall-clock date/time. mktime
    // fills in tm_wday (and may correct tm_isdst) for the given date under
    // whatever TZ is currently set.
    struct tm makeTm(int year, int month1to12, int day, int hour, int minute, int isdst)
    {
        struct tm t = {};
        t.tm_year = year - 1900;
        t.tm_mon = month1to12 - 1;
        t.tm_mday = day;
        t.tm_hour = hour;
        t.tm_min = minute;
        t.tm_sec = 0;
        t.tm_isdst = isdst;
        mktime(&t);
        return t;
    }

    // An alarm enabled only for the given tm_wday (0=Sunday..6=Saturday).
    AlarmEntry makeAlarm(int hour, int minute, int wday)
    {
        AlarmEntry a = {};
        strncpy(a.name, "test", sizeof(a.name) - 1);
        a.hour = hour;
        a.minute = minute;
        a.enabled = true;
        bool *days[7] = {&a.sunday, &a.monday, &a.tuesday, &a.wednesday,
                          &a.thursday, &a.friday, &a.saturday};
        *days[wday] = true;
        return a;
    }
}

void setUp(void)
{
    // Match the firmware's configured TIMEZONE (platformio.ini) so the DST
    // tests below exercise the same rules the real device runs under.
    setenv("TZ", "NZST-12NZDT,M9.5.0,M4.1.0/3", 1);
    tzset();
}

void tearDown(void) {}

void test_fires_when_day_and_time_match(void)
{
    struct tm now = makeTm(2024, 1, 8, 7, 30, -1);
    AlarmEntry alarm = makeAlarm(7, 30, now.tm_wday);
    TEST_ASSERT_TRUE(alarmMatchesNow(alarm, now, false));
}

void test_does_not_fire_when_disabled(void)
{
    struct tm now = makeTm(2024, 1, 8, 7, 30, -1);
    AlarmEntry alarm = makeAlarm(7, 30, now.tm_wday);
    alarm.enabled = false;
    TEST_ASSERT_FALSE(alarmMatchesNow(alarm, now, false));
}

void test_does_not_fire_on_wrong_day(void)
{
    struct tm now = makeTm(2024, 1, 8, 7, 30, -1);
    int wrongDay = (now.tm_wday + 1) % 7;
    AlarmEntry alarm = makeAlarm(7, 30, wrongDay);
    TEST_ASSERT_FALSE(alarmMatchesNow(alarm, now, false));
}

void test_does_not_fire_at_wrong_time(void)
{
    struct tm now = makeTm(2024, 1, 8, 7, 30, -1);
    AlarmEntry alarm = makeAlarm(7, 31, now.tm_wday);
    TEST_ASSERT_FALSE(alarmMatchesNow(alarm, now, false));
}

void test_holiday_skips_when_flagged(void)
{
    struct tm now = makeTm(2024, 1, 8, 7, 30, -1);
    AlarmEntry alarm = makeAlarm(7, 30, now.tm_wday);
    alarm.skip_phols = true;
    TEST_ASSERT_FALSE(alarmMatchesNow(alarm, now, true));
}

void test_holiday_does_not_skip_when_not_flagged(void)
{
    struct tm now = makeTm(2024, 1, 8, 7, 30, -1);
    AlarmEntry alarm = makeAlarm(7, 30, now.tm_wday);
    alarm.skip_phols = false;
    TEST_ASSERT_TRUE(alarmMatchesNow(alarm, now, true));
}

void test_fires_at_correct_wall_clock_time_during_dst(void)
{
    // 20 January is deep in NZ summer (NZDT, isdst=1). An alarm set for
    // 7:00 should still ring at 7:00 on the wall clock, not 6:00 or 8:00 -
    // this is what the "- currentTm.tm_isdst" correction in AlarmLogic.cpp
    // exists for.
    struct tm now = makeTm(2024, 1, 20, 7, 0, 1);
    TEST_ASSERT_EQUAL(1, now.tm_isdst);
    AlarmEntry alarm = makeAlarm(7, 0, now.tm_wday);
    TEST_ASSERT_TRUE(alarmMatchesNow(alarm, now, false));
}

void test_does_not_fire_one_hour_off_during_dst(void)
{
    // The off-by-one-hour bug the DST correction guards against.
    struct tm now = makeTm(2024, 1, 20, 8, 0, 1);
    TEST_ASSERT_EQUAL(1, now.tm_isdst);
    AlarmEntry alarm = makeAlarm(7, 0, now.tm_wday);
    TEST_ASSERT_FALSE(alarmMatchesNow(alarm, now, false));
}

void test_fires_at_correct_wall_clock_time_outside_dst(void)
{
    // 20 June is deep in NZ winter (NZST, isdst=0) - the baseline case.
    struct tm now = makeTm(2024, 6, 20, 7, 0, 0);
    TEST_ASSERT_EQUAL(0, now.tm_isdst);
    AlarmEntry alarm = makeAlarm(7, 0, now.tm_wday);
    TEST_ASSERT_TRUE(alarmMatchesNow(alarm, now, false));
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_fires_when_day_and_time_match);
    RUN_TEST(test_does_not_fire_when_disabled);
    RUN_TEST(test_does_not_fire_on_wrong_day);
    RUN_TEST(test_does_not_fire_at_wrong_time);
    RUN_TEST(test_holiday_skips_when_flagged);
    RUN_TEST(test_holiday_does_not_skip_when_not_flagged);
    RUN_TEST(test_fires_at_correct_wall_clock_time_during_dst);
    RUN_TEST(test_does_not_fire_one_hour_off_during_dst);
    RUN_TEST(test_fires_at_correct_wall_clock_time_outside_dst);
    return UNITY_END();
}
