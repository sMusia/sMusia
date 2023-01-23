// БИБЛИОТЕКИ
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <GyverTimer.h>
#include "ESP8266HTTPClient.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

// ИНИЦИАЛИЗАЦИЯ
//Adafruit_BME280 bme; // I2C
GTimer_ms Timer1;
GTimer_ms Timer2;
Adafruit_BMP280 bmp;

// PINOUT
#define relay_pin 14 
#define speaker 16 

// WIFI
const char *ssid[] = {"ben","MusiaNetwork","RemontKomputerov0976988099"};                      // WIFI ssid  
const char *password[] = {"10121981","zironkamorcheeba","zaqwsxcderfv"};  
#define count_ssid 3 

const char* ssid2 = "IDEA-DESIGN";                      // WIFI ssid  
const char* password2 = "ochendlinniy"; 

WiFiServer server(80);
String request = ""; 
WiFiUDP ntpUDP;

// НАСТРОЙКИ
int last_update_minute=5;
#define read_interval 1000  // интервал в милисек для опроса датчика температуры
#define INIT_ADDR 2
#define INIT_KEY 46
// ПЕРЕМЕННЫЕ
float temper = 22;
int target_temp = 46;
int target_temp_eeprom;
int httpResponseCode;
//int max_temp;
boolean isRequest=false;
String startup_time;
String now_time;
String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
String dev_token="15P896k7";
String dev_id="3491";
String serverName="http://iot.am-studio.com.ua/add.php";
String html_1 = R"=====(
<!DOCTYPE html>
<html>
 	<head>
		 <meta name='viewport' content='width=device-width, initial-scale=1.0'/>
		 <meta charset='utf-8'>
		 <!-- UIkit CSS -->
			<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/uikit@3.6.5/dist/css/uikit.min.css" />

			<!-- UIkit JS -->
			<script src="https://cdn.jsdelivr.net/npm/uikit@3.6.5/dist/js/uikit.min.js"></script>
			<script src="https://cdn.jsdelivr.net/npm/uikit@3.6.5/dist/js/uikit-icons.min.js"></script>
			<link rel="preconnect" href="https://fonts.gstatic.com">
			<link href="https://fonts.googleapis.com/css2?family=Jura&display=swap" rel="stylesheet">
		 <style>
			  body {background:#0e4165;color: white;
			  	font-family: 'Jura', sans-serif;
			  	text-align: center; transition: .5s; margin: 0; }
			  .page {display:flex; flex-direction: column; padding: 0 30px; height: 100vh;}
			  .col{display: block;}
			  .col2{margin: auto;width: 100%;}
        #temp_save {    font-size: 200%;    text-align: center;    margin: 0 auto;    display: block;
}
			  #col3,.col4{margin-top:30px;width: 100%;transition: 1s;}

			  h1 {text-align:center; font-size: 110%;text-transform: uppercase;margin-top: 0;color:white;}
			  #temp{font-size: 5rem;padding: 20px 0 0 0;margin: 0;}
			  #hum{font-size: 10rem;padding: 0;margin: 0;line-height: 8rem;}
			  span{font-size: 3rem;}			 
			  
			  #mode { padding:10px; color:white; border-radius: 10px; width: 100%; font-size: 2rem;flex: center;}
			  .mode0{ background:#2CA77B; border: 2px solid #8CD6FF; }
			  .mode1{ background:#2C7BA7; border: 2px solid #8CD6FF; }
			  .mode2{ background:#869BA7; border: 2px solid #8CD6FF; }
			  
			  #hum_range{display: block;position: relative;}
			  .footer span{font-size: 1rem;}
			  .footer{padding-bottom: 10px}
			  .setting{    display: block;text-align: center;padding: 20px;border: 1px solid #ffffff38;background: #ffffff21; font-size: 150%; color: white; width: 200px; margin: 0 auto;
    border-radius: 10px;}
    .uk-modal-dialog.uk-modal-body {    color: black;}
			  	input[type=range] {  -webkit-appearance: none;  margin: 10px 0;  width: 100%;  background: transparent; /* Otherwise white in Chrome */}
				input[type=range]:focus {  outline: none;}
				input[type=range]::-webkit-slider-runnable-track {
				  width: 100%;
				  height: 10px;
				  cursor: pointer;
				  animate: 1s;
				  box-shadow: 0px 0px 0px #000000;
				  background: #C4C4C4;
				  border-radius: 5px !important;
				  border: 0px solid #000000;
				}
				input[type=range]::-webkit-slider-thumb {
				  box-shadow: 0px 0px 5px #636363;
				  border: 0px solid #6b6b6b;
				  height: 30px;
				  width: 30px;
				  border-radius: 18px;
				  background: #fff;
				  cursor: pointer;
				  -webkit-appearance: none;
				  margin-top: -10px;
				}
				input[type=range]:focus::-webkit-slider-runnable-track {
				  background: #C4C4C4;
				}
		 </style>
		 <script> 
	function save_to(){
		UIkit.modal("#setting-window").hide();
		ajaxSEND("save_temp="+document.getElementById('temp_save').value);
	}
	function updateData(){  
		ajaxLoad('getData'); 
	}
	var ajaxRequest = null;
		    if (window.XMLHttpRequest)  { ajaxRequest =new XMLHttpRequest(); }
		    else                        { ajaxRequest =new ActiveXObject("Microsoft.XMLHTTP"); }
		  
    function Target1change(){
        target = "target=" + document.getElementById('temp_range').value;        
        ajaxSEND(target);
      }
     
    function ajaxSEND(send_data)
		    {
		      if(!ajaxRequest){ alert('AJAX is not supported.'); return; }
		 
		      	ajaxRequest.open('GET','setData&'+send_data,true);
          		ajaxRequest.setRequestHeader("Content-type", "application/x-www-form-urlencoded");		      
		      	ajaxRequest.send(send_data);            
            console.log("send data: "+send_data);
		    }

		    function ajaxLoad(ajaxURL)
		    {
		      if(!ajaxRequest){ alert('AJAX is not supported.'); return; }
		 
		      ajaxRequest.open('GET',ajaxURL,true);
		      ajaxRequest.onreadystatechange = function()
		      {
		        if(ajaxRequest.readyState == 4 && ajaxRequest.status==200)
		        {
		          var ajaxResult = ajaxRequest.responseText;
		          var tmpArray = ajaxResult.split("|");
		          document.getElementById('temp').innerHTML = tmpArray[0];
		        if (tmpArray[1] == 1) {
              								document.getElementById('relay').innerHTML = "Сейчас нагрев включен";
              							};
              	if (tmpArray[1] == 0) {
              								document.getElementById('relay').innerHTML = "Сейчас нагрев выключен";
              							};
              document.getElementById('target_text').innerHTML = tmpArray[2]; 
              document.getElementById('temp_range').value = tmpArray[2];      
		        }
		      }
		      ajaxRequest.send();
		    }
		 
		    var myVar1 = setInterval(updateData, 1000); 
		  </script> 
	</head>
<body>
<div class="page">
	<div class="col">
		<h1>Умная йогуртница :)</h1>
    <div><span id="temp">---</span><span><sup><small>o</small></sup>C</span></div>
	</div>
	<div class="col3">
		<input type="range" min="35" onchange="Target1change()" max="50" value="40" id="temp_range" />
		<div class="footer"><span>Держать температуру </span> <span id="target_text"></span></div>
		<div><p id="relay"></p></div>
	</div>
</div>
	  <p class="info">ver 0.2 beta (28.12.2020) | 7447721@gmail.com | <a href="tel:+380977447721">+380977447721</p>
	  <a class="setting" data-uk-toggle="target: #setting-window">НАСТРОЙКИ</a>

<!-- This is the modal -->
<div id="setting-window" uk-modal>
    <div class="uk-modal-dialog uk-modal-body">
        <h2 class="uk-modal-title">НАСТРОЙКИ</h2>
        <p>Данные настройки сохранятся в памяти йогуртницы</p>
        )====="; 
String html_2 = R"=====(  
        <p class="uk-text-right">
            <button class="uk-button uk-button-default uk-modal-close" type="button">Отмена</button>
            <button class="uk-button uk-button-primary" type="button" onclick="save_to()">Сохранить</button>
        </p>
    </div>
</div>	  
</body>
)=====";

