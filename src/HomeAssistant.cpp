#include "Display.h"
#include <TFT_eSPI.h>
#include "HomeAssistant.h"
#include <iostream>
#include <string>
using namespace std;

#include <WiFi.h>
#include <PubSubClient.h>

// Wifi
WiFiClient espClient;
PubSubClient client(espClient);

Display mqttDisplay;
Alarm mqttAlarm;

const char *weather_topic = "homeassistant/weather/forecast_home_2/state";
const char *holiday_topic = "homeassistant/calendar/new_zealand_auk/state";
const char *temperature_topic = "homeassistant/sensor/t_h_sensor_temperature/state";
const char *humidity_topic = "homeassistant/sensor/t_h_sensor_humidity/state";

TFT_eSprite *weatherSprite;
TFT_eSprite *temperatureSprite;
TFT_eSprite *humiditySprite;

void temperature_sprite()
{
    temperatureSprite->createSprite(80, 25);
    temperatureSprite->setColorDepth(8);
    temperatureSprite->setFreeFont(&FreeSansBold12pt7b);
    temperatureSprite->setTextSize(1);
    temperatureSprite->fillSprite(BACKGROUND_COLOUR);
    temperatureSprite->pushSprite(10, 205);
}

void humidity_sprite()
{
    humiditySprite->createSprite(80, 25);
    humiditySprite->setColorDepth(8);
    humiditySprite->setFreeFont(&FreeSansBold12pt7b);
    humiditySprite->setTextSize(1);
    humiditySprite->fillSprite(BACKGROUND_COLOUR);
    humiditySprite->pushSprite(90, 205);
}

void weather_sprite()
{
    weatherSprite->createSprite(160, 25);
    weatherSprite->setColorDepth(8);
    weatherSprite->setFreeFont(&FreeSansBold12pt7b);
    weatherSprite->setTextSize(1);
    weatherSprite->fillSprite(BACKGROUND_COLOUR);
    weatherSprite->pushSprite(10, 175);
}

void callback(char *topic, byte *payload, unsigned int length)
{
    char value[length + 1];
    strncpy(value, (char *)payload, length);
    value[length] = '\0';

    if (strcmp(topic, weather_topic) == 0)
    {
        if (isalpha(value[0]))
        {
            value[0] = toupper(value[0]);
        }
        weatherSprite->fillSprite(BACKGROUND_COLOUR);
        weatherSprite->setTextColor((TEXT_R << (5 + 6)) | (TEXT_G << 5) | TEXT_B);
        weatherSprite->drawString(value, 0, 0);
        weatherSprite->pushSprite(10, 175);
    }
    else if (strcmp(topic, holiday_topic) == 0)
    {
        char on[] = "on";
        char off[] = "off";
        if (memcmp(payload, on, length) == 0)
        {
            mqttAlarm.set_public_holiday(true);
        }
        else if (memcmp(payload, off, length) == 0)
        {
            mqttAlarm.set_public_holiday(false);
        }
    }
    else if (strcmp(topic, temperature_topic) == 0)
    {
        char temp[length + 2];
        snprintf(temp, length + 2, "%sC", value);
        temperatureSprite->fillSprite(BACKGROUND_COLOUR);
        temperatureSprite->setTextColor((TEXT_R << (5 + 6)) | (TEXT_G << 5) | TEXT_B);
        temperatureSprite->drawString(temp, 0, 0);
        temperatureSprite->pushSprite(10, 205);
    }
    else if (strcmp(topic, humidity_topic) == 0)
    {
        char humid[length + 2];
        snprintf(humid, length + 2, "%s%%", value);
        humiditySprite->fillSprite(BACKGROUND_COLOUR);
        humiditySprite->setTextColor((TEXT_R << (5 + 6)) | (TEXT_G << 5) | TEXT_B);
        humiditySprite->drawString(humid, 0, 0);
        humiditySprite->pushSprite(90, 205);
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
    temperatureSprite = new TFT_eSprite(&tft);
    humiditySprite = new TFT_eSprite(&tft);
    weatherSprite = new TFT_eSprite(&tft);

    temperature_sprite();
    humidity_sprite();
    weather_sprite();

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

void HomeAssistant::start(Display &display, Alarm &alarm)
{
    mqttDisplay = display;
    mqttAlarm = alarm;
    xTaskCreate(get_mqtt, "Display MQTT Data", 4096, NULL, 10, NULL);
}
