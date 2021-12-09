#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_GPS.h>
#include <math.h>
#include <RTCZero.h>

/*
  Lora Tracker Code (Low power mode with sleeping micro controller, no manual tracking on demand)
  Author: Alan Ko
  Replace the variables below before flashing the code onto the tracker.
*/

//CHANGE THESE VARIABLES AS NEEDED
//NOTE: gpsTrackerID should be 0, 1, or 2. No two trackers should have the same ID
int timeZoneDifference = -5; //The time difference between Greenwich time and your local time (-5 for Eastern Time)
unsigned long pingIntervalSeconds = 30; //interval between waking up chip and sending a ping in seconds
String gpsTrackerID = "0"; //tracker number

//DO NOT CHANGE ANY VARIABLES BELOW THIS
const long frequency = 915E6;  // LoRa Frequency
const int csPin = 8;          // LoRa radio chip select
const int resetPin = 4;        // LoRa radio reset
const int irqPin = 3;          // change for your board; must be a hardware interrupt pin
String receivedMessage = ""; //variable used to store incoming messages from the gateway
// what's the name of the hardware serial port?
#define GPSSerial Serial1
// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);
uint32_t timer; //used for timeouts
/* Create an rtc object */
RTCZero rtc;
/* NO NEED TO CHANGE THESE VALUES */
const byte seconds = 0;
const byte minutes = 00;
const byte hours = 00;
const byte day = 24;
const byte month = 9;
const byte year = 16;
bool isTrackerAwake = true;

void setup() {
  LoRa.setPins(csPin, resetPin, irqPin);
  //setup GPS and set to standby mode
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  GPS.standby();
  
  delay(20000); //delay so we can see normal current draw
  pinMode(LED_BUILTIN, OUTPUT); //set LED pin to output
  digitalWrite(LED_BUILTIN, LOW); //turn LED off
  //trackerStandby(pingIntervalSeconds);
}

void loop() {
  if(isTrackerAwake) {
    //begin lora
    if (!LoRa.begin(frequency)) {
      while (true);                       // if failed, do nothing
    }
    LoRa.onReceive(onReceive);
    LoRa.onTxDone(onTxDone);
    LoRa_rxMode();
    
    bool isLocationSent = sendLocation(); //send tracker location
    
    timer = millis(); // reset the timer
    while(millis() - timer < 3000) { //give 3 seconds to receive ping interval updates from gateway
      if(!receivedMessage.equals("")) { // a message has been received
        if (gpsTrackerID.equals(receivedMessage.substring(0, receivedMessage.indexOf(",")))) { //this is the right tracker to update
          String str = receivedMessage.substring(receivedMessage.indexOf(",") + 1, receivedMessage.length() - 3);
          unsigned long newPingInterval = stringToLong(str);
          pingIntervalSeconds = newPingInterval;
        }
        receivedMessage = ""; //clear the received message
        break;
      }
    }
    
    trackerStandby(pingIntervalSeconds);
  }
}

bool sendLocation() {
  GPS.wakeup();
  timer = millis(); // reset the timer
  while(1) { //loop to find a fix
    if (millis() - timer > 120000) { //two minute timeout case
      Serial.println("Two minute timeout reached, no fix found");
      LoRa_sendMessage(gpsTrackerID + "," + "X"); // send a failure message
      GPS.standby();
      delay(500);
      return false;
    }
    while (GPS.available()) { //read in the GPS data
        char c = GPS.read();
    }
    if (GPS.newNMEAreceived()) {  // check for new nmea arrived
        if (GPS.parse(GPS.lastNMEA()) && GPS.fix == 1) { //parse data and see if we have a fix
            break;
        } 
    }
  }
  //now that we have found a fix, format gps data and send it via lora
  float gpsLongitude = gpsToDecimal(GPS.longitude, (String)GPS.lon);
  float gpsLatitude = gpsToDecimal(GPS.latitude, (String)GPS.lat);
  //build the time field
  String gpsTime = "";
  int GPS_hour_adjusted = GPS.hour + timeZoneDifference;
  if (GPS_hour_adjusted < 10) { gpsTime.concat('0'); }
  gpsTime.concat(GPS_hour_adjusted); gpsTime.concat(':');
  if (GPS.minute < 10) { gpsTime.concat('0'); }
  gpsTime.concat(GPS.minute); gpsTime.concat(':');
  if (GPS.seconds < 10) { gpsTime.concat('0'); }
  gpsTime.concat(GPS.seconds);
  //build the date field
  String gpsDate = "";
  gpsDate.concat(GPS.month); gpsDate.concat('/');
  gpsDate.concat(GPS.day); gpsDate.concat("/20");
  gpsDate.concat(GPS.year);
  String gpsInfo = gpsTrackerID + ",*," + String(gpsLatitude, 6) + "," + String(gpsLongitude, 6) + "," + gpsTime + "," + gpsDate;
  //send message via lora
  LoRa_sendMessage(gpsInfo); // send a message
  //put gps into standby mode and return true
  GPS.standby();
  delay(500);
  return true;
}

float gpsToDecimal(float ddm, String quadrant) {
  int degree = (int)(ddm/100);
  float minutes = fmod(ddm,100);
  float decimal_degrees = minutes/60;
  float dd = degree + decimal_degrees;
  if (quadrant == "S" || quadrant == "W") {
    dd = -dd;
  }
  return dd;
}

unsigned long long stringToLong(String s)
{
   char arr[12];
   s.toCharArray(arr, sizeof(arr));
   return atoll(arr);
}

//interrupt service routine (ISR), called when interrupt is triggered 
//executes after MCU wakes up
void ISR()
{
  isTrackerAwake = true;
}

//sets tracker to standby mode
void trackerStandby(unsigned long secondsToSleep) {
  isTrackerAwake = false;
  unsigned long hrs = secondsToSleep / ((int)3600);
  secondsToSleep = secondsToSleep % 3600;
  unsigned long mins = secondsToSleep / ((int)60);
  unsigned long secs = secondsToSleep % 60;
  LoRa.end();
  rtc.begin(); //Start RTC library, this is where the clock source is initialized
  rtc.setTime(hours, minutes, seconds); //set time
  rtc.setDate(day, month, year); //set date
  rtc.setAlarmTime(hrs, mins, secs); //set alarm time to go off in secondsToSleep
  rtc.enableAlarm(rtc.MATCH_HHMMSS); //set alarm
  rtc.attachInterrupt(ISR); //creates an interrupt that wakes the SAMD21 which is triggered by a FTC alarm
  //puts SAMD21 to sleep
  rtc.standbyMode(); //library call
}

void LoRa_rxMode(){
  LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.disableInvertIQ();               // normal mode
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
