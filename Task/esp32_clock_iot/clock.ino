#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "si5351.h"

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

Si5351 si5351;

long frequency[3] = {0, 0, 0};
String dataClock[3] = {"", "", ""};
int dataClockInt[3] = {0, 0, 0};
String unit[3] = {"", "", ""};

void updateClockData(const char* clkPath, const char* unitPath, si5351_clock clkIndex);

void setup(){
  Serial.begin(115200);
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

  bool i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  if(!i2c_found) {
    Serial.println("I2C not found!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("I2C not found!");
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("I2C found");
  }
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
    updateClockData("/data/clk0", "/data/unit0", SI5351_CLK0);
    updateClockData("/data/clk1", "/data/unit1", SI5351_CLK1);
    updateClockData("/data/clk2", "/data/unit2", SI5351_CLK2);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(dataClockInt[0]);
    lcd.setCursor(0, 1);
    lcd.print(unit[0]);
    lcd.setCursor(5, 0);
    lcd.print(dataClockInt[1]);
    lcd.setCursor(5, 1);
    lcd.print(unit[1]);
    lcd.setCursor(10, 0);
    lcd.print(dataClockInt[2]);
    lcd.setCursor(10, 1);
    lcd.print(unit[2]);
  }
}

void updateClockData(const char* clkPath, const char* unitPath, si5351_clock clkIndex) {
  if(Firebase.RTDB.getString(&fbdo, clkPath)) {
    if (fbdo.dataType() == "string") {
      dataClock[clkIndex] = fbdo.stringData();
      dataClock[clkIndex].replace("\\", ""); 
      dataClock[clkIndex].replace("\"", ""); 
      Serial.println(dataClock[clkIndex]);

      dataClockInt[clkIndex] = atoi(dataClock[clkIndex].c_str());

      if(Firebase.RTDB.getString(&fbdo, unitPath)) {
        if (fbdo.dataType() == "string") {
          unit[clkIndex] = fbdo.stringData();
          unit[clkIndex].replace("\\", ""); 
          unit[clkIndex].replace("\"", ""); 
          Serial.println(unit[clkIndex]); 

          if (unit[clkIndex] == "KHz") {
            frequency[clkIndex] = dataClockInt[clkIndex] * 1000* 100ULL; 
          } else if (unit[clkIndex] == "MHz") {
            frequency[clkIndex] = dataClockInt[clkIndex] * 1000000 * 100ULL;
          }

          si5351.set_freq(frequency[clkIndex], clkIndex); 
        }
      }
    }
  }
}