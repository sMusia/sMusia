#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <Arduino.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoOTA.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "ZCbtGdaUUHS5QFyde-09Z-5cY5mDMp72";


void PWR_ON(){
  digitalWrite(5,HIGH);
  delay(500);
  digitalWrite(5,LOW);
  delay(500);
  Serial.println("PWR_ON");
}

void FORCE_PWR_OFF(){
  digitalWrite(5,HIGH);
  delay(5000);
  digitalWrite(5,LOW);
  delay(500);
}


// This function will be called every time Slider Widget
// in Blynk app writes values to the Virtual Pin V1
BLYNK_WRITE(V1)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if (pinValue==1) {PWR_ON();}
  // process received value
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  pinMode(5,OUTPUT);
  pinMode(13, INPUT);
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);

  //reset settings - for testing
  //wifiManager.resetSettings();


  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("PC Switcher", "")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");


  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  Serial.println("Start BLYNK auth");
  Blynk.config(auth);

  bool result = Blynk.connect(180);

  if (result != true)
  {
    Serial.println("BLYNK Connection Fail");
    wifiManager.resetSettings();
    ESP.reset();
    delay (5000);
  }
  else
  {
    Serial.println("BLYNK Connected");
  }
    // No authentication by default
  Serial.println("Start BLYNK ok");
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
  Serial.println("Start OTA");
  ArduinoOTA.begin();
    Serial.println("DONE!");
}

void loop() {
  // put your main code here, to run repeatedly:
  ArduinoOTA.handle();                      // проверяем обновление для прошивки по воздуху 
  Blynk.run();
  delay(10);
  if (digitalRead(13)) {Blynk.virtualWrite(V0,255);} else {Blynk.virtualWrite(V0,0);}
}