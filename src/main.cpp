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

void setup() {
    Serial.begin(115200);
    Serial.println("Starting");

    display.init();
    network.connect();
    mainClock.start(display);
    ldr.start(display);
    alarmPart.start(display);
    webServer.start(alarmPart);
}

void loop() {
    network.check();
    vTaskDelay(300 / portTICK_PERIOD_MS);
}
