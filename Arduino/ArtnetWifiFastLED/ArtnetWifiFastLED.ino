/*
Wi-Fi access point and clients for my LED rollup banner project 'SnipDrop'.
*/
#include <Arduino.h>
#include <ArtnetWifi.h>
#include <FastLED.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>
#include "secrets.h" // local variables

// Code configuration
/*
Valid values:
1 = Access Point (192.168.1.22) + Circle (C)
2 = Client 1 (192.168.1.31) Arrow (A)
3 = Client 2 (192.168.1.32) Laser + Scissors (L)
*/
const int config = 3;

// Configure IP addresses of the local access point
IPAddress local_IP_AP(192, 168, 1, 22);
IPAddress local_IP_C1(192, 168, 1, 31);
IPAddress local_IP_C2(192, 168, 1, 32);

IPAddress gateway(192, 168, 1, 5);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   // optional
IPAddress secondaryDNS(8, 8, 4, 4); // optional

// LED settings
const int NUM_LEDS_C = 507; // 507 leds_A in Circle
const int NUM_LEDS_A = 452; // 452 leds_A in Arrow
const int NUM_LEDS_L = 515; // 515 leds_A in Laser v2 + Scissors

// const int NUM_LEDS_C = 300;
// const int NUM_LEDS_A = 300;
// const int NUM_LEDS_L = 150;

const int START_UNIVERSE_A = 4;
const int START_UNIVERSE_L = 7;

const int pixelFactor = 3; // number of pixels displaying the same information to save universes

const int numberOfChannels = (NUM_LEDS_C + NUM_LEDS_A + NUM_LEDS_L) * 3 / pixelFactor; // Total number of receive channels (1 led = 3 channels)
const byte dataPin = 12;
CRGB leds_C[NUM_LEDS_C];
CRGB leds_A[NUM_LEDS_A];
CRGB leds_L[NUM_LEDS_L];

bool firstDmxFrameReceived = false;

// Art-Net settings
ArtnetWifi artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
// bool universesReceived[maxUniverses];
bool sendFrame = true;

// Web Serial
AsyncWebServer server(80);

// connect to wifi – returns true if successful or false if not
bool connectWifi(int clientNo = 1)
{
  bool state = true;
  int i = 0;

  Serial.printf("Wi-Fi clientNo is %i\n", clientNo);

  // Configures static IP address
  if (clientNo == 1)
  {
    if (!WiFi.config(local_IP_C1, gateway, subnet, primaryDNS, secondaryDNS))
    {
      Serial.println("STA Failed to configure");
    }
  }
  if (clientNo == 2)
  {
    if (!WiFi.config(local_IP_C2, gateway, subnet, primaryDNS, secondaryDNS))
    {
      Serial.println("STA Failed to configure");
    }
  }

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  WebSerial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");
  WebSerial.print("Connecting");
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
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    WebSerial.println("");
    WebSerial.print("Connected to ");
    WebSerial.println(ssid);
    WebSerial.print("IP address: ");
    WebSerial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Connection failed.");
    WebSerial.println("Connection failed.");
    connectWifi(clientNo);
  }

  return state;
}

bool startWifiAccessPoint()
{
  Serial.print("Setting up Access Point ... ");
  Serial.println(WiFi.softAPConfig(local_IP_AP, gateway, subnet) ? "Ready" : "Failed!");
  Serial.print("Starting Access Point ... ");
  WebSerial.print("Setting up Access Point ... ");
  WebSerial.println(WiFi.softAPConfig(local_IP_AP, gateway, subnet) ? "Ready" : "Failed!");
  WebSerial.print("Starting Access Point ... ");
  if (WiFi.softAP(ssid, password))
  {
    Serial.println("Ready");
  }
  else
  {
    Serial.println("Failed!");
    return false;
  }

  const IPAddress apIpAddress = WiFi.softAPIP();
  Serial.print("IP address = ");
  Serial.println(apIpAddress);
  WebSerial.print("IP address = ");
  WebSerial.println(apIpAddress);
  return true;
}

