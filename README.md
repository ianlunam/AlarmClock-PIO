# CYD Alarm Clock

A bedside alarm clock built on a "Cheap Yellow Display" (CYD ESP32-2432S028)
board: a 320x240 touchscreen ESP32 dev board. It shows the time, day and
date, and pulls weather/temperature/humidity/public-holiday data from Home
Assistant over MQTT. Alarms are configured through a small web page served
by the device itself, and dismissed by tapping the screen when they ring.

## Features

- Large clock display with day-of-week and date
- Weather condition icon, temperature and humidity, pulled from Home
  Assistant over MQTT and drawn as small vector icons (sun, cloud, rain,
  snow, etc. - see `src/WeatherIcons.cpp`)
- Public holiday awareness (also via MQTT) - alarms can be configured to
  skip public holidays
- Backlight auto-dims in the dark using the onboard LDR, with hysteresis so
  it doesn't flicker when the light level sits right on the threshold
- WiFi connection status icon
- Touchscreen alarm dismissal - tap the on-screen button to stop the alarm
- Web page (served by the device) for adding/editing/deleting alarms, with
  per-day-of-week scheduling, "skip public holidays", and "once only"
  options
- OTA firmware updates over WiFi, once the device is already running
  OTA-enabled firmware (see [OTA updates](#ota-updates) below)

## Hardware

- Board: CYD ESP32-2432S028 (ESP32 + 320x240 ILI9341 TFT + XPT2046
  resistive touch, both over SPI)
- Speaker wired to the `SPEAK` pin (GPIO26) through a ~400 ohm resistor to
  keep the volume down
- Onboard RGB LED exists but isn't currently used for anything (turned off
  at boot)
- Current screen layout (coordinates, sprite sizes) is documented in
  [DisplayLayout](DisplayLayout)

## Getting started

### Prerequisites

- [PlatformIO Core](https://platformio.org/) (`pio` on your `PATH`)
- A `.secrets` file **one directory above this project** (i.e. `../secrets`
  relative to this repo) containing:

  ```
  WIFI_SSID your-wifi-name
  WIFI_PWD your-wifi-password
  MQTT_USER your-mqtt-username
  MQTT_PWD your-mqtt-password
  ```

  `platformio.ini` reads these at build time and bakes them into the
  firmware as build flags - the file itself is never committed.

### Building, flashing, testing

All the common workflows are wrapped in the `Makefile` - run `make` or
`make help` to list them:

```
make build         # compile the firmware
make upload        # build and flash over USB (needed for the very first flash)
make upload-ota    # build and flash over WiFi (see OTA updates below)
make test          # pure-logic unit tests, run on your own machine
make hardware-test # test suite that runs on the real device over USB
make monitor       # open the serial monitor
make clean         # remove build artifacts
```

These are thin wrappers around the underlying `pio` commands in
`platformio.ini`, which is also where all the pin assignments and tunable
constants live (touchscreen/display pins, LDR thresholds, backlight
min/max PWM, speaker pin, timezone, NTP/MQTT server, etc).

### Home Assistant / MQTT

The device subscribes to these topics (see `src/HomeAssistant.cpp` to
change them):

| Topic | Used for |
|---|---|
| `homeassistant/weather/forecast_home/state` | weather condition icon |
| `homeassistant/weather/forecast_home/temperature` | temperature display |
| `homeassistant/weather/forecast_home/humidity` | humidity display |
| `homeassistant/calendar/workday_sensor_nz_bop_calendar` | public holiday flag (`on`/`off`) |

The MQTT broker address is set via `MQTT_BROKER`/`MQTT_PORT` in
`platformio.ini`; credentials come from `.secrets` as above.

### Web interface

The device serves a small configuration page on port 80:

- `/` - list of configured alarms
- `/new`, `/edit?name=...` - add/edit an alarm (time, days of week, skip
  public holidays, once-only, enabled)
- `/save` - saves the form from `/new` or `/edit`
- `/delete?name=...` - removes an alarm

Up to 6 alarms can be stored at once (see `alarmList` in `src/Alarm.cpp`).

### OTA updates

The first flash has to be over USB (`make upload`) - the device can't
receive an OTA push before it's running firmware that listens for one.
After that, `make upload-ota` (or `pio run -e esp32dev_ota -t upload`)
pushes new firmware over WiFi to `newalarmclock.local`, using the same
build as the USB path - `esp32dev_ota` differs only in *how* the result
gets to the device, not what gets built.

## Testing

There are two independent test suites, covering different things - see
[test/README](test/README) for the full picture:

- **`make test`** - pure decision logic (does this alarm fire right now,
  the backlight hysteresis, weather-condition-to-icon mapping) runs
  natively on your own machine in seconds, no hardware involved.
- **`make hardware-test`** - things that need the real device: an actual
  sensor reading, a real flash read/write round-trip, and a concurrency
  test that pins two tasks to different CPU cores to check a mutex holds
  up under genuine parallel access.

Everything else (display rendering, touch, WiFi, MQTT) is verified by
actually running the device rather than mocked - see test/README for why.

## Project layout

- `src/`, `include/` - firmware source, roughly one module per concern
  (`Display`, `Clock`, `Ldr`, `Alarm`/`AlarmLogic`, `WebServer`,
  `Configurator`, `HomeAssistant`, `WeatherIcons`/`WeatherIconShape`,
  `Network`, `Screamer`)
- `test/` - both test suites described above
- `platformio.ini` - environments (`esp32dev`, `esp32dev_ota`, `native`,
  `esp32dev_test`), pin assignments, and tunable constants
- `Makefile` - convenience wrappers around the `pio` commands above
- `DisplayLayout` - current screen layout reference

## Things worth knowing

- **Touch calibration is per physical panel.** The raw touchscreen ADC
  range in `src/Alarm.cpp` (`TOUCH_RAW_X/Y_MIN/MAX`) was measured against
  one specific device. If touches land in the wrong place on different
  hardware, that file's comments explain how to re-calibrate.
- **The DST correction assumes a one-hour offset**, matching
  `TIMEZONE` in `platformio.ini`. It's covered by `test/test_alarm_logic`,
  but a timezone with a different DST offset would need that logic
  revisited.
- Several tasks share the display and touch hardware; `DisplayLock` and
  `AlarmStoreLock` (see `include/Display.h` / `include/Alarm.h`) exist to
  keep concurrent access to the screen and to saved alarm data safe -
  worth understanding before adding a new task that draws to the screen
  or reads/writes alarms.

## Possible future hardware change: I2S audio

Currently uses a simple `ledcWriteNote` tone through the `SPEAK` pin.
Moving to an I2S DAC (e.g. MAX98357A) would need three GPIO pins plus 3V3
and GND. On this board's extended IO headers:

- P3: GND, GPIO35, GPIO22, GPIO21
- CN1: GND, GPIO22, GPIO27, 3V3
- GPIO21 is already used for the backlight, so it's not available
- GPIO22 is present on both connectors

That leaves GPIO35, GPIO22, and GPIO27 available. The existing `SPEAK`
pin (GPIO26) could potentially be reused instead of GPIO35, needing only
one 4-pin and one 2-pin JST connector rather than two 4-pin ones.
