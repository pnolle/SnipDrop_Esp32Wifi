/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-two-way-communication-esp32/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH THE MAC Address of your receiver 
// uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
// uint8_t broadcastAddress[] = {0x40, 0x22, 0xD8, 0x5F, 0xD7, 0xDC};  // AP
uint8_t broadcastAddress[] = {0xC0, 0x49, 0xEF, 0xCF, 0xAD, 0xFC};  // C1


// Define variables to store BME280 readings to be sent
uint16_t ledNum;
uint8_t colR;
uint8_t colG;
uint8_t colB;

// Define variables to store incoming readings
uint16_t incomingLedNum;
uint8_t incomingColR;
uint8_t incomingColG;
uint8_t incomingColB;

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    uint16_t ledNum;
    uint8_t colR;
    uint8_t colG;
    uint8_t colB;
} struct_message;

// Create a struct_message for dummy data
struct_message DummyData;
// Create a struct_message to hold incoming data
struct_message incomingReadings;

esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.printf("Bytes received:\t[%i]\n", len);
  incomingLedNum = incomingReadings.ledNum;
  incomingColR = incomingReadings.colR;
  incomingColG = incomingReadings.colG;
  incomingColB = incomingReadings.colB;
  updateDisplay();
}

void updateDisplay(){
  Serial.printf("INCOMING LedNum:\t[%u] | R:[%u] | G: [%u] | B: [%u]\n", incomingLedNum, incomingColR, incomingColG, incomingColB);
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
  generateDummyData();
  DummyData.ledNum = ledNum;
  DummyData.colR = colR;
  DummyData.colG = colG;
  DummyData.colB = colB;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &DummyData, sizeof(DummyData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(3000);
}

void generateDummyData(){
  ledNum = random(500);
  colR = random(255);
  colG = random(255);
  colB = random(255);
  Serial.printf("OUTGOING LedNum:\t[%u] | R:[%u] | G: [%u] | B: [%u]\n", ledNum, colR, colG, colB);
}
