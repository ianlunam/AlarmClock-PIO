#include <Arduino.h>

#include <WiFi.h>
#include "Network.h"
#include "icons.h"

TFT_eSprite *wifiSprite;

Network::Network() {}

void restart(WiFiEvent_t event, WiFiEventInfo_t info)
{
    wifiSprite->fillSprite(BACKGROUND_COLOUR);
    wifiSprite->pushImage(0, 0, 25, 25, wifi_off);
    wifiSprite->pushSprite(wifiX, wifiY);
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
}

void started(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("WiFi Up");
    wifiSprite->fillSprite(BACKGROUND_COLOUR);
    wifiSprite->pushImage(0, 0, 25, 25, wifi);
    wifiSprite->pushSprite(wifiX, wifiY);

    configTzTime(TIMEZONE, NTP_SERVER);
}

void Network::start(Display &display)
{
    TFT_eSPI tft = display.get_tft();
    wifiSprite = new TFT_eSprite(&tft);

    wifiSprite->createSprite(25, 25);
    wifiSprite->setColorDepth(8);
    wifiSprite->setSwapBytes(true);
    wifiSprite->fillSprite(BACKGROUND_COLOUR);
    wifiSprite->pushImage(0, 0, 25, 25, wifi_off);
    wifiSprite->pushSprite(wifiX, wifiY);
    WiFi.onEvent(restart, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(started, ARDUINO_EVENT_WIFI_STA_CONNECTED);

    WiFi.begin(WIFI_SSID, WIFI_PWD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
}
