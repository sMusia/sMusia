#include <Arduino.h>
#include "max6675.h"
#include <GyverTimer.h>
#include <ArduinoOTA.h>
#include <GyverButton.h>

#define BUT1_PIN 0
#define BUT2_PIN 2
GButton BUT1_BUT(BUT1_PIN);
GButton BUT2_BUT(BUT2_PIN);

#define MIN_TEMP 150
#define MAX_TEMP 400

int thermoDO = 14;
int thermoCS = 12;
int thermoCLK = 13;

int target_temp = 250;
int td;
int br=0;
bool up;
#define RELAY 16
#define SPEAKER 15

GTimer_ms Timer1;

#define CLK 5
#define DIO 4

#include "GyverTM1637.h"
GyverTM1637 disp(CLK, DIO);

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

const char *ssid = "Smart_HOOKAN";
const char *password = "hookanhookan";
bool startup = true;

byte prepare_banner[] = {_P, _r, _E, _P, _A, _r, _E, _empty, _empty, _empty, _empty };
byte service_banner[] = {_S, _E, _r, _U, _i, _c, _E, _empty, _empty, _empty, _empty };

void breathe(){
        if (br>6) {up=false;disp.displayInt(thermocouple.readCelsius());}
        if (br<1) {up=true;}
        if (up) {br++;} else {br--;}       
        delay(100);
        disp.brightness(br);
}

void start_sound(){
    tone(SPEAKER,500,100);
    delay(50);    
    tone(SPEAKER,1000,100);
    delay(50);
    tone(SPEAKER,1500,100);
    delay(50);
}

void ready_sound(){
    tone(SPEAKER,1000,50);
    delay(200);    
    tone(SPEAKER,1000,50);
    delay(200);
    tone(SPEAKER,1000,50);
    delay(200);
}

void preparing(){
  disp.runningString(prepare_banner, sizeof(prepare_banner), 100);  // выводим
  disp.displayInt(thermocouple.readCelsius());
  digitalWrite(RELAY,HIGH); 
  while (thermocouple.readCelsius()<240)
  {
    disp.displayInt(thermocouple.readCelsius());    
    delay(100);
    breathe();
  }
  digitalWrite(RELAY,LOW); 
  delay(5000);
  ready_sound();  
  startup=false;
  disp.brightness(7);
}

void service(){
  tone(SPEAKER,500,500);
  disp.runningString(service_banner, sizeof(service_banner), 100);  // выводим
  delay(1000);    
  while (true)
  {
    ArduinoOTA.handle();                      // проверяем обновление для прошивки по воздуху
    disp.runningString(service_banner, sizeof(service_banner), 100);  // выводим
  }
}

void setup() {
  tone(SPEAKER,200,50);
  delay(500);
  //Serial.begin(9600);
  disp.clear();
  pinMode(RELAY,OUTPUT);
  disp.brightness(7);  // яркость, 0 - 7 (минимум - максимум)

  WiFi.softAP(ssid, password);

  Timer1.setInterval(1000);
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setPassword("157953");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");  //  "Начало OTA-апдейта"
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");  //  "Завершение OTA-апдейта"
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100))); 
    disp.displayInt((progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    //  "Ошибка при аутентификации"
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    //  "Ошибка при начале OTA-апдейта"
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    //  "Ошибка при подключении"
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    //  "Ошибка при получении данных"
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
    //  "Ошибка при завершении OTA-апдейта"
  });
  ArduinoOTA.begin();

  digitalWrite(RELAY,LOW);
  delay(100);
  start_sound(); 
  if (!digitalRead(BUT1_PIN)) {service();} else {preparing();}
}



void display_current_temp(){
// put your main code here, to run repeatedly:
  if (Timer1.isReady()) {
    int temperature=thermocouple.readCelsius();
    disp.displayInt(temperature);
    //digitalWrite(RELAY,!digitalRead(RELAY)); 
    if (temperature<target_temp-5){        
        digitalWrite(RELAY,HIGH);
      };
    if (temperature>target_temp){
      disp.brightness(7);
      digitalWrite(RELAY,LOW);};
  }
}

void display_target_temp(){
  disp.point(POINT_OFF);
  disp.displayInt(target_temp);
}

void loop() {
  BUT1_BUT.tick();  // обязательная функция отработки Кнопки1
  BUT2_BUT.tick();  // обязательная функция отработки Кнопки2
  
  ArduinoOTA.handle();                      // проверяем обновление для прошивки по воздуху 

  if  ((BUT2_BUT.isPress()) or (BUT2_BUT.isHold())) {
    if (target_temp<MAX_TEMP){target_temp++; td=millis(); delay(100);}
  }
  if  ((BUT1_BUT.isPress()) or (BUT1_BUT.isHold())) {
    if (target_temp>MIN_TEMP){target_temp--; td=millis(); delay(100);}
  }
  if (digitalRead(RELAY)) {breathe();}
  if (millis()>td+2000){display_current_temp();} else {display_target_temp();};
   
  }