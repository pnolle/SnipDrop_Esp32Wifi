/*
  SnipDrop Access Point
*/

#include <WiFi.h> // needed to connect to WiFi
#include <ArtnetWifi.h>
#include <WebServer.h>        // needed to create a simple webserver (make sure tools -> board is set to ESP32, otherwise you will get a "WebServer.h: No such file or directory" error)
#include <WebSocketsServer.h> // needed for instant communication between client and server through Websockets
#include <ArduinoJson.h>      // needed for JSON encapsulation (send multiple variables with one string)
#include "secrets.h"          // local variables

// Configure IP addresses of the local access point
IPAddress local_IP(192, 168, 1, 22);
IPAddress gateway(192, 168, 1, 5);
IPAddress subnet(255, 255, 255, 0);

// The String below "webpage" contains the complete HTML code that is sent to the client whenever someone connects to the webserver
String webpage = "<!DOCTYPE html><html><head><title>Access Point</title></head><body style='background-color: #EEEEEE;'><span style='color: #003366;'><h1>Lets generate a random number</h1><p>The first random number is: <span id='rand1'>-</span></p><p>The second random number is: <span id='rand2'>-</span></p><p><button type='button' id='BTN_SEND_BACK'>Send info to ESP32</button></p></span></body><script>var Socket; document.getElementById('BTN_SEND_BACK').addEventListener('click', button_send_back); function init(){ Socket=new WebSocket('ws://' + window.location.hostname + ':81/'); Socket.onmessage=function(event){ processCommand(event);};} function button_send_back(){ var msg={brand: 'Gibson',type: 'Les Paul Studio',year: 2010,color: 'white'};Socket.send(JSON.stringify(msg));} function processCommand(event){var obj=JSON.parse(event.data);document.getElementById('rand1').innerHTML=obj.apRand1;document.getElementById('rand2').innerHTML=obj.apRand2; console.log('obj', obj);} window.onload=function(event){ init();}</script></html>";

// The JSON library uses static memory, so this will need to be allocated:
// -> in the video I used global variables for "doc_tx" and "doc_rx", however, I now changed this in the code to local variables instead "doc" -> Arduino documentation recomends to use local containers instead of global to prevent data corruption

// We want to periodically send values to the clients, so we need to define an "interval" and remember the last time we sent data to the client (with "previousMillis")
int interval = 40;                // send data to the client every 1000ms -> 1s
unsigned long previousMillis = 0; // we use the "millis()" command for time reference and this will output an unsigned long

StaticJsonDocument<200> doc_tx;
StaticJsonDocument<200> doc_rx;

// Artnet
ArtnetWifi artnet;
const int startUniverse = 0;
uint16_t previousDataLength = 0;

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message
{
  uint16_t ledNum;
  uint8_t colR;
  uint8_t colG;
  uint8_t colB;
} struct_message;

// Create a struct_message for dummy data
struct_message DummyData;
// Create a struct_message for Artnet data
struct_message artnetData;
// Create a struct_message to hold incoming data
struct_message incomingReadings;

// Initialization of webserver and websocket
WebServer server(80);
// WiFiServer wifiServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void setup()
{
  Serial.begin(115200); // init serial port for debugging

  Serial.print("Setting up Access Point ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Starting Access Point ... ");
  Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!");

  Serial.print("IP address = ");
  Serial.println(WiFi.softAPIP());

  server.on("/", []() {                     // define here wat the webserver needs to do
    server.send(200, "text/html", webpage); //    -> it needs to send out the HTML string "webpage" to the client
  });
  server.begin(); // start server

  // onDmxFrame will execute every time a packet is received by the ESP32
  artnet.begin();
  artnet.setArtDmxCallback(onDmxFrame);

  webSocket.begin(); // start websocket
  // webSocket.onEvent(webSocketEvent); // define a callback function -> what does the ESP32 need to do when an event from the websocket is received? -> run function "webSocketEvent()"
}

