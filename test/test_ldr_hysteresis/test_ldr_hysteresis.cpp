#include <unity.h>
#include "LdrHysteresis.h"

namespace
{
    const int DARK_THRESHOLD = 0;
    const int LIGHT_THRESHOLD = 20;
    const int DEBOUNCE_COUNT = 5;
}

void setUp(void) {}
void tearDown(void) {}

void test_stays_no_change_within_debounce(void)
{
    int darkCount = 0, lightCount = 0;
    // Currently dim (backlightDown=true), reading is dark, but not yet
    // enough consecutive readings to act.
    LdrDecision decision = LDR_NO_CHANGE;
    for (int i = 0; i < DEBOUNCE_COUNT - 1; i++)
    {
        decision = ldrUpdate(0, true, darkCount, lightCount, DARK_THRESHOLD, LIGHT_THRESHOLD, DEBOUNCE_COUNT);
        TEST_ASSERT_EQUAL(LDR_NO_CHANGE, decision);
    }
    TEST_ASSERT_EQUAL(DEBOUNCE_COUNT - 1, darkCount);
}

void test_goes_bright_after_debounce_count_of_dark_readings(void)
{
    int darkCount = 0, lightCount = 0;
    LdrDecision decision = LDR_NO_CHANGE;
    for (int i = 0; i < DEBOUNCE_COUNT; i++)
    {
        decision = ldrUpdate(0, true, darkCount, lightCount, DARK_THRESHOLD, LIGHT_THRESHOLD, DEBOUNCE_COUNT);
    }
    TEST_ASSERT_EQUAL(LDR_GO_BRIGHT, decision);
}

void test_goes_dim_after_debounce_count_of_light_readings(void)
{
    int darkCount = 0, lightCount = 0;
    LdrDecision decision = LDR_NO_CHANGE;
    for (int i = 0; i < DEBOUNCE_COUNT; i++)
    {
        decision = ldrUpdate(100, false, darkCount, lightCount, DARK_THRESHOLD, LIGHT_THRESHOLD, DEBOUNCE_COUNT);
    }
    TEST_ASSERT_EQUAL(LDR_GO_DIM, decision);
}

void test_dead_zone_reading_resets_both_counters(void)
{
    int darkCount = 0, lightCount = 0;
    for (int i = 0; i < DEBOUNCE_COUNT - 1; i++)
    {
        ldrUpdate(0, true, darkCount, lightCount, DARK_THRESHOLD, LIGHT_THRESHOLD, DEBOUNCE_COUNT);
    }
    TEST_ASSERT_EQUAL(DEBOUNCE_COUNT - 1, darkCount);

    // A reading strictly between the two thresholds resets progress -
    // this is what stops noise right at the boundary from flickering.
    ldrUpdate(10, true, darkCount, lightCount, DARK_THRESHOLD, LIGHT_THRESHOLD, DEBOUNCE_COUNT);
    TEST_ASSERT_EQUAL(0, darkCount);
    TEST_ASSERT_EQUAL(0, lightCount);
}

void test_no_repeat_decision_once_already_bright(void)
{
    int darkCount = DEBOUNCE_COUNT, lightCount = 0;
    // backlightDown=false means we're already bright - more dark readings
    // shouldn't produce another GoBright.
    LdrDecision decision = ldrUpdate(0, false, darkCount, lightCount, DARK_THRESHOLD, LIGHT_THRESHOLD, DEBOUNCE_COUNT);
    TEST_ASSERT_EQUAL(LDR_NO_CHANGE, decision);
}

void test_no_repeat_decision_once_already_dim(void)
{
    int darkCount = 0, lightCount = DEBOUNCE_COUNT;
    // backlightDown=true means we're already dim - more light readings
    // shouldn't produce another GoDim.
    LdrDecision decision = ldrUpdate(100, true, darkCount, lightCount, DARK_THRESHOLD, LIGHT_THRESHOLD, DEBOUNCE_COUNT);
    TEST_ASSERT_EQUAL(LDR_NO_CHANGE, decision);
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_stays_no_change_within_debounce);
    RUN_TEST(test_goes_bright_after_debounce_count_of_dark_readings);
    RUN_TEST(test_goes_dim_after_debounce_count_of_light_readings);
    RUN_TEST(test_dead_zone_reading_resets_both_counters);
    RUN_TEST(test_no_repeat_decision_once_already_bright);
    RUN_TEST(test_no_repeat_decision_once_already_dim);
    return UNITY_END();
}
