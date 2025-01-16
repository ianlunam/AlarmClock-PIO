#include "Display.h"
#include <TFT_eSPI.h>
#include "HomeAssistant.h"

#include <WiFi.h>
#include <PubSubClient.h>

// Wifi
WiFiClient espClient;
PubSubClient client(espClient);

Display mqttDisplay;

const char *mqtt_broker = "mqtt.local";
const char *weather_topic = "homeassistant/weather/forecast_home_2/state";
const char *holiday_topic = "homeassistant/calendar/new_zealand_bop/state";
const char *temperature_topic = "homeassistant/sensor/t_h_sensor_temperature/state";
const char *humidity_topic = "homeassistant/sensor/t_h_sensor_humidity/state";
byte on_state[] = {'o','n'};
const int mqtt_port = 1883;


void callback(char *topic, byte *payload, unsigned int length) {
    TFT_eSPI tft = mqttDisplay.get_tft();
    tft.setFreeFont(&FreeSerif18pt7b);
    tft.setTextSize(1);
    Serial.print("Topic: ");
    Serial.println(topic);
    Serial.print("Length: ");
    Serial.println(length);

    String content = "";
    for (size_t i = 0; i < length; i++)
    {
        content.concat((char)payload[i]);
    }
    Serial.println(content);

    if (strcmp(topic, weather_topic))
    {
    }
    else if (strcmp(topic, holiday_topic))
    {
    }
    else if (strcmp(topic, temperature_topic))
    {
    }
    else if (strcmp(topic, humidity_topic))
    {
    }
    else
    {
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

void HomeAssistant::start(Display &indisp)
{
    mqttDisplay = indisp;
    xTaskCreate(get_mqtt, "Display MQTT Data", 4096, NULL, 10, NULL);
}