void testIncr()
{
  for (int i = 0; i < 100; i++)
  {
    leds_L[i] = CRGB(127, 0, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0; i < 200; i++)
  {
    leds_L[i] = CRGB(0, 127, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0; i < 300; i++)
  {
    leds_L[i] = CRGB(0, 0, 127);
  }
  FastLED.show();
  delay(500);
  for (int i = 0; i < 400; i++)
  {
    leds_L[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0; i < 500; i++)
  {
    leds_L[i] = CRGB(127, 0, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0; i < NUM_LEDS_L; i++)
  {
    leds_L[i] = CRGB(0, 127, 0);
  }
  FastLED.show();
  delay(500);
}

void initTest()
{
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

  // if (thisUniverse >= maxUniverses)
  //  if (thisUniverse > maxUniverses)
  //  {
  //    return;
  //  }

  //  // Store which universe has got in
  //  universesReceived[thisUniverse] = true;
  //
  //  for (int i = 0 ; i < maxUniverses ; i++)
  //  {
  //    if (!universesReceived[i])
  //    {
  //      sendFrame = 0;
  //      break;
  //    }
  //  }

  // Serial.printf("onDmxFrame %u/%u %u %u %i %i\n", universe, maxUniverses, length, sequence, thisUniverse, sendFrame);

  // special treatment for L strip
  int leapLCounter = 0;
  int leapLNow = 0;

  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++)
  {
    // thisUniverse is the first relevant universe
    if (thisUniverse < START_UNIVERSE_A)  // this is the C strip on universe 1
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
    else if (thisUniverse >= START_UNIVERSE_A && thisUniverse < START_UNIVERSE_L)  // this is the A strip on universe 4
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
    else if (thisUniverse >= START_UNIVERSE_L)  // this is the L strip on universe 7
    { 
      // special treatment for the L strip because it's 5 longer than 3*170: add 1 extra LED every 34th time
      int led = i * pixelFactor + ((thisUniverse - START_UNIVERSE_L) * 170) + leapLCounter; // for thisUniverse==5 ? led start at 0 : <nothing else>
      // if (led==509) Serial.printf("L-STRIP from %i to infinityyy! \tled%i/%i %u/%u-%i %u %u %i %i\n", START_UNIVERSE_L, led, NUM_LEDS_L, universe, maxUniverses, START_UNIVERSE_L, length, sequence, thisUniverse, sendFrame);

      int thisPixelFactor = pixelFactor;
      if (leapLNow == 33) {
        thisPixelFactor++;
        leapLNow = 0;
        leapLCounter++;
      }
      leapLNow++;
      for (int p = 0; p < thisPixelFactor; p++)
      {
        if (led < NUM_LEDS_L)
        {
          leds_L[led] = getColors(i, data);
          // Serial.printf("leds_L ledNo %i | thisUniverse %i | r %i | g %i | b %i\n", led, i, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
          // Serial.printf("%i ", led);
        }
        led++;
      }
      // Serial.println("");
    }
  }

  if (sendFrame)
  {
    FastLED.show();
    // Reset universeReceived to 0
    // memset(universesReceived, 0, maxUniverses);
  }
}

CRGB getColors(int i, uint8_t *data)
{
  return CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
}

void onWebSerialMsg(uint8_t *data, size_t len){
  WebSerial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerial.println(d);
}

void setup()
{
  Serial.begin(115200);

  Serial.println("before WebSerial setup");
  // WebSerial is accessible at "<IP Address>/webserial" in browser
  WebSerial.begin(&server);
  // WebSerial.msgCallback(onWebSerialMsg);
  server.begin();

  if (config == 1)
  {
    startWifiAccessPoint();
    FastLED.addLeds<WS2813, dataPin, GRB>(leds_C, NUM_LEDS_C);
  }
  if (config == 2)
  {
    connectWifi(1);
    FastLED.addLeds<WS2813, dataPin, GRB>(leds_A, NUM_LEDS_A);
  }
  if (config == 3)
  {
    connectWifi(2);
    FastLED.addLeds<WS2813, dataPin, GRB>(leds_L, NUM_LEDS_L);
  }
  artnet.begin();
  initTest();
  // testIncr();

  // memset(universesReceived, 0, maxUniverses);
  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();
}
