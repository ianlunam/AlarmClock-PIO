#include <Arduino.h>

// Different machines :)
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error "This ain't a ESP8266 or ESP32!"
#endif

#include <Preferences.h>
#include "Network.h"


Network::Network() {}

void Network::connect() {
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

String Network::ipAddress() {
  return WiFi.localIP().toString();
}
