/*
  SnipDrop Client

  Establishing connection to WiFi with SSID: SnipDrop

  My devices:

  Connected to network with IP address: 192.168.1.23
  Client has MAC address: C0:49:EF:CF:AD:FC

  Connected to network with IP address: 192.168.1.24
  Client has MAC address: 34:86:5D:FC:80:B4

*/

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "secrets.h" // local variables

WebServer server(80);
WebSocketsClient webSocket;

String webpage = "<!DOCTYPE html><html><head><title>Client</title></head><body style='background-color: #EEEEEE;'><span style='color: #003366;'><h1>Randomizr</h1><p>Random number: <p><span id='rand1'>-</span></p><p><span id='rand2'>-</span></p></p><button type='button' id='BTN_SEND_BACK'>Send info to ESP32</button></span></body><script>var Socket; document .getElementById('BTN_SEND_BACK') .addEventListener('click', button_send_back); function button_send_back(){ var light_details={ ledNum: 1, r: 2, g: 3, b: 4,}; Socket.send(JSON.stringify(light_details));} function init(event){ Socket=new WebSocket('ws://' + window.location.hostname + ':81/'); Socket.onmessage=function (event){ processCommand(event);};} function processCommand(event){ var obj=JSON.parse(event.data); document.getElementById('rand1').innerHTML=obj.rand1; document.getElementById('rand2').innerHTML=obj.rand2; console.log(obj.rand1); console.log(obj.rand2);} window.onload=function (event){ init(event);}; </script></html>";
int interval = 1000;
unsigned long previousMillis = 0;

StaticJsonDocument<200> doc_tx;
StaticJsonDocument<200> doc_rx;

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Establishing connection to WiFi with SSID: " + String(ssid));

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.print("Connected to network with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Client has MAC address: ");
  Serial.println(WiFi.macAddress());

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
  Serial.println("Client webSocketEvent");
  if (type == WStype_TEXT)
  {
    Serial.printf("Text: %s\n", payload);
    parseJsonMessage(payload);
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
    const int apRand1 = doc["apRand1"];
    const int apRand2 = doc["apRand2"];
    Serial.println("Client received sth");
    Serial.println("apRand1:" + String(apRand1));
    Serial.println("apRand2:" + String(apRand2));
  }
  Serial.println("");
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
