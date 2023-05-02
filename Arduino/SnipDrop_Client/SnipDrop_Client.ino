/*
  SnipDrop Client

  Establishing connection to WiFi with SSID: SnipDrop

  My devices:

  Connected to network with IP address: 192.168.1.23
  Client has MAC address: C0:49:EF:CF:AD:FC

  Connected to network with IP address: 192.168.1.24
  Client has MAC address: 34:86:5D:FC:80:B4

*/

#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "secrets.h" // local variables
#include "dummypixels.h"

// How many leds in your strip?
#define NUM_LEDS 500 // Half Circle Dummy
// #define NUM_LEDS 452 // 452 LEDs in Arrow
// #define NUM_LEDS 515 // 515 LEDs in Laser v2 + Scissors
// #define NUM_LEDS 507  // 507 LEDs in Circle

#define NUM_ROWS 5
#define NUM_COLS 10

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 12
// #define CLOCK_PIN 13

// Define the array of leds
CRGB leds[NUM_LEDS];

uint16_t previousDataLength = 0;
uint8_t universe = 0;
uint8_t startUniverse = 0;

WebServer server(80);
WebSocketsClient webSocket;

String webpage = "<!DOCTYPE html><html><head><title>Client</title></head><body style='background-color: #EEEEEE;'><span style='color: #003366;'><h1>Randomizr</h1><p>Random number: <p><span id='rand1'>-</span></p><p><span id='rand2'>-</span></p></p><button type='button' id='BTN_SEND_BACK'>Send info to ESP32</button></span></body><script>var Socket; document .getElementById('BTN_SEND_BACK') .addEventListener('click', button_send_back); function button_send_back(){ var light_details={ ledNum: 1, r: 2, g: 3, b: 4,}; Socket.send(JSON.stringify(light_details));} function init(event){ Socket=new WebSocket('ws://' + window.location.hostname + ':81/'); Socket.onmessage=function (event){ processCommand(event);};} function processCommand(event){ var obj=JSON.parse(event.data); document.getElementById('rand1').innerHTML=obj.rand1; document.getElementById('rand2').innerHTML=obj.rand2; console.log(obj.rand1); console.log(obj.rand2);} window.onload=function (event){ init(event);}; </script></html>";
int interval = 100;
unsigned long previousMillis = 0;

// Define variables to store BME280 readings to be sent
uint16_t ledNum;
uint8_t colR;
uint8_t colG;
uint8_t colB;


void initTest()
{
  for (int i = 0 ; i < NUM_LEDS ; i++)
  {
    leds[i] = CRGB(127, 0, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < NUM_LEDS ; i++)
  {
    leds[i] = CRGB(0, 127, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < NUM_LEDS ; i++)
  {
    leds[i] = CRGB(0, 0, 127);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < NUM_LEDS ; i++)
  {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

void setup()
{
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // init LEDs
  FastLED.addLeds<WS2813, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  initTest();

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  // TODO: Clients should be able to send some kind of feedback. Figure out if it's better to use the webserver or websockets back to AP or the WebSerial lib (https://randomnerdtutorials.com/esp32-webserial-library/)
  server.on("/", []()
            { server.send(200, "text\html", webpage); });
  server.begin();

  // address, port, and URL path
  webSocket.begin("192.168.1.22", 81, "/");
  // WebSocket event handler
  webSocket.onEvent(webSocketEvent);
  // if connection failed retry every 5s
  webSocket.setReconnectInterval(5000);
}

// void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length)
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  Serial.printf("Client webSocketEvent %u\n", payload);
  if (type == WStype_TEXT)
  {
    // parseJsonMessage(payload);

    // read universe and put into the right part of the display buffer
    // using length/3 because 3 values define r/g/b of one pixel
    //    for (int dataNo = 0; dataNo < length / 3; dataNo++)
    for (int dataNo = 0; dataNo < NUM_LEDS; dataNo++)
    {
      int pxNum = dataNo; // + (universe - startUniverse) * (previousDataLength / 3);
      // Serial.printf("%i + (%u - %u) * (%u / 3) = %i<%i\n", dataNo, universe, startUniverse, previousDataLength, pxNum, pxTotal);

      uint16_t ledNum = pxNum;
      uint8_t colR = payload[dataNo * 3];
      uint8_t colG = payload[dataNo * 3 + 1];
      uint8_t colB = payload[dataNo * 3 + 2];

      Serial.printf("Client received:\tledNum: %u | colR: %u | colG: %u | colB: %u\n", ledNum, colR, colG, colB);
      // setLedRegions(ledNum, colR, colG, colB);
      setSingleLed(ledNum, colR, colG, colB);
    }

    // previousDataLength = length;
    FastLED.show();
    fadeall();
  }
}

void parseJsonMessage(uint8_t *payload)
{
  // try to decipher the JSON string received
  StaticJsonDocument<200> doc; // create a JSON container
  DeserializationError error = deserializeJson(doc, payload);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  else
  {
    uint16_t ledNum = doc["ledNum"];
    uint8_t colR = doc["colR"];
    uint8_t colG = doc["colG"];
    uint8_t colB = doc["colB"];
    // Serial.printf("Client received:\tledNum: %u | colR: %u | colG: %u | colB: %u\n", ledNum, colR, colG, colB);
    // setLedRegions(ledNum, colR, colG, colB);
    setSingleLed(ledNum, colR, colG, colB);
  }
}

void setSingleLed(uint16_t pxNum, uint8_t r, uint8_t g, uint8_t b)
{
  leds[pxNum] = CRGB(r, g, b);
}

void fadeall()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i].nscale8(250);
  }
}

