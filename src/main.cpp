#include <Arduino.h>

#include "Network.h"
#include "Display.h"
#include "Clock.h"
#include "Ldr.h"
#include "Alarm.h"
#include "WebServer.h"
#include "HomeAssistant.h"

Network network;
Display display;
Clock mainClock;
Ldr ldr;
Alarm alarmPart;
WebServer webServer;
HomeAssistant homeAssistant;

bool holiday = false;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting");

    // Turn off LED
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, HIGH);

    display.init();
    network.connect();
    ldr.start(display);
    mainClock.start(display);
    webServer.start(alarmPart);
    alarmPart.start(display, ldr, holiday);
    homeAssistant.start(display, alarmPart);
}

void loop() {
    network.check();
    vTaskDelay(100 / portTICK_PERIOD_MS);
}
