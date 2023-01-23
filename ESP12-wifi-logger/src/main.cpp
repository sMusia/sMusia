/*
ESP01 LOGGER
05.10.2020
ALEXANDER MUSIIENKO

*/
//#include <NTPClient.h>
#include <ESP8266WiFi.h>
//#include <PubSubClient.h>
#include "GyverTimer.h"
#include "CTBot.h"
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include "Gyverbutton.h"
#include <ArduinoOTA.h>

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

// создать таймер, в скобках период в миллисекундах
GTimer_ms Timer1(1000);                                 // таймер для опроса бота
GTimer_ms Timer2(300);                                 // таймер для опроса датчика
GTimer_ms Timer3(1000*60*15);                                 // таймер для опроса датчика

String DeviceName;                             // Имя устройста которым будем управлять

// Update these with values suitable for your network.
const char* ssid = "MusiaNetwork";                      // WIFI ssid  
const char* password = "zironkamorcheeba";              // WIFi пароль
#define MOVE_pin 5                                       // пин для датчика движения
#define SPK_pin 4

// Initialize Telegram BOT
String token = "1005940097:AAEwVd4ZfqxAT7gQMPIo1A78Eh0fw5c-vHY";        // токен для телеграм бота 
String adm;        // токен для телеграм бота 
CTBotReplyKeyboard telegramKbdCommand;                                  // ответная клавиатура с командами
int admin_id;                                             // id администратора

String message;
unsigned long move_time;
TBMessage msg;

WiFiClient espClient;
CTBot myBot;

void ring(int count){
  for(int i = 0;i<count;i++){
    tone(SPK_pin,2000,100);
    delay(200);
  } 
}

void setup_wifi() {
  int period=0;
  Serial.println("Connecting to WiFi.."); 
  WiFi.mode(WIFI_STA);  
  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      period++;
        if (period>1000){
        Serial.println("Не удалось подлючиться к WIFI - рестарт...");  
        ESP.restart(); period=0;}
      }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  ring(3);

  Serial.println("bot.....");  
  myBot.setTelegramToken(token);
  Serial.println("bot YES ");
  if (myBot.testConnection())
		Serial.println("\ntestConnection OK");
	else
		Serial.println("\ntestConnection NOK");
}

// Проверяем сообщения для бота
void telegram_getmessage(){    
String resp;
  if (myBot.getNewMessage(msg)) {    // Если пришло сообщение
    
    // Если пришло "/status"...
            if (msg.text.equalsIgnoreCase("/status")){              
              unsigned long last_move=((millis()-move_time)/60000) ;
              resp="Последнее движение зафиксировано " + String(last_move) + " минут назад";
              myBot.sendMessage(msg.sender.id, resp,telegramKbdCommand); 
              }

    // Если пришло "/info"...
            if (msg.text.equalsIgnoreCase("/info")){
              long rssi = WiFi.RSSI();
              resp= "Conected to SSID: "        +     WiFi.SSID()                   +       "\n"              
                  + "Local IP: "                +     WiFi.localIP().toString()     +       "\n"
                  + "MAC: "                     +     WiFi.macAddress()             +       "\n"
                  + "DNS: "                     +     WiFi.dnsIP().toString()       +       "\n"
                  + "signal strength (RSSI):"   +     rssi                          +       " dBm" +  "\n"
                  + "your id:"                  +     msg.sender.id; 
              myBot.sendMessage(msg.sender.id, resp,telegramKbdCommand);              
            }            
    }
}

void setup() {
Serial.begin(115200);
Serial.println("Begining.....");  

setup_wifi();

  // No authentication by default
  ArduinoOTA.setPassword("157953");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");  //  "Начало OTA-апдейта"
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");  //  "Завершение OTA-апдейта"
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));

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

  pinMode(MOVE_pin, INPUT); // Пин кнопки
  pinMode(SPK_pin,OUTPUT);
  
  // telegram keyboard Commands
  telegramKbdCommand.addButton("/info");   
  telegramKbdCommand.addButton("/status");   
  telegramKbdCommand.enableResize();
  // Start telegram bot  
    
// Initialize a server
}

void loop() {
  ArduinoOTA.handle();                      // проверяем обновление для прошивки по воздуху 
  
  if (WiFi.status() != WL_CONNECTED) {      // проверяем соединение WIFI
    setup_wifi();  
    }

  if (Timer1.isReady()) {                   // Таймер на 1 секунду - опрос питания  
    telegram_getmessage();                    // проверяем новые сообщения для бота    
    }
  if (Timer2.isReady()) {                     
    if (digitalRead(MOVE_pin)){
      move_time=millis();
      //ring(1);
    }                  
    }
    if (Timer3.isReady()) {   
        if (!myBot.testConnection()) {
            ring(1);
            //Serial.println("Не удалось подлючиться к ТЕЛЕГЕ - рестарт...");  
            ESP.restart();
        };
    }
}

