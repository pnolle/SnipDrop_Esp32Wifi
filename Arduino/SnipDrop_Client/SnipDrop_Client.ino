/*
  SnipDrop Client

  Establishing connection to WiFi with SSID: SnipDrop
  Connected to network with IP address: 192.168.1.23
  Client has MAC address: C0:49:EF:CF:AD:FC

*/

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "secrets.h"  // local variables

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

String webpage = "<!DOCTYPE html><html><head><title>Json Websocket</title></head><body style='background-color: #EEEEEE;'><span style='color: #003366;'><h1>Randomizr</h1><p>Random number: <p><span id='rand1'>-</span></p><p><span id='rand2'>-</span></p></p><button type='button' id='BTN_SEND_BACK'>Send info to ESP32</button></span></body><script>var Socket; document .getElementById('BTN_SEND_BACK') .addEventListener('click', button_send_back); function button_send_back(){ var light_details={ ledNum: 1, r: 2, g: 3, b: 4,}; Socket.send(JSON.stringify(light_details));} function init(event){ Socket=new WebSocket('ws://' + window.location.hostname + ':81/'); Socket.onmessage=function (event){ processCommand(event);};} function processCommand(event){ var obj=JSON.parse(event.data); document.getElementById('rand1').innerHTML=obj.rand1; document.getElementById('rand2').innerHTML=obj.rand2; console.log(obj.rand1); console.log(obj.rand2);} window.onload=function (event){ init(event);}; </script></html>";
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

  server.on("/", []()
            { server.send(200, "text\html", webpage); });
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length)
{
  Serial.println("Client webSocketEvent");
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
    DeserializationError error = deserializeJson(doc_rx, payload);
    if (error)
    {
      Serial.println("deserializeJson() failed");
      return;
    }
/*     else
    {
      const int ledNum = doc_rx["ledNum"];
      const int r = doc_rx["r"];
      const int g = doc_rx["g"];
      const int b = doc_rx["b"];
      Serial.println("Received lights info:");
      Serial.println("ledNum:" + String(ledNum));
      Serial.println("r:" + String(r));
      Serial.println("g:" + String(g));
      Serial.println("b:" + String(b));
    } */
    else {
      const int apRand1 = doc_rx["apRand1"];
      const int apRand2 = doc_rx["apRand2"];
      Serial.println("Client received sth");
      Serial.println("apRand1:" + String(apRand1));
      Serial.println("apRand2:" + String(apRand2));
    }
    break;
  }
}

void loop()
{
  server.handleClient();
  webSocket.loop();

  unsigned long now = millis();
  if (now - previousMillis > interval)
  {
    String jsonString = "";
    JsonObject object = doc_tx.to<JsonObject>();
    object["rand1"] = random(100);
    object["rand2"] = random(100);
    serializeJson(doc_tx, jsonString);
    Serial.print("Client sending JSON data: ");
    Serial.println(jsonString);
    webSocket.broadcastTXT(jsonString);
    previousMillis = now;
  }
}
