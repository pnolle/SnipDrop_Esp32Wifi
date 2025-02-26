/*
Wi-Fi access point and clients for my LED rollup banner project 'SnipDrop'.
*/
#include <ArtnetWifi.h>
#include <Arduino.h>
#include <FastLED.h>
#include "secrets.h" // local variables

// define enum for the different modes
enum Mode {
  MODE_CIRCLE,
  MODE_ARROW,
  MODE_LASERSCISSORS
};

// Define the pins
#define GPIO_16 16
#define GPIO_17 17
#define GPIO_18 18
#define GPIO_19 19
#define DATA_PIN 12

// Code configuration
/*
Valid values defined in enum Mode:
1 = Access Point (192.168.1.22) + Circle (C)
2 = Client 1 (192.168.1.31) Arrow (A)
3 = Client 2 (192.168.1.32) Laser + Scissors (L)
*/
Mode config = Mode::MODE_CIRCLE;

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
CRGB leds_C[NUM_LEDS_C];
CRGB leds_A[NUM_LEDS_A];
CRGB leds_L[NUM_LEDS_L];

bool firstDmxFrameReceived = false;

// Art-Net settings
ArtnetWifi artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool sendFrame = true;

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

bool startWifiAccessPoint()
{
  Serial.print("Setting up Access Point ... ");
  Serial.println(WiFi.softAPConfig(local_IP_AP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Starting Access Point ... ");
  if (WiFi.softAP(ssid, password))
  {
    Serial.println("Ready");
  }
  else
  {
    Serial.println("Failed!");
    return false;
  }

  Serial.print("IP address = ");
  Serial.println(WiFi.softAPIP());
  return true;
}

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
  delay(1000);
}

void initDisplayNumber(CRGB* leds, int numLeds)
{
  Serial.printf("Init display number %i\n", config);
  for (int n = 0; n < config+1; n++)
  {
    Serial.printf("Blink white %i times for %i LEDs\n", n+1, numLeds);
    for (int i = 0; i < numLeds; i++)
    {
      leds[i] = CRGB(127, 127, 127);
    }
    FastLED.show();
    delay(1000);
    for (int i = 0; i < numLeds; i++)
    {
      leds[i] = CRGB(0, 0, 0);
    }
    FastLED.show();
    delay(1000);
  }
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

void modeDetect() {
  // Configure GPIO 17 as input
  pinMode(GPIO_17, INPUT_PULLUP);

  // Configure GPIOs 16, 18, and 19 as outputs
  pinMode(GPIO_16, OUTPUT);
  pinMode(GPIO_18, OUTPUT);
  pinMode(GPIO_19, OUTPUT);

  // Initialize output pins to HIGH (disconnected state)
  digitalWrite(GPIO_16, HIGH);
  digitalWrite(GPIO_18, HIGH);
  digitalWrite(GPIO_19, HIGH);

  // Check connections
  checkConnections();
}

void checkConnections() {
  // Mode 1: Check if GPIO_16 and GPIO_17 are connected
  digitalWrite(GPIO_16, LOW);
  delay(100); // Small delay to stabilize the signal
  bool mode1 = digitalRead(GPIO_17) == LOW;
  digitalWrite(GPIO_16, HIGH);

  // Mode 2: Check if GPIO_18 and GPIO_17 are connected
  digitalWrite(GPIO_18, LOW);
  delay(100);
  bool mode2 = digitalRead(GPIO_17) == LOW;
  digitalWrite(GPIO_18, HIGH);

  // Mode 3: Check if GPIO_19 and GPIO_17 are connected
  digitalWrite(GPIO_19, LOW);
  delay(100);
  bool mode3 = digitalRead(GPIO_17) == LOW;
  digitalWrite(GPIO_19, HIGH);

  // Set firmware mode accordingly
  if (mode1) {
    Serial.println("Mode 1 detected (GPIO 16 and GPIO 17 connected)");
    setFirmwareMode(1);
  } else if (mode2) {
    Serial.println("Mode 2 detected (GPIO 18 and GPIO 17 connected)");
    setFirmwareMode(2);
  } else if (mode3) {
    Serial.println("Mode 3 detected (GPIO 19 and GPIO 17 connected)");
    setFirmwareMode(3);
  } else {
    Serial.println("No valid mode detected");
  }
}

void setFirmwareMode(int mode) {
  switch (mode) {
    case 1:
      Serial.println("Setting firmware to mode CIRCLE");
      config = Mode::MODE_CIRCLE;
      break;
    case 2:
      Serial.println("Setting firmware to mode ARROW");
      config = Mode::MODE_ARROW;
      break;
    case 3:
      Serial.println("Setting firmware to mode LASERSCISSORS");
      config = Mode::MODE_LASERSCISSORS;
      break;
    default:
      Serial.println("Unknown mode");
      break;
  }
}

void setup()
{
  // start logging
  Serial.begin(115200);

  // detect mode via jumper
  modeDetect();

Serial.print("DATA_PIN: ");
Serial.println(DATA_PIN);

  Serial.printf("MCU config is %i\n", config);
  if (config == Mode::MODE_CIRCLE)
  {
    FastLED.addLeds<WS2813, DATA_PIN, GRB>(leds_C, NUM_LEDS_C);
    FastLED.setBrightness(255);
    initTest();
    initDisplayNumber(leds_C, NUM_LEDS_C);
    startWifiAccessPoint();
  }
  if (config == Mode::MODE_ARROW)
  {
    FastLED.addLeds<WS2813, DATA_PIN, GRB>(leds_A, NUM_LEDS_A);
    FastLED.setBrightness(255);
    initTest();
    initDisplayNumber(leds_A, NUM_LEDS_A);
    connectWifi(1);
  }
  if (config == Mode::MODE_LASERSCISSORS)
  {
    FastLED.addLeds<WS2813, DATA_PIN, GRB>(leds_L, NUM_LEDS_L);
    FastLED.setBrightness(255);
    initTest();
    initDisplayNumber(leds_L, NUM_LEDS_L);
    connectWifi(2);
  }
  artnet.begin();

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();
}
