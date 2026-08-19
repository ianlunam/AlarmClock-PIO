#include <unity.h>
#include "WeatherIconShape.h"

void setUp(void) {}
void tearDown(void) {}

void test_maps_known_conditions(void)
{
    TEST_ASSERT_EQUAL(SHAPE_SUN, mapConditionToShape("sunny"));
    TEST_ASSERT_EQUAL(SHAPE_CLEAR_NIGHT, mapConditionToShape("clear-night"));
    TEST_ASSERT_EQUAL(SHAPE_PARTLY_CLOUDY, mapConditionToShape("partlycloudy"));
    TEST_ASSERT_EQUAL(SHAPE_CLOUDY, mapConditionToShape("cloudy"));
    TEST_ASSERT_EQUAL(SHAPE_FOG, mapConditionToShape("fog"));
    TEST_ASSERT_EQUAL(SHAPE_WIND, mapConditionToShape("windy"));
    TEST_ASSERT_EQUAL(SHAPE_WIND, mapConditionToShape("windy-variant"));
    TEST_ASSERT_EQUAL(SHAPE_RAIN, mapConditionToShape("rainy"));
    TEST_ASSERT_EQUAL(SHAPE_POURING, mapConditionToShape("pouring"));
    TEST_ASSERT_EQUAL(SHAPE_LIGHTNING, mapConditionToShape("lightning"));
    TEST_ASSERT_EQUAL(SHAPE_LIGHTNING_RAIN, mapConditionToShape("lightning-rainy"));
    TEST_ASSERT_EQUAL(SHAPE_SNOW, mapConditionToShape("snowy"));
    TEST_ASSERT_EQUAL(SHAPE_SNOW_RAIN, mapConditionToShape("snowy-rainy"));
    TEST_ASSERT_EQUAL(SHAPE_HAIL, mapConditionToShape("hail"));
}

void test_is_case_insensitive(void)
{
    TEST_ASSERT_EQUAL(SHAPE_SUN, mapConditionToShape("Sunny"));
    TEST_ASSERT_EQUAL(SHAPE_SUN, mapConditionToShape("SUNNY"));
    TEST_ASSERT_EQUAL(SHAPE_LIGHTNING_RAIN, mapConditionToShape("Lightning-Rainy"));
}

void test_unknown_condition_maps_to_unknown(void)
{
    TEST_ASSERT_EQUAL(SHAPE_UNKNOWN, mapConditionToShape("exceptional"));
    TEST_ASSERT_EQUAL(SHAPE_UNKNOWN, mapConditionToShape(""));
    TEST_ASSERT_EQUAL(SHAPE_UNKNOWN, mapConditionToShape("garbage"));
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_maps_known_conditions);
    RUN_TEST(test_is_case_insensitive);
    RUN_TEST(test_unknown_condition_maps_to_unknown);
    return UNITY_END();
}
