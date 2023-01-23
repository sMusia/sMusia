/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "GyverTimer.h"
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// создать таймер, в скобках период в миллисекундах
GTimer_ms Timer1(1000);
GTimer_ms Timer2(60000);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;

// Update these with values suitable for your network.
const char* ssid = "IDEADESIGN";
const char* password = "ochendlinniy";
const char* mqttServer = "tailor.cloudmqtt.com";
const int mqttPort = 14173;
const char* mqttUser = "tybrjfqz";
const char* mqttPassword = "Tim9CjLAoZ9s";
#define PWR_pin 2
#define PWR_state 0
int time_delay=millis();
bool prev_state=false;
String message;
int ticker=0;
char currtime[20];

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
Serial.println("Connecting to WiFi..");   
WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
 
void setup() {
Serial.begin(115200);
Serial.println("Begining.....");  
delay(2000);
setup_wifi();
client.setServer(mqttServer, mqttPort);
client.setCallback(callback);
 

  //client.publish("esp/test", "Hello from ESP8266");
  client.subscribe("HOME/COMP/PWR_ON");  
  
  pinMode(PWR_state, INPUT); // Пин диода питания
  pinMode(1, OUTPUT); // Пин диода на ESP-01
  pinMode(PWR_pin, OUTPUT); // Пин кнопки
  digitalWrite(PWR_pin,HIGH);

  // Port defaults to 8266
   ArduinoOTA.setPort(8277);

  // Hostname defaults to esp8266-[ChipID]
   ArduinoOTA.setHostname("Esp8266-COMP");

  // No authentication by default
   ArduinoOTA.setPassword("157953");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
  
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
  
// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(7200);
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
    OK_LED(3);
      Serial.println("connected"); 
      client.subscribe("HOME/COMP/PWR_ON"); // переподписываемся на топик 
     } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(5000);
    }
  } 
}

void PWR_ON(){
   digitalWrite(PWR_pin, LOW);
   delay(300);
   digitalWrite(PWR_pin, HIGH);
   client.publish("HOME/COMP/PWR_ON", "0");
   //Serial.println("POWERING ON... ");
   OK_LED(2);      
  }

void OK_LED(int count){   // Мигаем LED
   for (int i = 0; i == count; i++){
  digitalWrite(1, HIGH);
  delay(1500);
  digitalWrite(1, LOW);
  delay(500);
  };  
}

void callback(char* topic, byte* payload, unsigned int length) {
 
  //Serial.print("Message arrived in topic: ");
  //Serial.println(topic);
  message="";
  //Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    message=message+(char)payload[i];    
  }
  //Serial.print(message);

  if (message.toInt()==1){  PWR_ON();  }
 
  //Serial.println();
  //Serial.println("-----------------------");
 
}

String GetTime(){
    while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  return timeClient.getFormattedDate();
  }

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("RECONECT to WIFI");
    setup_wifi();
  }
   if (!client.connected()) {
    reconnect();
  }
  client.loop();   
   if (Timer1.isReady()) {  // Таймер на 1 секунду - опрос питания
    if (!digitalRead(PWR_state)==prev_state) {
    if (!digitalRead(PWR_state)) { client.publish("HOME/COMP/PWR_STATE", "1");} else { client.publish("HOME/COMP/PWR_STATE", "0");} ;
    prev_state=digitalRead(PWR_state); 
    time_delay=millis();};
    }
    if (Timer2.isReady()){
      
      client.publish("HOME/COMP/ONLINE", GetTime().c_str());
      }
}