void ring_on(){
  tone(speaker,5000,300);delay(350);
}
void ring_off(){
  tone(speaker,300,300);delay(350);
  noTone(speaker);
  tone(speaker,300,300);delay(350);
  noTone(speaker);
}
void ring_err(int counter){
  for (int i = 0; i < counter; i++)
  {
    tone(speaker,500,300);
    delay(200);
    noTone(speaker);
  }    
}

void ring_start(){
    tone(speaker,500,100);
    delay(100);    
    tone(speaker,1000,100);
    delay(100);
    tone(speaker,1500,100);
    delay(100);   
    tone(speaker,2000,100);
    delay(100);
    tone(speaker,2500,100);
    delay(100);   
    tone(speaker,3000,100);
    delay(100); 
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
                  while (millis()<(start_time_to_connect+time_to_connect) && (WiFi.status() != WL_CONNECTED)){ //пытаемся подключиться в течении времени                
                    delay(500);
                  }
               if (WiFi.status() == WL_CONNECTED) {Serial.println("Connected!"); ring_start(); break;}  
              }
            }   
          }
  } else
  {
    Serial.println("nothing found.."); 
  }  
  if (WiFi.status() != WL_CONNECTED) {ring_err(5);} 
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("Serial started at 115200");
    Serial.println();
    pinMode(relay_pin,OUTPUT);
    digitalWrite(relay_pin,HIGH);
    delay(100);
    digitalWrite(relay_pin,LOW);

    Timer1.setInterval(read_interval);
    Timer2.setInterval(60*60*1000); // Таймер для проверки wifi
    EEPROM.begin(512);delay(10);
    EEPROM.get(INIT_ADDR, target_temp_eeprom);
    target_temp=target_temp_eeprom;
    if (! bmp.begin()) {
        ring_err(3);
    }
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL, // Режим работы
                Adafruit_BMP280::SAMPLING_X2, // Точность изм. температуры
                Adafruit_BMP280::SAMPLING_X16, // Точность изм. давления
                Adafruit_BMP280::FILTER_X16, // Уровень фильтрации
                Adafruit_BMP280::STANDBY_MS_500); // Период просыпания, мСек
    // Connect to a WiFi network
    wifi_start();
 
    Serial.print("[IP ");              
    Serial.print(WiFi.SSID()); 
    Serial.println("]");
 
    // start a server
    server.begin();
    Serial.println("Server started"); 

  // Port defaults to 8266
   ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  //ArduinoOTA.setHostname("Esp8266-COMP");

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
} 

