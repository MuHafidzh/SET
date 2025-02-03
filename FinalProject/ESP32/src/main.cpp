#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HardwareSerial.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "MIKUMIKU"
#define WIFI_PASSWORD "zzzzzzzz"

#define API_KEY "AIzaSyA3l-RGeOyD4A5QCBQpGEcha6il5t2I2go"

#define DATABASE_URL "https://clock-af5bb-default-rtdb.firebaseio.com/" 

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long wifiCheckPrevMillis = 0;
unsigned long recvDataPrevMillis = 0;
bool signupOK = false;

const int ledPin = 2; 

LiquidCrystal_I2C lcd(0x27, 16, 2);


String Clock = "";
int ClockInt = 0;
long frequency = 0;

HardwareSerial ESPSerial(1);
const int ESPSerialRx = 16;
const int ESPSerialTx = 17;

void setup(){
  Serial.begin(115200);
  ESPSerial.begin(115200, SERIAL_8N1, ESPSerialRx, ESPSerialTx);
  pinMode(ledPin, OUTPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    lcd.print(".");
    digitalWrite(ledPin, !digitalRead(ledPin)); 
    delay(300);
  }
  delay(1000);
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(2000);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("SignUp OK");
    signupOK = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SignUp OK");
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SignUp Error:");
    lcd.setCursor(0, 1);
    lcd.print(config.signer.signupError.message.c_str());
    delay(2000);
  }
  delay(1000);

  config.token_status_callback = tokenStatusCallback; 
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  delay(2000); 
}

void loop(){

  if (millis() - wifiCheckPrevMillis >= 10000) {
    wifiCheckPrevMillis = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Connection lost. Reconnecting to Wi-Fi");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Reconnecting");
      while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        lcd.print(".");
        digitalWrite(ledPin, !digitalRead(ledPin)); 
        delay(1000);
      }
      Serial.println();
      Serial.print("Reconnected at IP: ");
      Serial.println(WiFi.localIP());
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Reconnected at IP:");
      lcd.setCursor(0, 1);
      lcd.print(WiFi.localIP());
      delay(2000);
    }
  }

  if (Firebase.ready() && signupOK && (millis() - recvDataPrevMillis > 2000 || recvDataPrevMillis == 0)){
    recvDataPrevMillis = millis();
    if(Firebase.RTDB.getString(&fbdo, "/data/clk")) {
      if (fbdo.dataType() == "string") {
        Clock = fbdo.stringData();
        Clock.replace("\\", "");
        Clock.replace("\"", "");
        Serial.println(Clock);

        ClockInt = atoi(Clock.c_str());
        frequency = ClockInt;
     
      }
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Frequency: ");
    lcd.setCursor(0, 1);
    lcd.print(ClockInt);
    lcd.print(" Hz");

    uint8_t serial_send[15] = {'i', 't', 's'};
    memcpy(serial_send+3, &frequency, sizeof(frequency));

    ESPSerial.write(serial_send, sizeof(serial_send));
  }
}