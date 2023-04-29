/*
This example will receive multiple universes via Art-Net and control a strip of
WS2812 leds_A via the FastLED library: https://github.com/FastLED/FastLED
This example may be copied under the terms of the MIT license, see the LICENSE file for details
*/
#include <ArtnetWifi.h>
#include <Arduino.h>
#include <FastLED.h>
#include "secrets.h"          // local variables


// LED settings
// #define NUM_LEDS_C 507 // 507 leds_A in Circle
// #define NUM_LEDS_A 452 // 452 leds_A in Arrow
// #define NUM_LEDS_L 515 // 515 leds_A in Laser v2 + Scissors
#define NUM_LEDS_C 300;
#define NUM_LEDS_A 300;
#define NUM_LEDS_L 150;

#define START_UNIVERSE_A 3;
#define START_UNIVERSE_L 4;

const int numberOfChannels = (NUM_LEDS_C + NUM_LEDS_A + NUM_LEDS_L) * 3 ; // Total number of receive channels (1 led = 3 channels)
const byte dataPin_C = 12;
const byte dataPin_A = 14;
const byte dataPin_L = 27;
CRGB leds_C[NUM_LEDS_C];
CRGB leds_A[NUM_LEDS_A];
CRGB leds_L[NUM_LEDS_L];

// Art-Net settings
ArtnetWifi artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
// bool universesReceived[maxUniverses];
bool sendFrame = 1;


// connect to wifi – returns true if successful or false if not
bool connectWifi(void)
{
  bool state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (i > 20)
    {
      state = false;
      break;
    }
    i++;
  }
  if (state)
  {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("");
    Serial.println("Connection failed.");
    connectWifi(void);
  }

  return state;
}

void initTest()
{
  for (int i = 0 ; i < NUM_LEDS_C ; i++)
  {
    leds_C[i] = CRGB(127, 0, 0);
  }
  for (int i = 0 ; i < NUM_LEDS_A ; i++)
  {
    leds_A[i] = CRGB(127, 0, 0);
  }
  for (int i = 0 ; i < NUM_LEDS_AL; i++)
  {
    leds_L[i] = CRGB(127, 0, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < NUM_LEDS_C ; i++)
  {
    leds_C[i] = CRGB(0, 127, 0);
  }
  for (int i = 0 ; i < NUM_LEDS_A ; i++)
  {
    leds_A[i] = CRGB(0, 127, 0);
  }
  for (int i = 0 ; i < NUM_LEDS_L ; i++)
  {
    leds_L[i] = CRGB(0, 127, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < NUM_LEDS_C ; i++)
  {
    leds_C[i] = CRGB(0, 0, 127);
  }
  for (int i = 0 ; i < NUM_LEDS_A ; i++)
  {
    leds_A[i] = CRGB(0, 0, 127);
  }
  for (int i = 0 ; i < NUM_LEDS_L ; i++)
  {
    leds_L[i] = CRGB(0, 0, 127);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < NUM_LEDS_C ; i++)
  {
    leds_C[i] = CRGB(0, 0, 0);
  }
  for (int i = 0 ; i < NUM_LEDS_A ; i++)
  {
    leds_A[i] = CRGB(0, 0, 0);
  }
  for (int i = 0 ; i < NUM_LEDS_L ; i++)
  {
    leds_L[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  sendFrame = 1;
  // set brightness of the whole strip
  if (universe == 15)
  {
    FastLED.setBrightness(data[0]);
    FastLED.show();
  }

  // range check
  if (universe < startUniverse)
  {
    return;
  }
  uint8_t index = universe - startUniverse;
  
  //if (index >= maxUniverses)
  if (index > maxUniverses)
  {
    return;
  }

//  // Store which universe has got in
//  universesReceived[index] = true;
//
//  for (int i = 0 ; i < maxUniverses ; i++)
//  {
//    if (!universesReceived[i])
//    {
//      sendFrame = 0;
//      break;
//    }
//  }

  //Serial.printf("onDmxFrame %u/%u %u %u %i %i\n", universe, maxUniverses, length, sequence, index, sendFrame);
  
  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++)
  {
    // index is the first relevant universe
    if (index < START_UNIVERSE_A) { // this is the C strip
      int led = i + ((index-1) * 170);
      Serial.printf("C-STRIP led%i/%i %u/%u-%i %u %u %i %i\n", led, NUM_LEDS_C, universe, maxUniverses, 0, length, sequence, index, sendFrame);
      if (led < NUM_LEDS_C)
      {
          leds_C[led] = getColors(i, data);
          Serial.printf("ledNo %i | r %i | g %i | b %i\n", led, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
      }
    } else if (index >= START_UNIVERSE_A && index < START_UNIVERSE_L) { // this is the A strip
      int led = i + ((index-1) * 170);
      Serial.printf("A-STRIP led%i/%i %u/%u-%i %u %u %i %i\n", led, NUM_LEDS_A, universe, maxUniverses, START_UNIVERSE_A, length, sequence, index, sendFrame);
      if (led < NUM_LEDS_A)
      {
          leds_A[led] = getColors(i, data);
          Serial.printf("ledNo %i | index %i | r %i | g %i | b %i\n", led, i, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
      }
    } else if (index >= START_UNIVERSE_L) { // this is the L strip
      int led = i + ((index-1) * 170);
      Serial.printf("L-STRIP led%i/%i %u/%u-%i %u %u %i %i\n", led, NUM_LEDS_L, universe, maxUniverses, START_UNIVERSE_L, length, sequence, index, sendFrame);
      if (led < NUM_LEDS_L)
      {
          leds_L[led] = getColors(i, data);
          Serial.printf("ledNo %i | index %i | r %i | g %i | b %i\n", led, i, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
      }
    }
  }

  if (sendFrame)
  {
    FastLED.show();
    // Reset universeReceived to 0
    // memset(universesReceived, 0, maxUniverses);
  }
}

CRGB getColors(int i, uint8_t* data) {
  return CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
}


void setup()
{
  Serial.begin(115200);
  connectWifi();
  artnet.begin();
  FastLED.addLeds<WS2813, dataPin_C, GRB>(leds_C, NUM_LEDS_C);
  FastLED.addLeds<WS2813, dataPin_A, GRB>(leds_A, NUM_LEDS_A);
  FastLED.addLeds<WS2813, dataPin_L, GRB>(leds_L, NUM_LEDS_L);
  initTest();

  // memset(universesReceived, 0, maxUniverses);
  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();
}