void loop() {
    ArduinoOTA.handle();                      // проверяем обновление для прошивки по воздуху 
  
    if (Timer1.isReady()){  
        //bmp.takeForcedMeasurement(); // has no effect in normal mode
        temper = bmp.readTemperature();
        if ((temper>target_temp)&&(digitalRead(relay_pin))) {digitalWrite(relay_pin,LOW);ring_off();} 
        if ((temper<target_temp)&&(!digitalRead(relay_pin))){digitalWrite(relay_pin,HIGH);ring_on();}
    }

    if (Timer2.isReady()){  
        if (WiFi.status() != WL_CONNECTED) {      // проверяем соединение WIFI
          Serial.println("RECONECT to WIFI");
          wifi_start();
        }
    }

    WiFiClient client = server.available();     // Check if a client has connected
    if (client)  {  
 
    request = client.readStringUntil('\r');     // Read the first line of the request
 
    Serial.println(request);
    Serial.println("");
    isRequest=false;
    //if ( request.indexOf("forceSend") > 0 ){ isRequest=true;SendData();  }
    
    if ( request.indexOf("getData") > 0 ){ 
                Serial.println("getData received");
                isRequest=true;
    
                if ( !isnan(temper))
                {
                    //client.print( header );
                    client.print( temper );   client.print( "|" ); 
                    client.print( digitalRead(relay_pin) );  client.print( "|" ); 
                    client.print( target_temp );
                }
                else
                {
                    //client.print( header );
                    client.print( "---" );    client.print( "|" );
                    client.print( "--" );    
                }
          }
    if ( request.indexOf("setData") > 0 ){
            isRequest=true;
            Serial.println("setData received");
            //request = client.readStringUntil('\r');
            Serial.println(request);
            if (request.indexOf("target=")>0){
                request=request.substring(request.lastIndexOf("target=")+7,request.lastIndexOf("target=")+9);
                target_temp=(int)request.toInt();
                Serial.print("target is ");Serial.println(target_temp);
            } 
            if (request.indexOf("save_temp=")>0){
                request=request.substring(request.lastIndexOf("save_temp=")+10,request.lastIndexOf("save_temp=")+12);
                target_temp_eeprom=(int)request.toInt();
                EEPROM.put(INIT_ADDR, target_temp_eeprom);    
                EEPROM.commit();
                target_temp=target_temp_eeprom;           
                Serial.print("target is ");Serial.println(target_temp);
                ring_on();
            }
          }
if (isRequest==false){ 
    client.flush();   
    client.print( header );
    client.print( html_1 ); 
    Serial.println("New page served");
    if (WiFi.status() == WL_CONNECTED) {
      client.print( "<p>Сейчас подключены к сети "); 
      client.print(WiFi.SSID());
      client.print( "</p>");
    }
    client.print( "<input type='number' name='temp_save' id='temp_save' min='30' max='99' value='"); 
    client.print(target_temp_eeprom);
    client.print( "'>Поддерживать температуру<Br> "); 
    client.print( html_2 ); 
  }
  }
 

}