void setLedRegions(uint16_t pxNum, uint8_t incomingColR, uint8_t incomingColG, uint8_t incomingColB)
{
  int16_t thisCount = 0;
  const int16_t *thisRegion;

  // printf("setLedValues #%i \tpxNum: %i | dataNo: %u\n", dataNo, pxNum);

  switch (pxNum)
  {
  // row 1
  case 10:
    thisCount = len_p11_1;
    thisRegion = p11_1;
    break;
  case 11:
    thisCount = len_p12_1;
    thisRegion = p12_1;
    break;
  case 12:
    thisCount = len_p13_1;
    thisRegion = p13_1;
    break;
  case 13:
    thisCount = len_p14_1;
    thisRegion = p14_1;
    break;
  case 14:
    thisCount = len_p15_1;
    thisRegion = p15_1;
    break;

  // row 2
  case 23:
    thisCount = len_p14_2;
    thisRegion = p14_2;
    break;
  case 24:
    thisCount = len_p15_2;
    thisRegion = p15_2;
    break;
  case 25:
    thisCount = len_p16_2;
    thisRegion = p16_2;
    break;

  // row 3
  case 35:
    thisCount = len_p16_3;
    thisRegion = p16_3;
    break;
  case 36:
    thisCount = len_p17_3;
    thisRegion = p17_3;
    break;
  case 37:
    thisCount = len_p18_3;
    thisRegion = p18_3;
    break;

  // row 4
  case 46:
    thisCount = len_p17_4;
    thisRegion = p17_4;
    break;
  case 47:
    thisCount = len_p18_4;
    thisRegion = p18_4;
    break;
  case 48:
    thisCount = len_p19_4;
    thisRegion = p19_4;
    break;

  // row 5
  case 58:
    thisCount = len_p19_5;
    thisRegion = p19_5;
    break;
  case 59:
    thisCount = len_p20_5;
    thisRegion = p20_5;
    break;
  case 60:
    thisCount = len_p21_5;
    thisRegion = p21_5;
    break;

  // row 6
  case 69:
    thisCount = len_p20_6;
    thisRegion = p20_6;
    break;
  case 70:
    thisCount = len_p21_6;
    thisRegion = p21_6;
    break;
  }
  // setLedValues(thisCount, thisRegion, data[dataNo * 3], data[dataNo * 3 + 1], data[dataNo * 3 + 2]);
  setLedValues(thisCount, thisRegion, incomingColR, incomingColG, incomingColB);
}

void setLedValues(int16_t thisCount, const int16_t *thisRegion, uint8_t r, uint8_t g, uint8_t b)
{
  for (int l = 0; l < thisCount; l++)
  {
    leds[thisRegion[l]] = CRGB(r, g, b);
  }
}

void loop()
{
  server.handleClient();
  webSocket.loop();

  // unsigned long now = millis();
  // if (now - previousMillis > interval)
  // {
  //   String jsonString = "";
  //   JsonObject object = doc_tx.to<JsonObject>();
  //   object["rand1"] = random(100);
  //   object["rand2"] = random(100);
  //   serializeJson(doc_tx, jsonString);
  //   Serial.print("Client sending JSON data: ");
  //   Serial.println(jsonString);
  //   webSocket.broadcastTXT(jsonString);
  //   previousMillis = now;
  // }
}
