#include <Display.h>
#include <TFT_eSPI.h>
#include <HomeAssistant.h>
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

const char *mqtt_broker = "mqtt.local";
const char *weather_topic = "homeassistant/weather/forecast_home_2/state";
const char *holiday_topic = "homeassistant/calendar/new_zealand_bop/state";
const char *temperature_topic = "homeassistant/sensor/t_h_sensor_temperature/state";
const char *humidity_topic = "homeassistant/sensor/t_h_sensor_humidity/state";
byte on_state[] = {'o','n'};
const int mqtt_port = 1883;

char temperature_now[20];
char temperature_then[20];
char humidity_now[20];
char humidity_then[20];
char weather_now[20];
char weather_then[20];

void temperature_sprite(void *pvParameters) {
    Serial.println("Starting temp loop");

    TFT_eSPI tft = mqttDisplay.get_tft();
    TFT_eSprite spr = TFT_eSprite(&tft);

    spr.setColorDepth(8);
    spr.setFreeFont(&FreeSansBold12pt7b);
    spr.setTextSize(1);

    spr.createSprite(100, 40);
    spr.fillSprite(TFT_BLACK);
    spr.setTextColor(TFT_DARKCYAN);
    spr.pushSprite(10, 200);

    for(;;) {
        if (strcmp(temperature_now, temperature_then) != 0) {
            Serial.printf("New temperature: %s\n", temperature_now);
            strncpy(temperature_then, temperature_now, strlen(temperature_now));
            spr.fillSprite(TFT_BLACK);
            char x[21];
            snprintf(x, 20, "%sC", temperature_now);

            spr.drawString(x, 0, 0);
            spr.pushSprite(10, 200);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

}

void humidity_sprite(void *pvParameters) {
    Serial.println("Starting humid loop");

    TFT_eSPI tft = mqttDisplay.get_tft();
    TFT_eSprite spr = TFT_eSprite(&tft);

    spr.setColorDepth(8);
    spr.setFreeFont(&FreeSansBold12pt7b);
    spr.setTextSize(1);

    spr.createSprite(100, 40);
    spr.fillSprite(TFT_BLACK);
    spr.setTextColor(TFT_DARKCYAN);
    spr.pushSprite(110, 200);

    for(;;) {
        if (strcmp(humidity_now, humidity_then) != 0) {
            Serial.printf("New humidity: %s\n", humidity_now);
            strncpy(humidity_then, humidity_now, strlen(humidity_now));
            spr.fillSprite(TFT_BLACK);
            char x[21];
            snprintf(x, 20, "%s%%", humidity_now);

            spr.drawString(x, 0, 0);
            spr.pushSprite(110, 200);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

}

void weather_sprite(void *pvParameters) {
    Serial.println("Starting weather loop");

    TFT_eSPI tft = mqttDisplay.get_tft();
    TFT_eSprite spr = TFT_eSprite(&tft);

    spr.setColorDepth(8);
    spr.setFreeFont(&FreeSansBold12pt7b);
    spr.setTextSize(1);

    spr.createSprite(100, 40);
    spr.fillSprite(TFT_BLACK);
    spr.setTextColor(TFT_DARKCYAN);
    spr.pushSprite(10, 170);

    for(;;) {
        if (strcmp(weather_now, weather_then) != 0) {
            Serial.printf("New weather: %s\n", weather_now);
            strncpy(weather_then, weather_now, strlen(weather_now));
            spr.fillSprite(TFT_BLACK);
            char x[21];
            snprintf(x, 20, "%s", weather_now);
            if (isalpha(x[0])) {
                x[0] = toupper(x[0]);
            }

            spr.drawString(x, 0, 0);
            spr.pushSprite(10, 170);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

}


void callback(char *topic, byte *payload, unsigned int length) {
    TFT_eSPI tft = mqttDisplay.get_tft();
    tft.setFreeFont(&FreeSerif18pt7b);
    tft.setTextSize(1);
    Serial.print("Topic: ");
    Serial.print(topic);
    Serial.print(" Length: ");
    Serial.println(length);

    if (strcmp(topic, weather_topic) == 0)
    {
        memcpy(weather_now, payload, length);
    }
    else if (strcmp(topic, holiday_topic) == 0)
    {
    }
    else if (strcmp(topic, temperature_topic) == 0)
    {
        memcpy(temperature_now, payload, length);
    }
    else if (strcmp(topic, humidity_topic) == 0)
    {
        memcpy(humidity_now, payload, length);
    }
    else
    {
        String content = "";
        for (size_t i = 0; i < length; i++)
        {
            content.concat((char)payload[i]);
        }

        Serial.printf("Dunno: '%s' '%s'\n", topic, content);
    }

}

void connect(){
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str())) {
            Serial.println("EMQX MQTT broker connected");
        } else {
            Serial.print("failed with state ");
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

    for(;;) {
        if (!client.connected()) {
            connect();
        }
        client.loop();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

HomeAssistant::HomeAssistant(){}

void HomeAssistant::start(Display &indisp, Alarm &inalarm)
{
    mqttDisplay = indisp;
    mqttAlarm = inalarm;
    xTaskCreate(get_mqtt, "Display MQTT Data", 4096, NULL, 10, NULL);
    xTaskCreate(temperature_sprite, "Temerature", 4096, NULL, 10, NULL);
    xTaskCreate(humidity_sprite, "Humidity", 4096, NULL, 10, NULL);
    xTaskCreate(weather_sprite, "Weather", 4096, NULL, 10, NULL);
}
