# Nano 33 IoT Home Assistant MQTT

This sketch exposes a relay and an analog voltage sensor from an Arduino Nano 33 IoT to Home Assistant using MQTT discovery.

## Hardware
- Board: Arduino Nano 33 IoT (SAMD21)
- Relay control pin: D7 (active HIGH)
- Analog input: A0 (voltage divider expected)

## Firmware configuration
- Wi-Fi SSID/password and the MQTT broker IP are hardcoded near the top of `src/main.cpp`. Update them before flashing.
- MQTT topics (as used in Home Assistant discovery):
  - Availability: `nano33/availability`
  - Relay command/state: `nano33/relay/command` / `nano33/relay/state`
  - Analog sensor state: `nano33/analog/state`
  - Discovery topics: `homeassistant/switch/nano33_relay/config`, `homeassistant/sensor/nano33_analog/config`
- Discovery payloads include QoS 0 and `retain` set to true. Relay payloads: `ON` / `OFF`.
- Analog voltage is computed as `analogRead(A0) * (5.0 / 1023.0)`; adjust the constant if your reference or divider differs.

## Building and uploading
1) Install the PlatformIO extension in VS Code.
2) Dependencies are declared in `platformio.ini` (WiFiNINA, PubSubClient, ArduinoJson); run "PlatformIO: Build" to fetch them.
3) Connect the Nano 33 IoT via USB and run "PlatformIO: Upload".
4) Open the serial monitor at 115200 baud to verify Wi-Fi and MQTT connectivity.

## Home Assistant
- With MQTT discovery enabled, entities should appear automatically after the device comes online and publishes discovery messages.
- Ensure your broker allows the device to publish/subscribe on the topics above.

## Troubleshooting
- If MQTT disconnects, the sketch retries every 5 seconds and republishes discovery on reconnect.
- If Wi-Fi does not connect, check credentials and signal strength.
- For voltage readings, verify the A0 wiring and voltage divider ratio.
