#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>

#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <GyverTimer.h>
#include <NTPClient.h>
#include "ESP8266HTTPClient.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme; // I2C

GTimer_ms Timer1;

// change these values to match your network
const char* ssid = "MusiaNetwork";                      // WIFI ssid  
const char* password = "zironkamorcheeba";  
 
WiFiServer server(80);
String request = ""; 

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"ua.pool.ntp.org");
//#define DataArrayCount 48 // сколько значений хранить (температура, влажн, время)
float temper = 0;
float humd = 0;
float atm = 0;
int last_update_minute=5;
int httpResponseCode;
//#define relay_pin 2 
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
     <script type="text/javascript" src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.9.3/Chart.min.js"></script>
     <script type="text/javascript" src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.9.3/Chart.bundle.min.js"></script>     
		 <style>
			  body {background:#0e4165;color: white;
			  	font-family: 'Jura', sans-serif;
			  	text-align: center; transition: .5s; margin: 0;}
			  .page {display:flex; flex-direction: column; padding: 0 30px; height: 100vh;}
			  .col{display: block;}
			  .col2{margin: auto;width: 100%;}
			  #col3,.col4{margin-top:30px;width: 100%;transition: 1s;}
			input.chart_butt[type="button"] {
				    transition: 1s;
				    padding: 10px;
				    text-transform: uppercase;
				    color: white;
				    border-radius: 10px;
				    width: 100%;
				    font-size: 1rem;
				    background: #09314e;
				}
			h1 {text-align:center; font-size: 90%;margin-bottom:20px ;text-transform: uppercase;margin-top: 0;}
			  #temp{font-size: 4rem; font-weight:100; padding: 20px 0 0 0;margin: 0;}
			  #hum{font-size: 12rem;font-weight:100; padding: 0;margin: 0;line-height: 8rem;}
			  span{font-size: 3rem;}			 
			  
			  #mode {transition:1s; padding:10px;text-transform: uppercase; color:white; border-radius: 10px; width: 100%; font-size: 2rem;flex: center;}
			  .mode0{ background:#2CA77B; border: 2px solid #fff; }
			  .mode1{ background:#2C7BA7; border: 2px solid #fff; }
			  .mode2{ background:#869BA7; border: 2px solid #fff; }
			  
			  #hum_range{display: block;position: relative;}
			  .footer span{font-size: 1rem;font-weight:500;}
			  .footer{padding-bottom: 10px}
			  	.hidden{opacity:0;transform: translateY(100px); transition: 1s}
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
		 <link href="https://fonts.googleapis.com/css2?family=Jura:wght@300;400;500;600;700&display=swap" rel="stylesheet">
		 <script> 
	var mode=0;

	function updateData(){  
		ajaxLoad('getData'); 
	}
	var ajaxRequest = null;
		    if (window.XMLHttpRequest)  { ajaxRequest =new XMLHttpRequest(); }
		    else                        { ajaxRequest =new ActiveXObject("Microsoft.XMLHTTP"); }
		  
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
              if (tmpArray[1]>0) 
                        {document.getElementById('hum').innerHTML = Math.round(tmpArray[1]);}
                  else  {document.getElementById('hum').innerHTML = tmpArray[1];}	              
                document.getElementById('atm').innerHTML = tmpArray[2];
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
		<h1>Домашняя погодная станция</br></h1>
		<div><span id="temp">---</span><span><sup><small>o</small></sup>C</span></div>
		<div><span id="hum">--</span><span>%</span></div>
    <div><span id="atm">----</span><span>mmHg</span></div>
	</div>
</div>	
	  <p class="info">startup time:
)====="; 
String html_2 = R"=====(  
   </p>
</body>
)=====";




String Get_Date(){
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  return String(timeClient.getEpochTime());
}

String Get_Time(){
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  return timeClient.getFormattedTime().c_str();  
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

  Serial.println("WiFi connected ---");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  timeClient.setTimeOffset(10800);
  startup_time=Get_Time();   
}
void SendData(){
      while(!timeClient.update()) {
        timeClient.forceUpdate();
      }
          HTTPClient http;
          
          // Your Domain name with URL path or IP address with path
          http.begin(serverName);

          // Specify content-type header
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");
          // Data to send with HTTP POST
          String httpRequestData = "k=";
          httpRequestData+=dev_token;
          httpRequestData+="&id=";
          httpRequestData+=dev_id;
          httpRequestData+="&p1=";
          httpRequestData+=Get_Date();
          httpRequestData+="&p2=";
          httpRequestData+=temper;
          httpRequestData+="&p3=";
          httpRequestData+=humd;
          httpRequestData+="&p4=";
          httpRequestData+=atm;
          
          // Send HTTP POST request
          httpResponseCode = http.POST(httpRequestData);
          
          // If you need an HTTP request with a content type: application/json, use the following:
          //http.addHeader("Content-Type", "application/json");
          //int httpResponseCode = http.POST("{\"api_key\":\"tPmAT5Ab3j7F9\",\"sensor\":\"BME280\",\"value1\":\"24.25\",\"value2\":\"49.54\",\"value3\":\"1005.14\"}");

          // If you need an HTTP request with a content type: text/plain
          //http.addHeader("Content-Type", "text/plain");
          //int httpResponseCode = http.POST("Hello, World!");
        
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);        
          // Free resources
          http.end();

}


