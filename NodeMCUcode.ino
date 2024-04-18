#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Ultrasonic.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const int moisturePin = D5;

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "realme"
#define WIFI_PASSWORD "QWERTYUIOP"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBneWXK9wj4T3SL8tMmJ5uTSMV0GUyW4F0"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "avinashn157@gmail.com"
#define USER_PASSWORD "Sparkbot"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://sparkbot-ee008-default-rtdb.firebaseio.com/"
// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String distancePath = "/distance";
String timePath = "/timestamp";

// Parent Node (to be updated in every loop)
String parentPath;

FirebaseJson json;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Variable to save current epoch time
int timestamp;

// Ultrasonic sensor pins
#define TRIGGER_PIN D1
#define ECHO_PIN D2
Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 180000;

// Variable to save previous distance value
int previousDistance = -1;

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}

void setup(){

  Serial.begin(115200);

  initWiFi();
  timeClient.begin();

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');

  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  //define led
  pinMode(D4, OUTPUT);
  pinMode(D5, INPUT);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
}

void loop(){

  // Read distance from ultrasonic sensor
  int distance = ultrasonic.distanceRead();


    // Send new readings to database
    if (Firebase.ready()){
      //Get current timestamp
      timestamp = getTime();
      Serial.print ("time: ");
      Serial.println (timestamp);

      parentPath= "/Roverdata";

      json.set(distancePath.c_str(), String(distance));
      json.set(timePath, String(timestamp));
      
      // Update JSON data in Firebase
      Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json);
      
      if (fbdo.dataType() == "json")
      {
        Serial.println("PASSED");
      }
      
      // Check if there was any error
      if (fbdo.httpCode() == 200)
      {
        Serial.println("DONE");
      }
      else
      {
        Serial.println("FAILED");
        Serial.println(fbdo.errorReason());
      }
    }
  
 
  // Read moisture sensor
  int moistureValue = analogRead(moisturePin);
  Serial.print("Moisture Level: ");
  Serial.println(moistureValue);

  // Check moisture level and send data to Firebase
  if (Firebase.ready()) {
    String parentPathMoisture = "/MoistureData";
    json.clear();
    json.set("Moisture", String(moistureValue));
    Firebase.RTDB.setJSON(&fbdo, parentPathMoisture.c_str(), &json);

    if (fbdo.dataType() == "json" && fbdo.httpCode() == 200) {
      Serial.println("Moisture data sent to Firebase successfully");
    } else {
      Serial.println("Failed to send moisture data to Firebase");
      Serial.println(fbdo.errorReason());
    }
  }





  //read data from firebase
    if (Firebase.RTDB.getInt(&fbdo, "/LED_STATUS/LED_STATUS")) {

    if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_integer) {
      Serial.println(fbdo.to<int>());
      if(fbdo.to<int>()> 0){
        digitalWrite(D4, HIGH);
      }
      else{
        digitalWrite(D4, LOW);
      }
    }

  } else {
    Serial.println(fbdo.errorReason());
  }
}
