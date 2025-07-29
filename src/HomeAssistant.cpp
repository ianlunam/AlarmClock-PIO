#include "Display.h"
#include <TFT_eSPI.h>
#include "HomeAssistant.h"
#include <iostream>
#include <string>
using namespace std;

#include <WiFi.h>
#include <PubSubClient.h>

#define HOLIDAY_SPRITE_X 275
#define HOLIDAY_SPRITE_Y 5
#define HUMIDITY_SPRITE_X 90
#define HUMIDITY_SPRITE_Y 205
#define TEMPERATURE_SPRITE_X 10
#define TEMPERATURE_SPRITE_Y 205
#define WEATHER_SPRITE_X 10
#define WEATHER_SPRITE_Y 175

// Wifi
WiFiClient espClient;
PubSubClient client(espClient);

Display mqttDisplay;
Alarm mqttAlarm;

const String holiday_topic = "homeassistant/calendar/new_zealand_auk/state";
const String humidity_topic = "homeassistant/sensor/t_h_sensor_humidity/state";
const String temperature_topic = "homeassistant/sensor/t_h_sensor_temperature/state";
const String weather_topic = "homeassistant/weather/forecast_harrisfield/state";

TFT_eSprite *holidaySprite;
TFT_eSprite *humiditySprite;
TFT_eSprite *temperatureSprite;
TFT_eSprite *weatherSprite;


void initSprite(TFT_eSprite *sprite, uint16_t width, uint16_t height,  uint16_t x, uint16_t y) {
    sprite->createSprite(width, height);
    sprite->setColorDepth(8);
    sprite->setFreeFont(&FreeSansBold12pt7b);
    sprite->setTextSize(1);
    sprite->fillSprite(BACKGROUND_COLOUR);
    sprite->pushSprite(x, y);
}

void updateSprite(TFT_eSprite *sprite, char value[], uint16_t x, uint16_t y)
{
    sprite->fillSprite(BACKGROUND_COLOUR);
    sprite->setTextColor((TEXT_R << (5 + 6)) | (TEXT_G << 5) | TEXT_B);
    sprite->drawString(value, 0, 0);
    sprite->pushSprite(x, y);
}

void callback(char *topic, byte *payload, unsigned int length)
{
    String sTopic(topic);
    String sPayload(payload, length);
    Serial.print(sTopic);
    Serial.print(" - ");
    Serial.println(sPayload);

    if (sTopic == weather_topic)
    {
        if (isalpha(sPayload[0]))
        {
            sPayload[0] = toupper(sPayload[0]);
        }
        updateSprite(weatherSprite, (char *)sPayload.c_str(), WEATHER_SPRITE_X, WEATHER_SPRITE_Y);
    }
    else if (sTopic == holiday_topic)
    {
        if (sPayload == String("on"))
        {
            mqttAlarm.set_public_holiday(true);
            updateSprite(holidaySprite, (char *)"H", HOLIDAY_SPRITE_X, HOLIDAY_SPRITE_Y);
        }
        else if (sPayload == String("off"))
        {
            mqttAlarm.set_public_holiday(false);
            updateSprite(holidaySprite, (char *)"", HOLIDAY_SPRITE_X, HOLIDAY_SPRITE_Y);
        }
    }
    else if (sTopic == temperature_topic)
    {
        sPayload.concat("C");
        updateSprite(temperatureSprite, (char *)sPayload.c_str(), TEMPERATURE_SPRITE_X, TEMPERATURE_SPRITE_Y);
    }
    else if (sTopic == humidity_topic)
    {
        sPayload.concat("%");
        updateSprite(humiditySprite, (char *)sPayload.c_str(), HUMIDITY_SPRITE_X, HUMIDITY_SPRITE_Y);
    }
}

void connect()
{
    client.setServer(MQTT_BROKER, MQTT_PORT);
    client.setCallback(callback);
    while (!client.connected())
    {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        if (client.connect(client_id.c_str()))
        {
            Serial.println("MQTT started");
        }
        else
        {
            Serial.print("MQTT broker connect failed with state ");
            Serial.println(client.state());
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
    client.subscribe(weather_topic.c_str());
    client.subscribe(holiday_topic.c_str());
    client.subscribe(temperature_topic.c_str());
    client.subscribe(humidity_topic.c_str());
}

void get_mqtt(void *pvParameters)
{
    TFT_eSPI tft = mqttDisplay.get_tft();
    holidaySprite = new TFT_eSprite(&tft);
    temperatureSprite = new TFT_eSprite(&tft);
    humiditySprite = new TFT_eSprite(&tft);
    weatherSprite = new TFT_eSprite(&tft);

    initSprite(holidaySprite, 20, 20, HOLIDAY_SPRITE_X, HOLIDAY_SPRITE_Y);
    initSprite(humiditySprite, 80, 25, HUMIDITY_SPRITE_X, HUMIDITY_SPRITE_Y);
    initSprite(temperatureSprite, 80, 25, TEMPERATURE_SPRITE_X, TEMPERATURE_SPRITE_Y);
    initSprite(weatherSprite, 160, 25, WEATHER_SPRITE_X, WEATHER_SPRITE_Y);

    for (;;)
    {
        if (!client.connected())
        {
            connect();
        }
        client.loop();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

HomeAssistant::HomeAssistant() {}

void HomeAssistant::start(const Display &display, const Alarm &alarm)
{
    mqttDisplay = display;
    mqttAlarm = alarm;
    xTaskCreate(get_mqtt, "Display MQTT Data", 4096, NULL, 10, NULL);
}
