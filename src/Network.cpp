#include <Arduino.h>

#include <WiFi.h>
#include "Network.h"

Network::Network(){}

void check(void *pvParameters)
{
    for(;;) {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        if (WiFi.status() != WL_CONNECTED) {
            Serial.print(millis());
            Serial.println("Reconnecting to WiFi...");
            WiFi.disconnect();
            WiFi.reconnect();

            while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(".");
            }
            Serial.println("\nWiFi connected.");
            configTzTime(TIMEZONE, NTP_SERVER);
        }
    }
}

void Network::start()
{
    WiFi.begin(WIFI_SSID, WIFI_PWD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");
    configTzTime(TIMEZONE, NTP_SERVER);

    xTaskCreate(check, "Display MQTT Data", 4096, NULL, 10, NULL);
}
