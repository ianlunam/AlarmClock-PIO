#include <Arduino.h>

#include <WiFi.h>
#include "Network.h"

Network::Network() {}

void restart(WiFiEvent_t event, WiFiEventInfo_t info)
{

    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");
    configTzTime(TIMEZONE, NTP_SERVER);
}

void started(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("WiFi Up");
}

void Network::start()
{
    WiFi.onEvent(restart, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(started, ARDUINO_EVENT_WIFI_STA_CONNECTED);

    WiFi.begin(WIFI_SSID, WIFI_PWD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");
    configTzTime(TIMEZONE, NTP_SERVER);
}
