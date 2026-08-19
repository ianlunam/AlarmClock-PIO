#include <WiFi.h>
#include "Network.h"
#include "icons.h"

bool starting = false;

TFT_eSprite *wifiSprite;

Network::Network() {}

void restart(WiFiEvent_t event, WiFiEventInfo_t info)
{
    if (starting) { return; }
    Serial.println("WiFi Down");
    wifiSprite->fillSprite(BACKGROUND_COLOUR);
    wifiSprite->pushImage(0, 0, 25, 25, wifi_off);
    wifiSprite->pushSprite(wifiX, wifiY);
    starting = true;
    WiFi.reconnect();
    delay(1000);
    starting = false;
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
    starting = true;

    TFT_eSPI &tft = display.get_tft();
    wifiSprite = new TFT_eSprite(&tft);

    wifiSprite->createSprite(25, 25);
    wifiSprite->setColorDepth(8);
    wifiSprite->setSwapBytes(true);
    wifiSprite->fillSprite(BACKGROUND_COLOUR);
    wifiSprite->pushImage(0, 0, 25, 25, wifi_off);
    wifiSprite->pushSprite(wifiX, wifiY);

    WiFi.onEvent(restart, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(started, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    Serial.print("Starting WiFi ");
    WiFi.begin(WIFI_SSID, WIFI_PWD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" Connected");
    starting = false;
}
