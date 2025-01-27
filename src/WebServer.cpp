#include <Arduino.h>
#include <AsyncTCP.h>

#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <ArduinoOTA.h>

#include "Alarm.h"
#include "Configurator.h"
#include "WebServer.h"

AsyncWebServer server(80);
Configurator configurator;

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}


String alarmToTableRow(AlarmEntry &thisAlarm) {
    String output = "<tr><td>";
    output.concat(thisAlarm.name);
    output.concat("</td><td>");
    if (thisAlarm.hour < 10) output.concat("0");
        output.concat(thisAlarm.hour);
        output.concat(":");
    if (thisAlarm.minute < 10) output.concat("0");
        output.concat(thisAlarm.minute);
        output.concat("</td><td>");
    if (thisAlarm.sunday) {
            output.concat("S");
    } else {
            output.concat(".");
    }
        output.concat("</td><td>");
    if (thisAlarm.monday) {
            output.concat("M");
    } else {
            output.concat(".");
    }
    output.concat("</td><td>");
    if (thisAlarm.tuesday) {
        output.concat("T");
    } else {
        output.concat(".");
    }
    output.concat("</td><td>");
    if (thisAlarm.wednesday) {
        output.concat("W");
    } else {
        output.concat(".");
    }
    output.concat("</td><td>");
    if (thisAlarm.thursday) {
        output.concat("T");
    } else {
        output.concat(".");
    }
    output.concat("</td><td>");
    if (thisAlarm.friday) {
        output.concat("F");
    } else {
        output.concat(".");
    }
    output.concat("</td><td>");
    if (thisAlarm.saturday) {
        output.concat("S");
    } else {
        output.concat(".");
    }
    output.concat("</td><td style=\"text-align:center\">");
    if (thisAlarm.skip_phols) {
        output.concat("Y");
    } else {
        output.concat("N");
    }
    output.concat("</td><td style=\"text-align:center\">");
    if (thisAlarm.once) {
        output.concat("Y");
    } else {
        output.concat("N");
    }
    output.concat("</td><td style=\"text-align:center\">");
    if (thisAlarm.enabled) {
        output.concat("Y");
    } else {
        output.concat("N");
    }
    output.concat("</td><td><a href=\"/edit?name=");
    output.concat(thisAlarm.name);
    output.concat("\">Edit</a> or <a href=\"/delete?name=");
    output.concat(thisAlarm.name);
    output.concat("\">Delete</a></td></tr>\n");
    return output;
}


bool getCurrentAlarm(char *name, AlarmEntry &newAlarm) {

    Preferences alarmStore;
    alarmStore.begin("alarmStore", true);

    if (alarmStore.isKey(name)) {
        int size = alarmStore.getBytesLength(name);
        if (size > 0) {
        char *buf[size + 1];
        int result = alarmStore.getBytes(name, &buf, size);
        memcpy(&newAlarm, buf, size);
        alarmStore.end();
        return true;
        }
    }
    alarmStore.end();
    return false;
}


void handleEdit(AsyncWebServerRequest *request) {
    if (!request->hasArg("name")) {
        request->send(400, "text/html", "<html><p>Missing parameter: name</p></html>");
        return;
    }

    AlarmEntry alarmEntry;
    if (getCurrentAlarm((char *)request->getParam("name")->value().c_str(), alarmEntry)) {
        request->send(200, "text/html", configurator.editPage(alarmEntry));
    } else {
        request->send(404, "text/plain", request->getParam("name")->value() + " doesn't exist.");
    }
}


void handleNew(AsyncWebServerRequest *request) {
    request->send(200, "text/html", configurator.newPage());
}


