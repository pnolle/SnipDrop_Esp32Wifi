#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

// SSID and password of Wifi connection:
const char* ssid = "SnipDrop";
const char* password = "rrndGrl23";

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

String webpage = "<!DOCTYPE html><html><head><title>Websocket bi-directional</title></head><body style='background-color: #EEEEEE;'><span style='color: #003366;'><h1>Randomizr</h1><p>Random number: <span id='rand'>-</span></p><button type='button' id='BTN_SEND_BACK'>Send info to ESP32</button></span></body><script>var Socket; document.getElementById('BTN_SEND_BACK').addEventListener('click', button_send_back); function button_send_back(){ Socket.send('Sending back stuff');} function init(){ Socket=new WebSocket('ws://' + window.location.hostname + ':81/'); Socket.onmessage=function (event){ processCommand(event);};} function processCommand(event){ document.getElementById('rand').innerHTML=event.data; console.log(event.data);} window.onload=function (event){ init();}</script></html>";
int interval = 1000;
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);                 
 
  WiFi.begin(ssid, password);
  Serial.println("Establishing connection to WiFi with SSID: " + String(ssid));
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.print("Connected to network with IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", []() {
    server.send(200, "text\html", webpage);
  });
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("Client disconnected");
      break;
    case WStype_CONNECTED:
      Serial.println("Client connected");
      break;
    case WStype_TEXT:
      for (int i=0; i<length; i++) {
        Serial.print((char)payload[i]);
      }
      Serial.println("");
      break;
  }
}
 
void loop() {
  server.handleClient();
  webSocket.loop();

  unsigned long now = millis();
  if (now - previousMillis > interval) {
    String str = String(random(100));
    int str_len = str.length() + 1;
    char char_array[str_len];
    str.toCharArray(char_array, str_len);
    webSocket.broadcastTXT(char_array);
    previousMillis = now;
  }
}