void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("Serial started at 115200");
    Serial.println();
    //pinMode(relay_pin,OUTPUT);
    //digitalWrite(relay_pin,HIGH);
    Timer1.setInterval(10000);
    bme.begin(0x76, &Wire);
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                        Adafruit_BME280::SAMPLING_X1, // temperature
                        Adafruit_BME280::SAMPLING_X1, // pressure
                        Adafruit_BME280::SAMPLING_X1, // humidity
                        Adafruit_BME280::FILTER_OFF   );
    
    // Connect to a WiFi network
    setup_wifi();
 
    Serial.println("");
    Serial.println(F("[CONNECTED]"));
    Serial.print("[IP ");              
    Serial.print(WiFi.localIP()); 
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
  
    if (WiFi.status() != WL_CONNECTED) {      // проверяем соединение WIFI
      Serial.println("RECONECT to WIFI");
      setup_wifi();
    }

    WiFiClient client = server.available();     // Check if a client has connected
    if (!client)  {  return;  }
 
    request = client.readStringUntil('\r');     // Read the first line of the request
 
    Serial.println(request);
    Serial.println("");
    isRequest=false;
    if ( request.indexOf("forceSend") > 0 ){ 
      isRequest=true;SendData();
    }
    if ( request.indexOf("getData") > 0 ){ 
                Serial.println("getData received");
                isRequest=true;
    
                if ( !isnan(humd) && !isnan(temper))
                {
                    //client.print( header );
                    client.print( temper );   client.print( "|" ); 
                    client.print( humd );   client.print( "|" ); 
                    client.print( atm ); 
                }
                else
                {
                    //client.print( header );
                    client.print( "---" );    client.print( "|" );
                    client.print( "--" );     client.print( "|" );
                    client.print( "----" );  
                }
          }
if (isRequest==false){ 
    client.flush();   
    client.print( header );
    client.print( html_1 ); 
    client.print( startup_time ); 
    client.print( "</br>" );
    client.print( now_time );client.print( " (resp: " );
    client.print(httpResponseCode);client.print( " | " );
    client.print(last_update_minute);client.print( " )" );
    client.print( html_2 ); 

    Serial.println("New page served");
  }

if (Timer1.isReady()){  
  if ((timeClient.getMinutes()==30) or (timeClient.getMinutes()==0)) {
    if (timeClient.getMinutes()!=last_update_minute){
      SendData();
      last_update_minute=timeClient.getMinutes();
    }
  }  
  now_time=Get_Time();
  bme.takeForcedMeasurement(); // has no effect in normal mode
  temper = bme.readTemperature();
  atm = bme.readPressure() / 133.322F;
  humd = bme.readHumidity();

  }
} 