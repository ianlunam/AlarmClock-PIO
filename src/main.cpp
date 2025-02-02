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

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting");

    // Turn off LED
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, HIGH);

    display.start();
    network.start(display);
    ldr.start(display);
    mainClock.start(display);
    webServer.start();
    alarmPart.start(display, ldr);
    homeAssistant.start(display, alarmPart);
}

void loop(){}
