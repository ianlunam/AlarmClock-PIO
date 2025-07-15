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
    // // WiFi.scanNetworks will return the number of networks found.
    // int n = WiFi.scanNetworks();
    // Serial.println("Scan done");
    // if (n == 0) {
    //     Serial.println("no networks found");
    // } else {
    //     Serial.print(n);
    //     Serial.println(" networks found");
    //     Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
    //     for (int i = 0; i < n; ++i) {
    //     // Print SSID and RSSI for each network found
    //     Serial.printf("%2d", i + 1);
    //     Serial.print(" | ");
    //     Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
    //     Serial.print(" | ");
    //     Serial.printf("%4ld", WiFi.RSSI(i));
    //     Serial.print(" | ");
    //     Serial.printf("%2ld", WiFi.channel(i));
    //     Serial.print(" | ");
    //     switch (WiFi.encryptionType(i)) {
    //         case WIFI_AUTH_OPEN:            Serial.print("open"); break;
    //         case WIFI_AUTH_WEP:             Serial.print("WEP"); break;
    //         case WIFI_AUTH_WPA_PSK:         Serial.print("WPA"); break;
    //         case WIFI_AUTH_WPA2_PSK:        Serial.print("WPA2"); break;
    //         case WIFI_AUTH_WPA_WPA2_PSK:    Serial.print("WPA+WPA2"); break;
    //         case WIFI_AUTH_WPA2_ENTERPRISE: Serial.print("WPA2-EAP"); break;
    //         case WIFI_AUTH_WPA3_PSK:        Serial.print("WPA3"); break;
    //         case WIFI_AUTH_WPA2_WPA3_PSK:   Serial.print("WPA2+WPA3"); break;
    //         case WIFI_AUTH_WAPI_PSK:        Serial.print("WAPI"); break;
    //         default:                        Serial.print("unknown");
    //     }
    //     Serial.println();
    //     delay(10);
    //     }
    // }
    // Serial.println("");

    // // Delete the scan result to free memory for code below.
    // WiFi.scanDelete();

    // // Wait a bit before scanning again.
    // delay(5000);

    starting = true;

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
