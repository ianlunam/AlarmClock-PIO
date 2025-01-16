#include <Arduino.h>

#include <WiFi.h>
#include "Network.h"
#include "NTP.h"

NTP ntp;


unsigned long previousMillis = 0;
unsigned long interval = 30000;

Network::Network(){}

void Network::connect()
{
    WiFi.begin(WIFI_SSID, WIFI_PWD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");
    ntp.start();
}

void Network::check()
{
    unsigned long currentMillis = millis();

    if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
        Serial.print(millis());
        Serial.println("Reconnecting to WiFi...");
        WiFi.disconnect();
        WiFi.reconnect();
        previousMillis = currentMillis;

        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println("\nWiFi connected.");
        ntp.start();
    }
}

String Network::ipAddress() {
    return WiFi.localIP().toString();
}