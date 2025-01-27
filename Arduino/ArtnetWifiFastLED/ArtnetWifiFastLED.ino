/*
  Wi-Fi access point and clients for my LED rollup banner project 'SnipDrop'.
*/
#include <ArtnetWifi.h>
#include <Arduino.h>
#include <FastLED.h>
#include "secrets.h" // local variables

// Code configuration
/*
  Valid values:
  1 = Access Point (192.168.1.22) + Circle (C)
  2 = Client 1 (192.168.1.31) Arrow (A)
  3 = Client 2 (192.168.1.32) Laser + Scissors (L)
*/
const int config = 1;

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

const int START_UNIVERSE_A = 4;
const int START_UNIVERSE_L = 7;

const int pixelFactor = 3; // number of pixels displaying the same information to save universes

const int numberOfChannels = (NUM_LEDS_C + NUM_LEDS_A + NUM_LEDS_L) * 3 / pixelFactor; // Total number of receive channels (1 led = 3 channels)
const byte dataPin = 12;
CRGB leds_C[NUM_LEDS_C];
CRGB leds_A[NUM_LEDS_A];
CRGB leds_L[NUM_LEDS_L];

bool firstDmxFrameReceived = false;
static unsigned long lastPrintTime = 0;

// Art-Net settings
ArtnetWifi artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool sendFrame = true;


/* 
 *  START
 *  Code for Wi-Fi client(s)
 */

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
    connectWifi(clientNo);
  }

  return state;
}

/* 
 *  END
 *  Code for Wi-Fi client(s)
 */


/* 
 *  START
 *  Code for Wi-Fi access point
 */
void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
  switch (event)
  {
  case WIFI_EVENT_AP_STACONNECTED:
    Serial.println("WIFI_EVENT_AP_STACONNECTED");
//    Serial.printf("Client connected! MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
//                  info.wifi_sta_connected.mac[0], info.wifi_sta_connected.mac[1], info.wifi_sta_connected.mac[2],
//                  info.wifi_sta_connected.mac[3], info.wifi_sta_connected.mac[4], info.wifi_sta_connected.mac[5]);
//    Serial.printf("AID = %d\n", info.wifi_ap_stadisconnected.aid); // AID is the Association ID
  Serial.println("IP address connected: ");
  Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
    break;

  case WIFI_EVENT_AP_STADISCONNECTED:
  Serial.print("WIFI_EVENT_AP_STADISCONNECTED");
//    Serial.printf("Client disconnected! MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
//                  info.wifi_ap_stadisconnected.mac[0], info.wifi_ap_stadisconnected.mac[1], info.wifi_ap_stadisconnected.mac[2],
//                  info.wifi_ap_stadisconnected.mac[3], info.wifi_ap_stadisconnected.mac[4], info.wifi_ap_stadisconnected.mac[5]);
//    Serial.printf("AID = %d\n", info.wifi_ap_stadisconnected.aid);
  Serial.println("IP address disconnected: ");
  Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
    break;

  default:
    break;
  }
}

bool startWifiAccessPoint()
{
  WiFi.mode(WIFI_AP);
  delay(250);

  Serial.print("Setting up Access Point ... ");
  Serial.println(WiFi.softAPConfig(local_IP_AP, gateway, subnet) ? "Ready" : "Failed!");
  delay(250);

  Serial.print("Starting Access Point ... ");
  if (WiFi.softAP(ssid, password))
  {
    Serial.printf("Ready %s %s\n", ssid, password);
  }
  else
  {
    Serial.println("Failed!");
    return false;
  }
  delay(250);
  
  // Attach WiFi event handler
  WiFi.onEvent(onWiFiEvent);

  Serial.print("Access point IP address: ");
  Serial.println(WiFi.softAPIP());
  return true;
}

/* 
 *  END
 *  Code for Wi-Fi access point
 */

void initTest()
{
  Serial.printf("Init test %i\n", config);
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

  // Serial.printf("onDmxFrame %u/%u %u %u %i %i\n", universe, maxUniverses, length, sequence, thisUniverse, sendFrame);

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
    else if (thisUniverse >= START_UNIVERSE_L)  // this is the L strip on universe 7
    {
      int led = i * pixelFactor + ((thisUniverse - START_UNIVERSE_L) * 170) + leapLCounter; // for thisUniverse==7 ? led start at 0 : <nothing else>
      // if (led==509) Serial.printf("L-STRIP from %i to infinityyy! \tled%i/%i %u/%u-%i %u %u %i %i\n", START_UNIVERSE_L, led, NUM_LEDS_L, universe, maxUniverses, START_UNIVERSE_L, length, sequence, thisUniverse, sendFrame);

      // special treatment for the L strip because uses 585 leds, which is 75 longer than 3*170 (=510): add 1 extra LED every 2nd time
      int thisPixelFactor = pixelFactor;
      if (leapLNow == 2) {
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

int addDeadSpace(int led) {
  int deadSpace = 0;
  if (led > 19)
  {
    deadSpace = 5;
  }
  if (led > 45)
  {
    deadSpace += 6;
  }
  if (led > 65) //3
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
  if (led > 138)  //6
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

void setup()
{
  Serial.begin(115200);
  Serial.printf("MCU config is %i\n", config);

  if (config == 1)
  {
    FastLED.addLeds<WS2813, dataPin, GRB>(leds_C, NUM_LEDS_C);
    FastLED.setBrightness(255);
    initTest();
    startWifiAccessPoint();
  }
  if (config == 2)
  {
    FastLED.addLeds<WS2813, dataPin, GRB>(leds_A, NUM_LEDS_A);
    FastLED.setBrightness(255);
    initTest();
    connectWifi(1);
  }
  if (config == 3)
  {
    FastLED.addLeds<WS2813, dataPin, GRB>(leds_L, NUM_LEDS_L);
    FastLED.setBrightness(255);
    initTest();
    connectWifi(2);
  }
  artnet.begin();

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
}

void printConnectedClients()
{
  int numClients = WiFi.softAPgetStationNum();
  Serial.printf("Number of connected clients: %d\n", numClients);
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();
  
  // Print the number of connected clients every 5 seconds
  if (config == 1 && millis() - lastPrintTime >= 5000)
  {
    printConnectedClients();
    lastPrintTime = millis();
  }
}
