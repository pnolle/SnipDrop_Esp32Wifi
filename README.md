# SnipDrop
This repo contains the Wi-Fi access point and client projects for my LED rollup banner project 'SnipSign 2.0'

## Sources
Based on the work of mo thunderz: https://github.com/mo-thunderz/Esp32WifiPart2 / https://youtu.be/15X0WvGaVg8
And on this tutorial on data transfer between several ESP32s: https://iotdesignpro.com/projects/real-time-data-transfer-between-two-esp32-using-websocket-client-on-arduino-ide

## Installation
For installation, the following libraries need to be installed:
* Websockets by Markus Sattler (can be tricky to find -> search for "Arduino Websockets"
* ArduinoJson by Benoit Blanchon

Copy secrets.h_template and rename to secrets.h, fill in SSID and Password for ESP32 Wi-Fi.
