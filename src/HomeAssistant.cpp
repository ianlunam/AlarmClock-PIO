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

const char *holiday_topic = "homeassistant/calendar/new_zealand_auk/state";
const char *humidity_topic = "homeassistant/sensor/t_h_sensor_humidity/state";
const char *temperature_topic = "homeassistant/sensor/t_h_sensor_temperature/state";
const char *weather_topic = "homeassistant/weather/forecast_harrisfield/state";

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
    char value[length + 1];
    strncpy(value, reinterpret_cast<char *>(payload), length);
    value[length] = '\0';

    if (strcmp(topic, weather_topic) == 0)
    {
        if (isalpha(value[0]))
        {
            value[0] = toupper(value[0]);
        }
        updateSprite(weatherSprite, value, WEATHER_SPRITE_X, WEATHER_SPRITE_Y);
    }
    else if (strcmp(topic, holiday_topic) == 0)
    {
        const char on[] = "on";
        const char off[] = "off";
        if (memcmp(payload, on, length) == 0)
        {
            mqttAlarm.set_public_holiday(true);
            char hach[] = "H";
            updateSprite(holidaySprite, hach, HOLIDAY_SPRITE_X, HOLIDAY_SPRITE_Y);
        }
        else if (memcmp(payload, off, length) == 0)
        {
            mqttAlarm.set_public_holiday(false);
            char nout[] = "";
            updateSprite(holidaySprite, nout, HOLIDAY_SPRITE_X, HOLIDAY_SPRITE_Y);
        }
    }
    else if (strcmp(topic, temperature_topic) == 0)
    {
        char temp[length + 2];
        snprintf(temp, length + 2, "%sC", value);
        updateSprite(temperatureSprite, temp, TEMPERATURE_SPRITE_X, TEMPERATURE_SPRITE_Y);
    }
    else if (strcmp(topic, humidity_topic) == 0)
    {
        char humid[length + 2];
        snprintf(humid, length + 2, "%s%%", value);
        updateSprite(humiditySprite, humid, HUMIDITY_SPRITE_X, HUMIDITY_SPRITE_Y);
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
    client.subscribe(weather_topic);
    client.subscribe(holiday_topic);
    client.subscribe(temperature_topic);
    client.subscribe(humidity_topic);
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
