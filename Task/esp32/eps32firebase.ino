#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "MIKUMIKU"
#define WIFI_PASSWORD "zzzzzzzz"

#define API_KEY "AIzaSyB4Wb4mhgFwE85ytkYYJC5wctbBjdOKOp8"

#define DATABASE_URL "https://cobaset-352d6-default-rtdb.firebaseio.com/" 

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
unsigned long wifiCheckPrevMillis = 0;
int count = 0;
bool signupOK = false;

const int ledPin = 2; 

void setup(){
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    digitalWrite(ledPin, !digitalRead(ledPin)); 
    delay(300);
  }
  digitalWrite(ledPin, HIGH); 
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;

  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; 
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop(){
  unsigned long currentMillis = millis();

  if (currentMillis - wifiCheckPrevMillis >= 10000) {
    wifiCheckPrevMillis = currentMillis;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Connection lost. Reconnecting to Wi-Fi");
      while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        digitalWrite(ledPin, !digitalRead(ledPin)); 
        delay(1000);
      }
      digitalWrite(ledPin, HIGH);
      Serial.println();
      Serial.print("Reconnected with IP: ");
      Serial.println(WiFi.localIP());
      Serial.println();
    }
  }

  if (Firebase.ready() && signupOK && (currentMillis - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = currentMillis;
    if (Firebase.RTDB.setInt(&fbdo, "test/int", count)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    count++;
  }
}