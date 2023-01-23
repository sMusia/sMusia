#include <ArduinoOTA.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"
#include "UniversalTelegramBot.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <GyverTimer.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

//servo
#define SERVO_1      14
#define SERVO_2      15
#define SERVO_STEP   5
Servo servo1;
Servo servo2;
int servo1Pos = 0;
int servo2Pos = 0;

// Timers
GTimer_ms Timer1;
GTimer_ms Timer2;
GTimer_ms Timer3;

#define timer_wifi 2*60*1000          // Таймер отработки проверки WIFI
#define auto_photo_timer 10*60*1000   // ТАймер для авто отправки фото
#define bot_timer 1000                // ТАймер для  проверки бота

// WIFI
const char *ssid[] = {"MyNet"};                      // WIFI ssid  
const char *password[] = {"romashka"};  
#define count_ssid 1 

bool sendPhoto = false;

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

String token = "1078551337:AAES4abYeFYkbIvC3xJ-6LZW2aU6qihMSqY";   // Create your bot and get the token -> https://telegram.me/fatherbot
String chat_id_def = "333923630";   // Get chat_id -> https://telegram.me/userinfobot
String chat_id;
bool auto_mode=false;

#define TELEGRAM_DEBUG

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(token, clientTCP);

camera_fb_t * fb;
uint8_t* fb_buffer;
size_t fb_length;
int currentByte;
String keyboardJson = "[[\"/get_photo\"], [\"/auto_off\",\"/auto_on\"], [\"/UP\"], [\"/LEFT\",\"/RIGHT\"], [\"/DOWN\"]]";

#define DEVICE "ENDER3PRO_CAMERA"
#define FlashLight_led 4


//CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void configInitCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;


  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63 чем меньше значение тем лучше качество
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  //0-63 чем меньше значение тем лучше качество
    config.fb_count = 2;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  // Drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_SXGA);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
}


void Flash(int count){
  digitalWrite(FlashLight_led,LOW);
  for (int i = 0; i < count; i++)
  {
    digitalWrite(FlashLight_led,HIGH);
    delay(250);
    digitalWrite(FlashLight_led,LOW);
    delay(250);
  }
}

void wifi_start(){
  Serial.println("Starting search of known WiFi.."); 
  WiFi.mode(WIFI_STA);
  int time_to_connect = 10000;
  unsigned long start_time_to_connect = millis(); 
  int numberOfNetworks = WiFi.scanNetworks();
  if (numberOfNetworks>0) {                         //Если найдены SSID
          for(int i =0; i<numberOfNetworks; i++){    //Перебираем все имена SSID
            for (int k = 0; k < (count_ssid); k++){   //Перебираем все имена SSID в массиве      
              if (WiFi.SSID(i)==ssid[k]){
                  Serial.print(WiFi.SSID(i));
                  Serial.print("->");
                  Serial.println((ssid[k])); 
                  WiFi.begin(ssid[k], password[k]);
                  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
                  while (millis()<(start_time_to_connect+time_to_connect) && (WiFi.status() != WL_CONNECTED)){ //пытаемся подключиться в течении времени                
                    delay(100);
                  }
               if (WiFi.status() == WL_CONNECTED) {
                 Serial.println("Connected!"); 
                 Serial.println(WiFi.localIP());  break;}  
              }
            }   
          }
  } else
  {
    Serial.println("nothing found..");
  }  
  if (WiFi.status() != WL_CONNECTED){Flash(2);} 
}

void setup() {
  Serial.begin(115200);
  ESP32PWM::timerCount[0]=4;
  ESP32PWM::timerCount[1]=4;
  servo1.setPeriodHertz(50); // standard 50 hz servo
  servo2.setPeriodHertz(50); // standard 50 hz servo
  servo1.attach(SERVO_1, 1000, 2000);
  servo2.attach(SERVO_2, 1000, 2000);

  pinMode(FlashLight_led,OUTPUT);
  wifi_start();
  //client.setInsecure();
  Timer1.setInterval(timer_wifi);
  Timer2.setInterval(auto_photo_timer);
  Timer3.setInterval(bot_timer);
  esp_log_level_set("*", ESP_LOG_VERBOSE);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  // Config and init the camera
  configInitCamera();
  
  // Port defaults to 8266
   ArduinoOTA.setPort(8232);

  // Hostname defaults to esp8266-[ChipID]
   ArduinoOTA.setHostname("Esp32-CAM");

  // No authentication by default
   ArduinoOTA.setPassword("157953");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
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
  Serial.println("Phase2");  
  bot.sendMessageWithReplyKeyboard(chat_id_def,"ENDER3PRO_CAMERA: READY!","", keyboardJson, true);
  Serial.println("Phase3");
  servo1.write(0);
  delay(200);
  servo2.write(0);
  delay(200);
  servo1.write(90);
  delay(200);
  servo2.write(90);
  delay(200);
  servo1Pos=90;
  servo2Pos=90;
  Serial.println("Phase4");
}

