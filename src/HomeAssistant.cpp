#include "Display.h"
#include <TFT_eSPI.h>
#include "HomeAssistant.h"
#include "WeatherIcons.h"
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

const String holiday_topic = "homeassistant/calendar/workday_sensor_nz_bop_calendar";
const String humidity_topic = "homeassistant/weather/forecast_home/humidity";
const String temperature_topic = "homeassistant/weather/forecast_home/temperature";
const String weather_topic = "homeassistant/weather/forecast_home/state";

TFT_eSprite *holidaySprite;
TFT_eSprite *humiditySprite;
TFT_eSprite *temperatureSprite;
TFT_eSprite *weatherSprite;

// The last value received for each topic, kept so the display can be fully
// redrawn from scratch once the alarm screen releases the display, without
// waiting for the next MQTT update to happen to arrive.
String lastWeatherCondition = "";
bool lastHolidayActive = false;
String lastTemperature = "";
String lastHumidity = "";


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
        lastWeatherCondition = sPayload;
        if (!alarmActive)
        {
            updateWeatherIcon(weatherSprite, sPayload, WEATHER_SPRITE_X, WEATHER_SPRITE_Y);
        }
    }
    else if (sTopic == holiday_topic)
    {
        if (sPayload == String("on"))
        {
            mqttAlarm.set_public_holiday(true);
            lastHolidayActive = true;
            if (!alarmActive)
            {
                updateSprite(holidaySprite, (char *)"H", HOLIDAY_SPRITE_X, HOLIDAY_SPRITE_Y);
            }
        }
        else if (sPayload == String("off"))
        {
            mqttAlarm.set_public_holiday(false);
            lastHolidayActive = false;
            if (!alarmActive)
            {
                updateSprite(holidaySprite, (char *)"", HOLIDAY_SPRITE_X, HOLIDAY_SPRITE_Y);
            }
        }
    }
    else if (sTopic == temperature_topic)
    {
        sPayload.concat("C");
        lastTemperature = sPayload;
        if (!alarmActive)
        {
            updateSprite(temperatureSprite, (char *)sPayload.c_str(), TEMPERATURE_SPRITE_X, TEMPERATURE_SPRITE_Y);
        }
    }
    else if (sTopic == humidity_topic)
    {
        sPayload.concat("%");
        lastHumidity = sPayload;
        if (!alarmActive)
        {
            updateSprite(humiditySprite, (char *)sPayload.c_str(), HUMIDITY_SPRITE_X, HUMIDITY_SPRITE_Y);
        }
    }
    else
    {
        Serial.println("Unknown topic");
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
        if (client.connect(client_id.c_str(), MQTT_USER, MQTT_PWD))
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
    TFT_eSPI &tft = mqttDisplay.get_tft();
    holidaySprite = new TFT_eSprite(&tft);
    temperatureSprite = new TFT_eSprite(&tft);
    humiditySprite = new TFT_eSprite(&tft);
    weatherSprite = new TFT_eSprite(&tft);

    initSprite(holidaySprite, 20, 20, HOLIDAY_SPRITE_X, HOLIDAY_SPRITE_Y);
    initSprite(humiditySprite, 80, 25, HUMIDITY_SPRITE_X, HUMIDITY_SPRITE_Y);
    initSprite(temperatureSprite, 80, 25, TEMPERATURE_SPRITE_X, TEMPERATURE_SPRITE_Y);
    initSprite(weatherSprite, 160, 25, WEATHER_SPRITE_X, WEATHER_SPRITE_Y);

    bool wasAlarmActive = false;
    for (;;)
    {
        if (alarmActive)
        {
            wasAlarmActive = true;
        }
        else if (wasAlarmActive)
        {
            // The alarm just released the display - redraw everything from
            // the last known values instead of waiting for the next MQTT
            // update, which might be minutes away.
            wasAlarmActive = false;
            if (lastWeatherCondition.length() > 0)
            {
                updateWeatherIcon(weatherSprite, lastWeatherCondition, WEATHER_SPRITE_X, WEATHER_SPRITE_Y);
            }
            updateSprite(holidaySprite, lastHolidayActive ? (char *)"H" : (char *)"", HOLIDAY_SPRITE_X, HOLIDAY_SPRITE_Y);
            if (lastTemperature.length() > 0)
            {
                updateSprite(temperatureSprite, (char *)lastTemperature.c_str(), TEMPERATURE_SPRITE_X, TEMPERATURE_SPRITE_Y);
            }
            if (lastHumidity.length() > 0)
            {
                updateSprite(humiditySprite, (char *)lastHumidity.c_str(), HUMIDITY_SPRITE_X, HUMIDITY_SPRITE_Y);
            }
        }

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

bool HomeAssistant::mqtt_status()
{
    return client.state();
}
