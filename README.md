# CYD (cheap yellow display) Alarm Clock

- Programmed using PlatformIO
- Making use of esp32 FreeRTOS
- Web interface for settings
- MQTT interface to Home Assistant for holidays, weather, temperature and humidity
- LDR used to dim screen
- WiFi (dis)connected icon
- Basic alarm tone
- Currently left half of touchscreen for off, right for snooze
- Speaker connected to SPEAK with ~400 ohm resister to reduce volume

## Settings

- See `platformio.ini` for most things

## Outstanding issues

- I'm still a cpp beginner
- Display stop / snooze buttons, and maybe snooze countdown
- Backlight goes off when tone ends (goes on and off during alarm), currently forcing back on after alarm

## Move from `tone` to max98357a and I2C

- Requires three GPIO pins, plus 3V3 and GND

- Extended IO provides three across two separate connectors

    - P3 has GND, GPIO 35, GPIO 22 and GPIO 21
    - CN1 has GND, GPIO 22, GPIO 27 and 3V3
    - GPIO is the backlight control, so can't be used
    - GPIO 22 is on both connectors

- That leaves 35, 22 and 27
- SPEAK connector is pin 26. Might try to use that instead of 35 so I can just use one 4 pin and one 2 pin JSTs, instead of 2x4 pin JSTs which I don't have laying around
