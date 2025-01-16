#include <Arduino.h>

#include "Network.h"
#include "Display.h"
#include "Clock.h"
#include "Ldr.h"
#include "Alarm.h"
#include "WebServer.h"

Network network;
Display display;
Clock mainClock;
Ldr ldr;
Alarm alarmPart;
WebServer webServer;

bool holiday = false;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting");

    display.init();
    network.connect();
    ldr.start(display);
    mainClock.start(display);
    webServer.start(alarmPart);
    alarmPart.start(display, ldr, holiday);
}

void loop() {
    network.check();
    vTaskDelay(300 / portTICK_PERIOD_MS);
}
