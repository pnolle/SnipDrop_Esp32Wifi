/*
   Wi-Fi access point and clients for my LED rollup banner project 'SnipDrop'.
 */

#include <Arduino.h>
#include <Ethernet2.h>    // Use Ethernet2 library for W5500
#include <EthernetUdp2.h> // Udp support for Ethernet2
#include <Artnet.h>
#include <FastLED.h>
#include "secrets.h" // local variables

// Network settings
EthernetUDP Udp; // UDP instance (for Ethernet2)
Artnet artnet;       // Artnet instance

// Declare MAC and IP variables
byte mac[6];
IPAddress ip;

// Device configuration
/*
  Valid values:
  1 = Circle (C)
  2 = Arrow (A)
  3 = Laser + Scissors (L)
*/
#define DEVICE_NUMBER 1

#if DEVICE_NUMBER == 1
byte mac1[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip1(192, 168, 1, 50);

#elif DEVICE_NUMBER == 2
byte mac2[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE};
IPAddress ip2(192, 168, 1, 51);

#elif DEVICE_NUMBER == 3
byte mac3[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF};
IPAddress ip3(192, 168, 1, 52);

#else
#error "Invalid DEVICE_NUMBER. Please define as 1, 2, or 3."
#endif

// Configure IP addresses of the local access point
IPAddress local_IP_AP(192, 168, 1, 22); // C strip
IPAddress local_IP_C1(192, 168, 1, 31); // A strip
IPAddress local_IP_C2(192, 168, 1, 32); // L strip

IPAddress gateway(192, 168, 1, 5);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   // optional
IPAddress secondaryDNS(8, 8, 4, 4); // optional

// LED settings
const int NUM_LEDS_C = 507; // 507 leds_A in Circle
const int NUM_LEDS_A = 452; // 452 leds_A in Arrow
// const int NUM_LEDS_L = 646; // 646 leds_A in Laser v3 + Scissors, 585 in use without deadSpace
const int NUM_LEDS_L = 627; // 646 leds_A in Laser v3 + Scissors, 585 in use without deadSpace

const int numberOfChannels = (NUM_LEDS_C + NUM_LEDS_A + NUM_LEDS_L) * 3 / pixelFactor; // Total number of receive channels (1 led = 3 channels)
const byte DATA_PIN = 12;
CRGB leds_C[NUM_LEDS_C];
CRGB leds_A[NUM_LEDS_A];
CRGB leds_L[NUM_LEDS_L];

// Art-Net / DMX settings
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

const int START_UNIVERSE_A = 4;
const int START_UNIVERSE_L = 7;
const int pixelFactor = 3; // number of pixels displaying the same information to save universes
bool firstDmxFrameReceived = false;

// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool sendFrame = true;

void initTest()
{
  Serial.printf("Init test %i\n", DEVICE_NUMBER);
  for (int i = 0; i < NUM_LEDS_C; i++)
  {
    leds_C[i] = CRGB(127, 0, 0);
  }
  for (int i = 0; i < NUM_LEDS_A; i++)
  {
    leds_A[i] = CRGB(127, 0, 0);
  }
  for (int i = 0; i < NUM_LEDS_L; i++)
  {
    leds_L[i] = CRGB(127, 0, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0; i < NUM_LEDS_C; i++)
  {
    leds_C[i] = CRGB(0, 127, 0);
  }
  for (int i = 0; i < NUM_LEDS_A; i++)
  {
    leds_A[i] = CRGB(0, 127, 0);
  }
  for (int i = 0; i < NUM_LEDS_L; i++)
  {
    leds_L[i] = CRGB(0, 127, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0; i < NUM_LEDS_C; i++)
  {
    leds_C[i] = CRGB(0, 0, 127);
  }
  for (int i = 0; i < NUM_LEDS_A; i++)
  {
    leds_A[i] = CRGB(0, 0, 127);
  }
  for (int i = 0; i < NUM_LEDS_L; i++)
  {
    leds_L[i] = CRGB(0, 0, 127);
  }
  FastLED.show();
  delay(500);
  for (int i = 0; i < NUM_LEDS_C; i++)
  {
    leds_C[i] = CRGB(0, 0, 0);
  }
  for (int i = 0; i < NUM_LEDS_A; i++)
  {
    leds_A[i] = CRGB(0, 0, 0);
  }
  for (int i = 0; i < NUM_LEDS_L; i++)
  {
    leds_L[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data)
{
  if (!firstDmxFrameReceived)
  {
    Serial.println("DMX reception started.");
    firstDmxFrameReceived = true;
  }

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
  uint8_t thisUniverse = universe - startUniverse;

  // special treatment for L strip
  int leapLCounter = 0;
  int leapLNow = 0;

  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++)
  {
    // thisUniverse is the first relevant universe
    if (thisUniverse < START_UNIVERSE_A) // this is the C strip on universe 1
    {
      int led = i * pixelFactor + ((thisUniverse - 1) * 170); // for thisUniverse==1 ? led start at 0 : led start at 170
      // Serial.printf("C-STRIP from 0 to %i \tled%i/%i %u/%u-%i %u %u %i %i\n", START_UNIVERSE_A-1, led, NUM_LEDS_C, universe, maxUniverses, 0, length, sequence, thisUniverse, sendFrame);
      for (int p = 0; p < pixelFactor; p++)
      {
        if (led < NUM_LEDS_C)
        {
          leds_C[led] = getColors(i, data);
          // Serial.printf("ledNo %i | r %i | g %i | b %i\n", led, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
        }
        led++;
      }
    }
    else if (thisUniverse >= START_UNIVERSE_A && thisUniverse < START_UNIVERSE_L) // this is the A strip on universe 4
    {
      int led = i * pixelFactor + ((thisUniverse - START_UNIVERSE_A) * 170); // for thisUniverse==3 ? led start at 0 : led start at 170
      // Serial.printf("%i: A-STRIP from %i to %i \tthisUniverse%i led%i/%i %u/%u %u\n", i, START_UNIVERSE_A, START_UNIVERSE_L-1, thisUniverse, led, NUM_LEDS_A, length);
      for (int p = 0; p < pixelFactor; p++)
      {
        if (led < NUM_LEDS_A)
        {
          leds_A[led] = getColors(i, data);
          // Serial.printf("leds_A ledNo %i | thisUniverse %i | r %i | g %i | b %i\n", led, thisUniverse, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
        }
        led++;
      }
    }
    else if (thisUniverse >= START_UNIVERSE_L) // this is the L strip on universe 7
    {
      int led = i * pixelFactor + ((thisUniverse - START_UNIVERSE_L) * 170) + leapLCounter; // for thisUniverse==7 ? led start at 0 : <nothing else>
      // if (led==509) Serial.printf("L-STRIP from %i to infinityyy! \tled%i/%i %u/%u-%i %u %u %i %i\n", START_UNIVERSE_L, led, NUM_LEDS_L, universe, maxUniverses, START_UNIVERSE_L, length, sequence, thisUniverse, sendFrame);

      // special treatment for the L strip because uses 585 leds, which is 75 longer than 3*170 (=510): add 1 extra LED every 2nd time
      int thisPixelFactor = pixelFactor;
      if (leapLNow == 2)
      {
        thisPixelFactor++;
        leapLNow = 0;
        leapLCounter++;
      }
      leapLNow++;
      for (int p = 0; p < thisPixelFactor; p++)
      {
        int deadSpaceLed = addDeadSpace(led);
        if (deadSpaceLed < NUM_LEDS_L)
        {
          leds_L[deadSpaceLed] = getColors(i, data);
          // Serial.printf("leds_L led %i => led incl. deadSpace %i | thisUniverse %i | r %i | g %i | b %i\n", led, addDeadSpace(led), thisUniverse, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
        }
        led++;
      }
      // Serial.println("");
    }
  }

  if (sendFrame)
  {
    FastLED.show();
  }
}

int addDeadSpace(int led)
{
  int deadSpace = 0;
  if (led > 19)
  {
    deadSpace = 5;
  }
  if (led > 45)
  {
    deadSpace += 6;
  }
  if (led > 65) // 3
  {
    deadSpace += 5;
  }
  if (led > 91)
  {
    deadSpace += 6;
  }
  if (led > 111)
  {
    deadSpace += 5;
  }
  if (led > 138) // 6
  {
    deadSpace += 6;
  }
  if (led > 158)
  {
    deadSpace += 5;
  }
  if (led > 186)
  {
    deadSpace += 6;
  }
  if (led > 205)
  {
    deadSpace += 5;
  }
  if (led > 232)
  {
    deadSpace += 5;
  }
  if (led > 252)
  {
    deadSpace += 7;
  }
  // maximum deadSpace is 61
  return led + deadSpace;
}

CRGB getColors(int i, uint8_t *data)
{
  return CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
}

/*
 * Setup logging and ethernet
 */
void basicSetup()
{
  Serial.println(ESP.getSdkVersion());
  esp_log_level_set("*", ESP_LOG_VERBOSE);

  Serial.printf("MCU DEVICE_NUMBER is %i\n", DEVICE_NUMBER);

  // Assign MAC and IP based on DEVICE_NUMBER
  #if DEVICE_NUMBER == 1
    memcpy(mac, mac1, sizeof(mac1));
    ip = ip1;
  #elif DEVICE_NUMBER == 2
    memcpy(mac, mac2, sizeof(mac2));
    ip = ip2;
  #elif DEVICE_NUMBER == 3
    memcpy(mac, mac3, sizeof(mac3));
    ip = ip3;
  #endif

  Serial.println("Device Configuration:");
  Serial.print("MAC: ");
  for (int i = 0; i < 6; i++)
  {
    Serial.print(mac[i], HEX);
    if (i < 5)
      Serial.print(":");
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(ip);

  // Initialize Ethernet with assigned MAC and IP
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("DHCP failed, using static IP.");
    Ethernet.begin(mac, ip);
  }

  Serial.print("Ethernet IP: ");
  Serial.println(Ethernet.localIP());
}

/*
 * Setup FastLED and artnet
 */
void ledSetup()
{
  if (DEVICE_NUMBER == 1)
  {
    FastLED.addLeds<WS2813, DATA_PIN, GRB>(leds_C, NUM_LEDS_C);
    FastLED.setBrightness(255);
    initTest();
  }
  if (DEVICE_NUMBER == 2)
  {
    FastLED.addLeds<WS2813, DATA_PIN, GRB>(leds_A, NUM_LEDS_A);
    FastLED.setBrightness(255);
    initTest();
  }
  if (DEVICE_NUMBER == 3)
  {
    FastLED.addLeds<WS2813, DATA_PIN, GRB>(leds_L, NUM_LEDS_L);
    FastLED.setBrightness(255);
    initTest();
  }
  artnet.begin(Ethernet.localIP(), &Udp);

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
}

void setup()
{
  Serial.begin(115200);
  basicSetup();
  ledSetup();
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();
}