String sendPhotoTelegram() {
  digitalWrite(FlashLight_led,HIGH);
  const char* myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";
  
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }  
  Serial.println("Connect to " + String(myDomain));


  if (clientTCP.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    
    String head = "--AMS\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + chat_id_def + "\r\n--AMS\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--AMS--\r\n";

    uint16_t imageLen = fb->len;
    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = imageLen + extraLen;
  
    clientTCP.println("POST /bot"+token+"/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=AMS");
    clientTCP.println();
    clientTCP.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        clientTCP.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        clientTCP.write(fbBuf, remainder);
      }
    }  
    
    clientTCP.print(tail);
    
    esp_camera_fb_return(fb);
    
    int waitTime = 10000;   // timeout 10 seconds
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + waitTime) > millis()){
      Serial.print(".");
      delay(100);      
      while (clientTCP.available()) {
        char c = clientTCP.read();
        if (state==true) getBody += String(c);        
        if (c == '\n') {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } 
        else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length()>0) break;
    }
    digitalWrite(FlashLight_led,LOW);
    clientTCP.stop();
    Serial.println(getBody);
  }
  else {
    getBody="Connected to api.telegram.org failed.";
    Serial.println("Connected to api.telegram.org failed.");
  }
  return getBody;
}

void handleNewMessages(int numNewMessages) {
  //Serial.println("handleNewMessages");
  //Serial.println(String(numNewMessages));
//bot.sendMessageWithReplyKeyboard(chat_id, "Choose from one of the following options", "", keyboardJson, true);
  //String keyboardJson = "[[\"/get_photo\"], [\"/auto_off\",\"/auto_on\"]]";
  
  for (int i=0; i<numNewMessages; i++) {
    chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    Serial.println(text);
    if (text == "/get_photo") {
        bot.sendMessage(chat_id,"sending photo!");
        sendPhotoTelegram();
        sendPhoto = false;
      }
    if (text == "/auto_off") {
        auto_mode=false;
        bot.sendMessageWithReplyKeyboard(chat_id,"Auto get photo is off!","", keyboardJson, true);
    }
    if (text == "/auto_on") {
        auto_mode=true;
        bot.sendMessageWithReplyKeyboard(chat_id,"Auto get photo is ON!","", keyboardJson, true);
    }
    if (text == "/v") {
      if(servo2Pos >= 20) {
        servo2Pos -= 20;
        servo2.write(servo2Pos);
        Serial.print("SERVO2 pos: ");
        Serial.println(servo2Pos);
        Serial.print("SERVO1 pos: ");
        Serial.println(servo1Pos);
    } 
    }
    if (text == "/^") {
      if(servo2Pos <= 180) {
        servo2Pos += 20;
        servo2.write(servo2Pos);
        Serial.print("SERVO2 pos: ");
        Serial.println(servo2Pos);
        Serial.print("SERVO1 pos: ");
        Serial.println(servo1Pos);
    }
  }
  if (text == "/->") {
      if(servo1Pos >= 20) {
        servo1Pos -= 20;
        servo1.write(servo1Pos);
        Serial.print("SERVO2 pos: ");
        Serial.println(servo2Pos);
        Serial.print("SERVO1 pos: ");
        Serial.println(servo1Pos);
    } 
    }
    if (text == "/<-") {
      if(servo1Pos <= 180) {
        servo1Pos += 20;
        servo1.write(servo1Pos);
        Serial.print("SERVO2 pos: ");
        Serial.println(servo2Pos);
        Serial.print("SERVO1 pos: ");
        Serial.println(servo1Pos);
    }
}
    if (text == "/FLASH") {
      digitalWrite(FlashLight_led,!digitalRead(FlashLight_led));
    }
}
}

void loop(){
  ArduinoOTA.handle();
  
  if (Timer3.isReady()){
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
  }

  if (Timer1.isReady()){
    Serial.println("---------------Checking WIFI---------------");  
        if (WiFi.status() != WL_CONNECTED) {      // проверяем соединение WIFI
          Serial.println("RECONECT to WIFI");
          wifi_start();
        }
  }
  if (auto_mode==true) {
    if (Timer2.isReady()){
      Serial.println("---------------AUTO PHOTO---------------"); 
      digitalWrite(FlashLight_led,HIGH);
      sendPhotoTelegram(); 
      sendPhoto = false;}

  }
}