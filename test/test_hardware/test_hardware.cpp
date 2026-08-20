// Hardware-in-the-loop tests: these run ON the real ESP32 (flashed and
// driven over serial by `pio test`), unlike test_alarm_logic/etc which run
// natively on your PC. Use this file for things that genuinely need real
// hardware - a sensor pin, real flash storage, real concurrent tasks - not
// for logic that could be tested natively instead.
#include <Arduino.h>
#include <unity.h>
#include <Preferences.h>

void setUp(void) {}
void tearDown(void) {}

void test_ldr_reads_valid_range(void)
{
    analogSetAttenuation(ADC_0db);
    pinMode(LDR_PIN, INPUT);
    for (int i = 0; i < 5; i++)
    {
        int value = analogRead(LDR_PIN);
        TEST_ASSERT_GREATER_OR_EQUAL(0, value);
        TEST_ASSERT_LESS_OR_EQUAL(4095, value);
        delay(20);
    }
}

void test_nvs_roundtrip(void)
{
    // Uses its own namespace/key, never "alarmStore" - this must never
    // touch real configured alarms.
    Preferences prefs;
    TEST_ASSERT_TRUE(prefs.begin("testStore", false));

    const char *testValue = "hello-hardware-test";
    prefs.putString("greeting", testValue);

    String readBack = prefs.getString("greeting", "");
    TEST_ASSERT_EQUAL_STRING(testValue, readBack.c_str());

    prefs.remove("greeting");
    prefs.end();
}

namespace
{
    SemaphoreHandle_t testMutex;
    volatile int sharedCounter;
    const int INCREMENTS_PER_TASK = 2000;

    void incrementTask(void *pvParameters)
    {
        for (int i = 0; i < INCREMENTS_PER_TASK; i++)
        {
            xSemaphoreTake(testMutex, portMAX_DELAY);
            sharedCounter++;
            xSemaphoreGive(testMutex);
        }
        vTaskDelete(NULL);
    }
}

// Regression test for the exact class of bug that caused the random reboots
// fixed by DisplayLock: two tasks, pinned to different cores so they can
// genuinely run at the same instant, hammering a shared counter through a
// mutex. If the mutex ever failed to serialize them, increments would be
// lost and the final count would come up short.
void test_mutex_prevents_lost_updates(void)
{
    testMutex = xSemaphoreCreateMutex();
    sharedCounter = 0;

    xTaskCreatePinnedToCore(incrementTask, "inc1", 2048, NULL, 10, NULL, 0);
    xTaskCreatePinnedToCore(incrementTask, "inc2", 2048, NULL, 10, NULL, 1);

    delay(3000); // generous time for both tasks to finish

    TEST_ASSERT_EQUAL_INT(INCREMENTS_PER_TASK * 2, sharedCounter);
    vSemaphoreDelete(testMutex);
}

void setup()
{
    delay(2000); // let the board settle and the serial monitor attach
    UNITY_BEGIN();
    RUN_TEST(test_ldr_reads_valid_range);
    RUN_TEST(test_nvs_roundtrip);
    RUN_TEST(test_mutex_prevents_lost_updates);
    UNITY_END();
}

void loop() {}
