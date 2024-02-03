#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <time.h>

#include "Display.h"

#define SCREEN_WIDTH 160  // OLED display width, in pixels
#define SCREEN_HEIGHT 128   // OLED display height, in pixels

TFT_eSPI tft = TFT_eSPI();    // Invoke library, pins defined in User_Setup.h

// Backlight
uint8_t bl_pin = 25;
uint8_t backlight = 220;
uint8_t BL_MIN = 202;
uint8_t BL_MAX = 220;

// Clock
uint8_t hh, mm, ss;    // Get H, M, S from compile time
struct tm _timeinfo;
uint32_t targetTime = 0;             // for next 1 second timeout
byte omm = 99;
bool initial = 1;
byte xcolon = 0;

String zoneData[5] = { "", "", "", "", "" };

void setBacklight(int8_t value) {
    if (backlight != value) {
        backlight = value;
        dacWrite(bl_pin, backlight);
    }
}

Display::Display() {}

void Display::init() {
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
}

void Display::setString(int zone, String value) {
    tft.setTextSize(1);
    if (zone == 2 && value != zoneData[zone]) {
    Serial.print(zone);
    Serial.print(": ");
    Serial.println(value);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setCursor (0, 120);
        tft.print("                ");
        tft.setCursor (0, 120);
        tft.print(value);
        zoneData[zone] = value;
    }
    if (zone == 3 && value != zoneData[zone]) {
    Serial.print(zone);
    Serial.print(": ");
    Serial.println(value);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setCursor (130, 110);
        tft.print(value);
        zoneData[zone] = value;
    }
    if (zone == 4 && value != zoneData[zone]) {
    Serial.print(zone);
    Serial.print(": ");
    Serial.println(value);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setCursor (140, 120);
        tft.print(value);
        zoneData[zone] = value;
    }
//   if (zoneData[zone] != value) {
//     zoneData[zone] = value;
//     updateDisplay();
//   }
}


void Display::updateTime() {
    getLocalTime(&_timeinfo);
    ss = _timeinfo.tm_sec;
    mm = _timeinfo.tm_min;
    hh = _timeinfo.tm_hour;
    
    if (targetTime < millis()) {
        targetTime = millis()+1000;

        // Update digital time
        byte xpos = 6;
        byte ypos = 30;

        if (ss==0 || initial) {
            initial = 0;
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.setCursor (0, 110);

            char ptr[20];
            int rc = strftime(ptr, 20, "%a %e %b", &_timeinfo);
            tft.print(ptr);
        }


        if (omm != mm) { // Only redraw every minute to minimise flicker
            // Uncomment ONE of the next 2 lines, using the ghost image demonstrates text overlay as time is drawn over it
            // tft.setTextColor(0x39C4, TFT_BLACK);    // Leave a 7 segment ghost image, comment out next line!
            tft.setTextColor(TFT_BLACK, TFT_BLACK); // Set font colour to black to wipe image
            // Font 7 is to show a pseudo 7 segment display.
            // Font 7 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 0 : .
            tft.drawString("88:88",xpos,ypos,7); // Overwrite the text to clear it
            // tft.setTextColor(0xFBE0); // Orange
            tft.setTextColor(0xFBE0); // BluelocalName
            omm = mm;

            if (hh<10) xpos+= tft.drawChar('0',xpos,ypos,7);
            xpos+= tft.drawNumber(hh,xpos,ypos,7);
            xcolon=xpos;
            xpos+= tft.drawChar(':',xpos,ypos,7);
            if (mm<10) xpos+= tft.drawChar('0',xpos,ypos,7);
            tft.drawNumber(mm,xpos,ypos,7);

            if ((hh >= 21 || hh <= 6)) {
                setBacklight(BL_MIN);
            } else {
                setBacklight(BL_MAX);
            }
        }

        if (ss%2) { // Flash the colon
            tft.setTextColor(0x39C4, TFT_BLACK);
            xpos+= tft.drawChar(':',xcolon,ypos,7);
        } else {
            tft.setTextColor(0xFBE0, TFT_BLACK);
            tft.drawChar(':',xcolon,ypos,7);
        }
    }
}