// Callback when data is received (DMX)
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data)
{
  Serial.printf("onDmxFrame %u %u %u\n", universe, length, sequence);
  webSocket.broadcastTXT(data);

  String jsonString = "";      // create a JSON string for sending data to the client
  StaticJsonDocument<2000> doc; // create a JSON container

  JsonArray array = doc.to<JsonArray>();

  // read universe and put into the right part of the display buffer
  // using length/3 because 3 values define r/g/b of one pixel
  for (int dataNo = 0; dataNo < length / 3; dataNo++)
  {
//    int pxNum = dataNo + (universe - startUniverse) * (previousDataLength / 3);
//    Serial.printf("%i + (%u - %u) * (%u / 3) = %i<%i\n", dataNo, universe, startUniverse, previousDataLength, pxNum, length);
//    Serial.printf("r %i | g %i | b %i\n", data[dataNo * 3], data[dataNo * 3 + 1], data[dataNo * 3 + 2]);

    //  array.add(pxNum);
    array.add(data[dataNo * 3]);
    array.add(data[dataNo * 3 + 1]);
    array.add(data[dataNo * 3 + 2]);

    //  JsonObject object = doc.to<JsonObject>(); // create a JSON Object
    //  object["ledNum"] = pxNum;
    //  object["colR"] = data[dataNo * 3];
    //  object["colG"] = data[dataNo * 3 + 1];
    //  object["colB"] = data[dataNo * 3 + 2];
  }
  serializeJson(doc, jsonString); // convert JSON object to string
  Serial.printf("AP sending JSON doc of size %i: \n",  doc.size());
  Serial.println(jsonString);         // print JSON string to console for debug purposes (you can comment this out)
  webSocket.broadcastTXT(jsonString); // send JSON string to clients
}

void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length)
{
  Serial.println("AP webSocketEvent");
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.printf("[%u] disconnected!\n", num);
    break;
  case WStype_CONNECTED:
  {
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[%u] Connection from ", num);
    Serial.println(ip.toString());
  }
  break;
  case WStype_TEXT:
    Serial.printf("[%u] Text: %s\n", num, payload);
    // webSocket.sendTXT(num, payload);

    parseJsonMessage(payload);
    break;
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
    const int rand1 = doc_rx["rand1"];
    const int rand2 = doc_rx["rand2"];
    Serial.println("AP received sth");
    Serial.println("rand1:" + String(rand1));
    Serial.println("rand2:" + String(rand2));
  }
  Serial.println("");
}

void loop()
{
  server.handleClient(); // Needed for the webserver to handle all clients

  artnet.read();
  webSocket.loop(); // Update function for the webSockets

  // unsigned long now = millis(); // read out the current "time" ("millis()" gives the time in ms since the Arduino started)
  // if ((unsigned long)(now - previousMillis) > interval)
  // { // check if "interval" ms has passed since last time the clients were updated
  //   generateDummyData();

  //   String jsonString = "";                   // create a JSON string for sending data to the client
  //   StaticJsonDocument<200> doc;              // create a JSON container
  //   JsonObject object = doc.to<JsonObject>(); // create a JSON Object

  //   object["ledNum"] = DummyData.ledNum;
  //   object["colR"] = DummyData.colR;
  //   object["colG"] = DummyData.colG;
  //   object["colB"] = DummyData.colB;

  //   serializeJson(doc, jsonString); // convert JSON object to string
  //   Serial.print("AP sending JSON data: ");
  //   Serial.println(jsonString);         // print JSON string to console for debug purposes (you can comment this out)
  //   webSocket.broadcastTXT(jsonString); // send JSON string to clients

  //   previousMillis = now; // reset previousMillis
  // }
}

void generateDummyData()
{
  DummyData.ledNum = random(500);
  DummyData.colR = random(255);
  DummyData.colG = random(255);
  DummyData.colB = random(255);
  // Serial.printf("OUTGOING LedNum:\t[%u] | R:[%u] | G: [%u] | B: [%u]\n", DummyData.ledNum, DummyData.colR, DummyData.colG, DummyData.colB);
}
