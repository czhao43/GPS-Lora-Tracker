#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

/*
  Lora Gateway Code (Low power mode with sleeping micro controller, no manual tracking on demand)
  Author: Alan Ko
  Replace the WIFI ssid and password variables below before flashing the code onto the tracker.
*/

//ENTER YOUR WIFI SSID AND PASSWORD INTO THESE VARIABLES
char ssid[] = "";
char wifiPass[] = "";

//ENTER THE WEBHOOK URLS FROM MONGO DB INTO THESE VARIABLES
//http server name to POST to mongoDB
char POSTserverName[] = "https://us-east-1.aws.webhooks.mongodb-realm.com/api/client/v2.0/app/application-0-xzveg/service/gatewayAPI/incoming_webhook/POSTwebhook";
//http server name to GET from mongoDB
char GETserverName[] = "https://us-east-1.aws.webhooks.mongodb-realm.com/api/client/v2.0/app/application-0-xzveg/service/gatewayAPI/incoming_webhook/GETwebhook";

//DO NOT CHANGE ANY VARIABLES BELOW THIS
//json document
StaticJsonDocument<500> doc;
const long frequency = 915E6;  // LoRa Frequency
String receivedMessage = "";  //used to store incoming messages from trackers
const int csPin = 18;          // LoRa radio chip select
const int resetPin = 23;        // LoRa radio reset
const int irqPin = 26;          // change for your board; must be a hardware interrupt pin
uint32_t GETtimer = millis(); //timer for making get call to mongoDB
String newPingInterval0 = ""; //if there is a request to change tracker 0's ping interval, it is stored in this variable
String newPingInterval1 = "";
String newPingInterval2 = "";

void setup() {
  Serial.begin(115200);                   // initialize serial
  while (!Serial);

  //set up lora
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(frequency)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  Serial.println("LoRa init succeeded.");
  Serial.println();
  Serial.println("LoRa Simple Gateway");
  Serial.println("Only receive messages from nodes");
  Serial.println("Tx: invertIQ enable");
  Serial.println("Rx: invertIQ disable");
  Serial.println();

  //set up wifi
  connectToWifi();

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();

}

void loop() {
  //received data from a tracker
  if(!receivedMessage.equals("")) {
    Serial.print("Gateway Receive: ");
    Serial.println(receivedMessage);
    //get the tracker ID first
    String receivedTrackerID = receivedMessage.substring(0, receivedMessage.indexOf(","));
    
    //check if there is a request to change the tracker's ping interval
    if(receivedTrackerID.equals("0") && !newPingInterval0.equals("")) {
      Serial.println("sending lora signal to tracker " + receivedTrackerID + " to change the ping interval to " + newPingInterval0);
      LoRa_sendMessage(receivedTrackerID + "," + newPingInterval0);
      newPingInterval0 = "";
    } else if (receivedTrackerID.equals("1") && !newPingInterval1.equals("")) {
      Serial.println("sending lora signal to tracker " + receivedTrackerID + " to change the ping interval to " + newPingInterval1);
      LoRa_sendMessage(receivedTrackerID + "," + newPingInterval1);
      newPingInterval1 = "";
    } else if (receivedTrackerID.equals("2") && !newPingInterval2.equals("")) {
      Serial.println("sending lora signal to tracker " + receivedTrackerID + " to change the ping interval to " + newPingInterval2);
      LoRa_sendMessage(receivedTrackerID + "," + newPingInterval2);
      newPingInterval2 = "";
    } else {
      Serial.println("No ping interval updates for tracker " + receivedTrackerID);
    }
    receivedMessage = receivedMessage.substring(receivedMessage.indexOf(",") + 1);
    
    //check if the gps data is successfully transmitted
    if (receivedMessage.substring(0, receivedMessage.indexOf(",")).equals("*")) { //gps data has successfully been received
      receivedMessage = receivedMessage.substring(receivedMessage.indexOf(",") + 1);
      //parse through the received message and extract the rest of the fields using the commas
      String receivedLatitude = receivedMessage.substring(0, receivedMessage.indexOf(","));
      receivedMessage = receivedMessage.substring(receivedMessage.indexOf(",") + 1);
      String receivedLongitude = receivedMessage.substring(0, receivedMessage.indexOf(","));
      receivedMessage = receivedMessage.substring(receivedMessage.indexOf(",") + 1);
      String receivedTime = receivedMessage.substring(0, receivedMessage.indexOf(","));
      String receivedDate = receivedMessage.substring(receivedMessage.indexOf(",") + 1); 
      //build json doc
      doc["trackerID"] = receivedTrackerID;
      doc["location"]["lat"] = receivedLatitude.toFloat();
      doc["location"]["lng"] = receivedLongitude.toFloat();
      doc["gpsDate"] = receivedDate;
      doc["gpsTime"] = receivedTime;
      //post test data
      POSTData();
      serializeJsonPretty(doc, Serial);
      doc.clear();
      Serial.println("\nDone.");
      receivedMessage = "";
    } else if (receivedMessage.substring(0, receivedMessage.indexOf(",")).equals("X")) { // no fix found
      Serial.println("Tracker " + receivedTrackerID + " did not find a fix");
      doc["trackerID"] = receivedTrackerID;
      doc["error"] = "fix";
      //post test data
      POSTData();
      serializeJsonPretty(doc, Serial);
      doc.clear();
      Serial.println("\nDone.");
      receivedMessage = "";
    } else { //GPS data was corrupted up over lora 
      Serial.println("GPS data corrupted during lora transmission");
      //we can do more stuff here if we need to, such as forwarding the error to the UI
      receivedMessage = "";
    }
  }

  if (millis() - GETtimer > 10000) {
    GETtimer = millis(); // reset the timer
    GETdata();
    String requestType = doc["requestType"]; 
    String trackerID = doc["trackerID"];
    if(requestType.equals("setInterval")) {
      String newInterval = doc["newInterval"];
      //update the correct interval based on trackerID
      if(trackerID.equals("0")) {
        newPingInterval0 = newInterval;
        Serial.print("new interval for tracker 0 ready to be set: "); Serial.println(newPingInterval0);
      } else if(trackerID.equals("1")) {
        newPingInterval1 = newInterval;
        Serial.print("new interval for tracker 1 ready to be set: "); Serial.println(newPingInterval1);
      } else if(trackerID.equals("2")) {
        newPingInterval2 = newInterval;
        Serial.print("new interval for tracker 2 ready to be set: "); Serial.println(newPingInterval2);
      }
    }
    doc.clear();
  }
}

void POSTData(){
  Serial.println("Posting...");
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
  
    http.begin(POSTserverName);
    http.addHeader("Content-Type", "application/json");
  
    String json;
    serializeJson(doc, json);
  
    Serial.println(json);
    int httpResponseCode = http.POST(json);
    Serial.println(httpResponseCode);
  }
}

void GETdata() {
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    http.begin(GETserverName);
    int httpCode = http.GET();
    if (httpCode > 0) { //Check for the returning code
        String payload = http.getString();
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }
    } else {
      Serial.println("Error on HTTP request");
    }
  }
}

void connectToWifi() {
  Serial.print(WiFi.macAddress());
  Serial.print(" Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, wifiPass);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void LoRa_rxMode(){
  LoRa.disableInvertIQ();               // normal mode
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.enableInvertIQ();                // active invert I and Q signals
}

void LoRa_sendMessage(String message) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

void onReceive(int packetSize) {
  while (LoRa.available()) {
    receivedMessage += (char)LoRa.read();
  }
}

void onTxDone() {
  Serial.println("TxDone");
  LoRa_rxMode();
}

boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