void handleSave(AsyncWebServerRequest *request) {
    if (!request->hasArg("name")) {
        request->send(400, "text/html", "<html><p>Missing parameter: name</p></html>");
        return;
    }

    AlarmEntry alarmEntry = { "", 0, 0, false, false, false, false, false, false, false, false, false, false };
    Preferences preferences;
    strcpy(alarmEntry.name, request->getParam("name")->value().c_str());
    alarmEntry.hour = request->getParam("hour")->value().toInt();
    alarmEntry.minute = request->getParam("minute")->value().toInt();
    if (request->hasArg("sunday")) { alarmEntry.sunday = true; }
    if (request->hasArg("monday")) { alarmEntry.monday = true; }
    if (request->hasArg("tuesday")) { alarmEntry.tuesday = true; }
    if (request->hasArg("wednesday")) { alarmEntry.wednesday = true; }
    if (request->hasArg("thursday")) { alarmEntry.thursday = true; }
    if (request->hasArg("friday")) { alarmEntry.friday = true; }
    if (request->hasArg("saturday")) { alarmEntry.saturday = true; }
    if (request->hasArg("skip_phols")) { alarmEntry.skip_phols = true; }
    if (request->hasArg("once")) { alarmEntry.once = true; }
    if (request->hasArg("enabled")) { alarmEntry.enabled = true; }
    preferences.begin("alarmStore", false);
    preferences.putBytes(alarmEntry.name, &alarmEntry, sizeof(alarmEntry));

    char alarmList[][20] = { "", "", "", "", "", "" };
    if (preferences.isKey("alarms")) {
        int size = preferences.getBytesLength("alarms");
        if (size > 0) {
        char *buf[size + 1];
        int result = preferences.getBytes("alarms", &buf, size);
        memcpy(&alarmList, buf, size);
        }
    } else {
        for (int x = 0; x < 6; x++) {
        strcpy(alarmList[x], "");
        }
    }
    bool found = false;
    int freePos = -1;
    for (int x = 0; x < 6; x++) {
        if (strcmp(alarmList[x], request->getParam("name")->value().c_str()) == 0) {
        found = true;
        } else if (strlen(alarmList[x]) == 0 && freePos == -1) {
        freePos = x;
        }
    }
    if (!found) {
        if (freePos > -1) {
        strcpy(alarmList[freePos], request->getParam("name")->value().c_str());
        }
    }
    preferences.putBytes("alarms", &alarmList, sizeof(alarmList));

    preferences.end();

    request->redirect("/");
}


void handleDelete(AsyncWebServerRequest *request) {
    if (!request->hasArg("name")) {
        request->send(400, "text/html", "<html><p>Missing parameter: name</p></html>");
        return;
    }

    char alarmList[][20] = { "", "", "", "", "", "" };
    Preferences preferences;

    preferences.begin("alarmStore", false);
    if (preferences.isKey("alarms")) {
        int size = preferences.getBytesLength("alarms");
        if (size > 0) {
        char *buf[size + 1];
        int result = preferences.getBytes("alarms", &buf, size);
        memcpy(&alarmList, buf, size);
        }
    } else {
        for (int x = 0; x < 6; x++) {
        strcpy(alarmList[x], "");
        }
    }
    for (int x = 0; x < 6; x++) {
        if (strcmp(alarmList[x], request->getParam("name")->value().c_str()) == 0) {
        strcpy(alarmList[x], "");
        }
    }
    preferences.putBytes("alarms", &alarmList, sizeof(alarmList));
    preferences.remove(request->getParam("name")->value().c_str());
    preferences.end();

    request->redirect("/");
}


void handleRoot(AsyncWebServerRequest *request) {
    char alarmList[][20] = { "", "", "", "", "", "" };
    Preferences preferences;

    preferences.begin("alarmStore", false);
    if (preferences.isKey("alarms")) {
        int size = preferences.getBytesLength("alarms");
        if (size > 0) {
        char *buf[size + 1];
        int result = preferences.getBytes("alarms", &buf, size);
        memcpy(&alarmList, buf, size);
        }
    } else {
        for (int x = 0; x < 6; x++) {
        strcpy(alarmList[x], "");
        }
        preferences.putBytes("alarms", &alarmList, sizeof(alarmList));
    }
    preferences.end();

    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("<!DOCTYPE html><html><head><title>Current Alarms</title><style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; } table, th, td { border: 1px solid black; padding: 5px;}</style></head><body>\n");
    response->print("<h1>Hello from the Alarm Clock!</h1>\n");
    response->print("<p>These are the current alarms set.</p>\n");
    response->print("<table>\n");
    response->print("<tr><th>Name</th><th>Time</th><th>&nbsp</th><th>&nbsp</th><th>&nbsp</th><th>&nbsp</th><th>&nbsp</th><th>&nbsp</th><th>&nbsp</th><th>Skip Holidays</th><th>Once Only</th><th>Enabled</th><th>Change</th></tr>\n");
    for (int x = 0; x < 6; x++) {
        AlarmEntry myAlarm;
        if (getCurrentAlarm(alarmList[x], myAlarm)) {
        response->print(alarmToTableRow(myAlarm));
        }
    }
    response->print("</table><br/>\n");
    response->print("<a href=\"/new\">Create New</a>\n");
    response->print("</body></html>");
    request->send(response);
};



WebServer::WebServer(){}

void WebServer::start()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        handleRoot(request);
    });
    server.on("/edit", HTTP_GET, [](AsyncWebServerRequest *request) {
        handleEdit(request);
    });
    server.on("/new", HTTP_GET, [](AsyncWebServerRequest *request) {
        handleNew(request);
    });
    server.on("/save", HTTP_GET, [](AsyncWebServerRequest *request) {
        handleSave(request);
    });
    server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
        handleDelete(request);
    });

    ArduinoOTA
        .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else  // U_SPIFFS
            type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
        })
        .onEnd([]() {
        Serial.println("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });

    ArduinoOTA.setHostname("newalarmclock");
    ArduinoOTA.begin();

    server.onNotFound(notFound);
    server.begin();

}
