# Data collection and Transmission code
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

// Include the Firebase libraries and the required addons
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Replace with your network credentials
#define WIFI_SSID "HAKUNA BATATA"
#define WIFI_PASSWORD "zpto5838"

// Replace with your Firebase project API Key
#define API_KEY "AIzaSyAPXz0znPyQo3-IeZ-VTS-STG_PWTMcB_g"

// Replace with your RTDB URL
#define DATABASE_URL "https://uv-monitoring-device-1a8a7-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Define Firebase Data object
FirebaseData fbdo; // Declare the FirebaseData object globally

FirebaseAuth auth;
FirebaseConfig config;

// Define the pin for reading UV sensor value
#define SENSOR_PIN A0

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
bool initialTransmissionComplete = false; // Flag to track initial transmission

void setup(){
  Serial.begin(115200); // Set the baud rate for Serial communication
  Serial.println("Starting...");

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  // Initialize Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Firebase sign up successful");
    signupOK = true;
  }
  else{
    Serial.println("Firebase sign up failed");
    Serial.printf("Error: %s\n", config.signer.signupError.message.c_str());
  }

  // Set token status callback
  config.token_status_callback = tokenStatusCallback;

  // Initialize Firebase connection
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop(){
  // Read analog sensor value and send data to Firebase every 15 seconds
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    // Read analog sensor value
    int sensorValue = analogRead(SENSOR_PIN);
    float sensorVoltage = sensorValue / 1024.0 * 5.0;

    // Write UV sensor value to Firebase
    if (Firebase.RTDB.setInt(&fbdo, "uv_sensor/value", sensorValue)){
      Serial.println("UV sensor value sent to Firebase");
    }
    else {
      Serial.println("Failed to send UV sensor value to Firebase");
      Serial.printf("Error reason: %s\n", fbdo.errorReason().c_str());
    }

    // Check if initial transmission is complete
    if (initialTransmissionComplete) {
      // Increment count for testing
      count++;
      
      // Write a random float value to Firebase for testing
      if (Firebase.RTDB.setFloat(&fbdo, "test/float", 0.01 + random(0,100))){
        Serial.println("Random float value sent to Firebase");
      }
      else {
        Serial.println("Failed to send random float value to Firebase");
        Serial.printf("Error reason: %s\n", fbdo.errorReason().c_str());
      }
    }
    else {
      // Set flag to indicate initial transmission is complete
      initialTransmissionComplete = true;
    }
  }

  // Read analog sensor value and print to Serial
  float sensorVoltage; 
  float sensorValue;
 
  sensorValue = analogRead(A0);
  sensorVoltage = sensorValue / 1024 * 5.0;
  Serial.print("sensor reading = ");
  Serial.print(sensorValue);
  Serial.print("        sensor voltage = ");
  Serial.print(sensorVoltage);
  Serial.println(" V");
  delay(1000);
}
