/*
ESP01 PC SWITCHER 
31.03.2020
ALEXANDER MUSIIENKO

*/
//#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
//#include <PubSubClient.h>
#include "GyverTimer.h"
#include "CTBot.h"
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>
#include "Gyverbutton.h"

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

// создать таймер, в скобках период в миллисекундах
GTimer_ms Timer1(500);                                 // таймер для опроса питания и бота
GTimer_ms Timer4(5*60*1000);                           // таймер для проверки бота на поключение

// Define NTP Client to get time
//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP);
//String formattedDate;
String DeviceName;                             // Имя устройста которым будем управлять

// Update these with values suitable for your network.
//const char* ssid = "BARBOSS";                      // WIFI ssid  
//const char* password = "ai4999ae";              // WIFi пароль
//const char* mqttServer = "tailor.cloudmqtt.com";      // mqttServer
//const int mqttPort = 14173;                           // mqttPort
//const char* mqttUser = "tybrjfqz";                    // mqttUser
//const char* mqttPassword = "Tim9CjLAoZ9s";            // mqttPassword
#define PWR_pin 13                                       // пин для управления питанием

// Initialize Telegram BOT
String token;        // токен для телеграм бота 
String adm;        // токен для телеграм бота 
CTBotReplyKeyboard telegramKbdCommand;                                  // ответная клавиатура с командами
int admin_id;                                             // id администратора

String message;
bool PWR_state;
TBMessage msg;
WiFiManager wifiManager;
WiFiClient espClient;
//PubSubClient client(espClient);
CTBot myBot;
ESP8266WebServer server(80);

void setup_wifi() {
  wifiManager.autoConnect();
  Serial.println("bot.....");  
  myBot.setTelegramToken(token);
  	Serial.println("bot YES ");
   Serial.println(admin_id);
   Serial.println(token); 
   Serial.println(DeviceName); 
   if (myBot.testConnection())
		Serial.println("\ntestConnection OK");
	else
		Serial.println("\ntestConnection NOK");
  if (PWR_state) myBot.sendMessage(admin_id, "ESP connected to WIFI. Питание для "+DeviceName+" выключено!");
    else myBot.sendMessage(admin_id, "ESP connected to WIFI. Питание для "+DeviceName+" включено!"); 
    
}

void PWR_ON(){
   if (!PWR_state) myBot.sendMessage(msg.sender.id, "Питание "+DeviceName+" уже включено",telegramKbdCommand);
   else {
      myBot.sendMessage(msg.sender.id, "Включаем питание для "+DeviceName,telegramKbdCommand);
      PWR_state=!PWR_state;
      digitalWrite(PWR_pin,PWR_state);
      EEPROM.begin(1);
      delay(50);
      EEPROM.put(1,PWR_state);
      EEPROM.commit();
      
   }  
  }

void PWR_OFF(){
   if (PWR_state) myBot.sendMessage(msg.sender.id, "Питание "+DeviceName+" уже выключено",telegramKbdCommand);
   else {
      myBot.sendMessage(msg.sender.id, "Выключаем питание для "+DeviceName,telegramKbdCommand);
      PWR_state=!PWR_state;
      digitalWrite(PWR_pin,PWR_state);
      EEPROM.begin(1);
      delay(50);
      EEPROM.put(1,PWR_state);
      EEPROM.commit();
          
   }   
  }

// Проверяем сообщения для бота
void telegram_getmessage(){    
String resp;
  if (myBot.getNewMessage(msg)) {    // Если пришло сообщение
    
    // Если пришло "/status"...
            if (msg.text.equalsIgnoreCase("/status")){              
            if (digitalRead(!PWR_state)) myBot.sendMessage(msg.sender.id, DeviceName+" Выключена",telegramKbdCommand);
               else myBot.sendMessage(msg.sender.id, DeviceName+" Включена",telegramKbdCommand);                
              }

    // Если пришло "/pwr_on"...
            if (msg.text.equalsIgnoreCase("/pwr_on")){
                PWR_ON();
            }
    // Если пришло "/pwr_off"...
            if (msg.text.equalsIgnoreCase("/pwr_off")){
                PWR_OFF();
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
    WiFiManager wifiManager;
    WiFiManagerParameter telegram_id("tel_id", "telegram admin id", "439003274", 15);
    wifiManager.addParameter(&telegram_id);
    WiFiManagerParameter telegram_token("tel_token", "bot token", "1270340636:AAERXaR_g0bDtLwhs5d2-Teqr81rp1O0GnU", 80);
    wifiManager.addParameter(&telegram_token);
    WiFiManagerParameter device_name("dev_name", "Имя Устройства", "Кофемашина", 50);
    wifiManager.addParameter(&device_name);
  pinMode(14,INPUT_PULLUP);
  if (digitalRead(14)==LOW){
    wifiManager.startConfigPortal("+++");
  }

token = (String)telegram_token.getValue();
adm = (String)telegram_id.getValue();
admin_id = adm.toInt();
DeviceName = (String)device_name.getValue();

  pinMode(PWR_pin, OUTPUT); // Пин кнопки
  
  //digitalWrite(LED,LOW);
  EEPROM.begin(1);
  delay(50);
  PWR_state=false;
  EEPROM.get(1,PWR_state);
  digitalWrite(PWR_pin,PWR_state);
  //EEPROM.end();
   // telegram keyboard Commands
  telegramKbdCommand.addButton("/info");   
  telegramKbdCommand.addButton("/status");   
  telegramKbdCommand.addButton("/pwr_on");   
  telegramKbdCommand.addButton("/pwr_off");
  telegramKbdCommand.enableResize();
  // Start telegram bot  
  
  if (PWR_state) myBot.sendMessage(admin_id, "ESP connected to WIFI. Питание для "+DeviceName+" выключено!");
    else myBot.sendMessage(admin_id, "ESP connected to WIFI. Питание для "+DeviceName+" включено!");  
   setup_wifi(); 
// Initialize a server
  httpUpdater.setup(&httpServer,"/update","admin","157953");
  httpServer.begin();
}

void loop() {
  
  httpServer.handleClient();                   // Слушаем клиентов для обновления
  
  if (WiFi.status() != WL_CONNECTED) {      // проверяем соединение WIFI
    setup_wifi();  
    }

  if (Timer1.isReady()) {                   // Таймер на 1 секунду - опрос питания  
    telegram_getmessage();                    // проверяем новые сообщения для бота    
    }

    if (Timer4.isReady()) {   
        if (!myBot.testConnection()) {
            //Serial.println("Не удалось подлючиться к ТЕЛЕГЕ - рестарт...");  
            ESP.restart();
        };
    }
}

