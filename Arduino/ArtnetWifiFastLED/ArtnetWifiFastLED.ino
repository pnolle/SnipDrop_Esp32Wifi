/*
This example will receive multiple universes via Art-Net and control a strip of
WS2812 LEDs via the FastLED library: https://github.com/FastLED/FastLED
This example may be copied under the terms of the MIT license, see the LICENSE file for details
*/
#include <ArtnetWifi.h>
#include <Arduino.h>
#include <FastLED.h>

// Wifi settings
const char* ssid = "Koehn.Koeln";
const char* password = "dueddelschi_g21";
// const char* ssid = "ssid";
// const char* password = "pAsSwOrD";

// LED settings
const int numLeds = 300;
const int numLeds2 = 150;
const int numberOfChannels = (numLeds + numLeds2) * 3 ; // Total number of channels you want to receive (1 led = 3 channels)
const byte dataPin = 12;
const byte dataPin2 = 14;
CRGB leds[numLeds];
CRGB leds2[numLeds2];

// Art-Net settings
ArtnetWifi artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;


// connect to wifi – returns true if successful or false if not
bool ConnectWifi(void)
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
  }

  return state;
}

void initTest()
{
  for (int i = 0 ; i < numLeds ; i++)
  {
    leds[i] = CRGB(127, 0, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < numLeds ; i++)
  {
    leds[i] = CRGB(0, 127, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < numLeds ; i++)
  {
    leds[i] = CRGB(0, 0, 127);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < numLeds ; i++)
  {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

void initTest2()
{
  for (int i = 0 ; i < numLeds ; i++)
  {
    leds2[i] = CRGB(127, 0, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < numLeds2 ; i++)
  {
    leds2[i] = CRGB(0, 127, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < numLeds2 ; i++)
  {
    leds2[i] = CRGB(0, 0, 127);
  }
  FastLED.show();
  delay(500);
  for (int i = 0 ; i < numLeds2 ; i++)
  {
    leds2[i] = CRGB(0, 0, 0);
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
    if (index < 3) {
      int led = i + ((index-1) * 170);
      // Serial.printf("onDmxFrame led%i/%i %u/%u %u %u %i %i\n", led, numLeds, universe, maxUniverses, length, sequence, index, sendFrame);
      if (led < numLeds)
      {
          leds[led] = getColors(i, data);
        //Serial.printf("ledNo %i | r %i | g %i | b %i\n", led, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
      }
    } else {
      int led = i; // + ((index-1) * 170);
      //Serial.printf("onDmxFrame led%i/%i %u/%u %u %u %i %i\n", led, numLeds2, universe, maxUniverses, length, sequence, index, sendFrame);
      if (led < numLeds2)
      {
          leds2[led] = getColors(i, data);
          //Serial.printf("ledNo %i | index %i | r %i | g %i | b %i\n", led, i, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
      }
    }
  }

  if (sendFrame)
  {
    FastLED.show();
    // Reset universeReceived to 0
    memset(universesReceived, 0, maxUniverses);
  }
}

CRGB getColors(int i, uint8_t* data) {
  return CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
}


void setup()
{
  Serial.begin(115200);
  ConnectWifi();
  artnet.begin();
  FastLED.addLeds<WS2813, dataPin, GRB>(leds, numLeds);
  FastLED.addLeds<WS2813, dataPin2, GRB>(leds2, numLeds2);
  initTest();
  initTest2();

  memset(universesReceived, 0, maxUniverses);
  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();
}
