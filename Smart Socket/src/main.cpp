#include <GyverButton.h>
#include <U8g2lib.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <GyverTimer.h>
#include <NTPClient.h>
#include "ESP8266HTTPClient.h"
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WiFi.h>
#include "ACS712.h"

// НАСТРОЙКИ
#define TIMER_RELAY   1000      // интервал в милисек для обработки реле
#define TIMER_STAT    5000      // Сбор и отправка статистики
#define TIMER_TEXT    2000      // Врем показа текстовых сообщений
#define TIMER_WATT    1000      // Интервал опроса датчика тока
#define TIMER_WIFI    120000    // Интервал переподключения WiFi


ACS712 sensor(ACS712_30A, A0);

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // pin remapping with ESP8266 HW I2C

ESP8266HTTPUpdateServer httpUpdater;

GTimer_ms Timer1;
GTimer_ms Timer2;
GTimer_ms Timer3;
GTimer_ms Timer4;
GTimer_ms Timer5;       // Таймер проверки WiFi
#define RELAY_PIN 13
#define BUT1_PIN 14
#define BUT2_PIN 12
#define SPEAKER 15
#define DSPL_T DisplayText(text)
#define DSPL_L Displayloging(loging)

GButton BUT1_BUT(BUT1_PIN);
GButton BUT2_BUT(BUT2_PIN);

// WIFI
const char *ssid[] = {"MusiaNetwork","1","1"};                      // WIFI ssid  
const char *password[] = {"zironkamorcheeba","1","1"};  
#define count_ssid 3 

WiFiServer server(80);
String request = ""; 
ESP8266WebServer httpServer(8080);

float min_analog;
float max_analog;
bool display_paused=false;
bool display_debug=false;
String text;
String loging;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
char logingmsg[100];

String messageBox[3] = {"","",""}; 
int currWatt = 0;
int allWatt = 0;
float hourWatt = 0;
int last_update_time=0;
int lasthourWatt=0;
int httpResponseCode,count_hour;
boolean relayState=false;
boolean isRequest=false;
String startup_time,resp_time;
String now_time;
String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
String dev_token="15P896k7";
String dev_id="3495";
String serverName="http://iot.am-studio.com.ua/add.php";
String html_1 = R"=====(
<!DOCTYPE html>
<html>
 	<head>
   <title>SMART розетка</title>
		 <meta name='viewport' content='width=device-width, initial-scale=1.0'/>
		 <meta charset='utf-8'>
     <script type="text/javascript" src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.9.3/Chart.min.js"></script>
     <script type="text/javascript" src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.9.3/Chart.bundle.min.js"></script>     
		 <style>
			  body {background-color:hsl(200,90%,15%);color: white;
			  	font-family: 'Jura', sans-serif;
			  	text-align: center; transition: .5s; margin: 0;}
			  .page {display:flex; flex-direction: column; padding: 0 30px; height: 100vh;}
			  .col{display: block;}
			  .col2{margin: auto;width: 100%;}
			  #col3,.col4{margin-top:30px;width: 100%;transition: 1s;}
			
			h1 {text-align:center; font-size: 90%;margin-bottom:20px ;text-transform: uppercase;margin-top: 0;}
			  #currWatt{font-size: 10rem; font-weight:100; padding: 20px 0 0 0;margin: 0;}
			  span{font-size: 3rem;}			 
			  
			  #relay {transition:1s; padding:10px;text-transform: uppercase; color:white; border-radius: 10px; font-size: 2rem;flex: center;}
			  .relay_wait{ background:#62a2af; border: 2px solid #fff; }
			  .relay_on{ background:#447716; border: 2px solid #fff; }
			  .relay_off{ background:#2f3435; border: 2px solid #fff; }
					.lds-ring {
					  display: none;
					}
					.relay_wait .lds-ring {
					  display: inline-block !important;
					  position: relative;
					  width: 40px;
					  height: 40px;
					}
					.lds-ring div {
					  box-sizing: border-box;
					  display: block;
					  position: absolute;
					  width: 32px;
					  height: 32px;
					  margin: 8px;
					  border: 4px solid #fff;
					  border-radius: 50%;
					  animation: lds-ring 1.2s cubic-bezier(0.5, 0, 0.5, 1) infinite;
					  border-color: #fff transparent transparent transparent;
					}
					.lds-ring div:nth-child(1) {
					  animation-delay: -0.45s;
					}
					.lds-ring div:nth-child(2) {
					  animation-delay: -0.3s;
					}
					.lds-ring div:nth-child(3) {
					  animation-delay: -0.15s;
					}
					@keyframes lds-ring {
					  0% {
					    transform: rotate(0deg);
					  }
					  100% {
					    transform: rotate(360deg);
					  }
					}
			  
			  .footer span{font-size: 1rem;font-weight:500;}
			  .footer{padding-bottom: 10px}
		 </style>
		 <link href="https://fonts.googleapis.com/css2?family=Jura:wght@300;400;500;600;700&display=swap" rel="stylesheet">
		 <script> 	

	var ajaxRequest = null;
		    if (window.XMLHttpRequest)  { ajaxRequest =new XMLHttpRequest(); }
		    else                        { ajaxRequest =new ActiveXObject("Microsoft.XMLHTTP"); }

  	function set_MODE(relay){
        if (relay==0) { 
        	document.getElementById("relay").className="relay_off";
        	document.getElementById("butt_rel").innerHTML="off";}
        if (relay==1) { 
        	document.getElementById("relay").className="relay_on";
        	document.getElementById("butt_rel").innerHTML="on";
          }		
	}
     
    function ajaxSEND(relay_state)
		    {
		      if(!ajaxRequest){ alert('AJAX is not supported.'); return; }		 
		      	ajaxRequest.open('GET',"/setData&relay="+relay_state ,true);
            ajaxRequest.send();
		    }
	function relay_change_state(){
		if (document.getElementById("relay").classList=="relay_off") {
			ajaxSEND(1);
			document.getElementById("relay").className="relay_wait";
			document.getElementById("butt_rel").innerHTML="";}
		if (document.getElementById("relay").classList=="relay_on") {ajaxSEND(0);
			document.getElementById("relay").className="relay_wait";
			document.getElementById("butt_rel").innerHTML="";}		
	}	    

	function ajaxGETDATA()
		{
		    if(!ajaxRequest)
		    	{ alert('AJAX is not supported.'); return;}
		      ajaxRequest.open('GET',"getData",true);
		      ajaxRequest.onreadystatechange = function()
		      {
		        if(ajaxRequest.readyState == 4 && ajaxRequest.status==200)
		        {
		          var ajaxResult = ajaxRequest.responseText;
		          var tmpArray = ajaxResult.split("|");
		          document.getElementById('currWatt').innerHTML = tmpArray[0];
              	  document.getElementById('allWatt').innerHTML = tmpArray[1];
              	  if (tmpArray[2]=="1") {set_MODE(1);} else {set_MODE(0);}
              var back_color=Math.round(100-(tmpArray[0]/40));              
              document.body.style='background-color: hsl('+back_color+',89%,15%);';
		        }
		      }
		      ajaxRequest.send();
		    }
	
	var myVar1 = setInterval(ajaxGETDATA, 1000);

		  </script> 
	</head>
<body>
<div class="page">
	<div class="col">
		<h1>SMART розетка<br></h1>
		<div>сейчас: <span id="currWatt">---</span><span>W</span></div>
    <div>за час: <span id="allWatt">----</span><span>W</span></div>
	</div>
	<div class="col2">
		<div class="relay_wait" id = "relay" onclick="relay_change_state()"><span id="butt_rel"></span><div class="lds-ring"><div></div><div></div><div></div><div></div></div></div>
	</div>
  <div class="col3">
		<a style="color:white;" href="https://iot.am-studio.com.ua/">Статистика</a>
	</div>
  <div class="footer">
	    <p class="info">startup time:
)====="; 
String html_2 = R"=====(  
      </p>
  </div>
</body>
)=====";

void ring(){
  tone(SPEAKER,800,100);delay(100);
}
void ring_relay(){
  tone(SPEAKER,1000);delay(100);
  noTone(SPEAKER);
  tone(SPEAKER,1500);delay(100);
  noTone(SPEAKER);
}
void ring_off(){
  tone(SPEAKER,300,300);delay(350);
  noTone(SPEAKER);
  tone(SPEAKER,300,300);delay(350);
  noTone(SPEAKER);
}
void ring_err(int counter){
  for (int i = 0; i < counter; i++)
  {
    tone(SPEAKER,500,300);
    delay(200);
    noTone(SPEAKER);
  }    
}

void ring_start(){
    tone(SPEAKER,500,100);
    delay(100);    
    tone(SPEAKER,1000,100);
    delay(100);
    tone(SPEAKER,1500,100);
    delay(100);   
    tone(SPEAKER,2000,100);
    delay(100);
    tone(SPEAKER,2500,100);
    delay(100);   
    tone(SPEAKER,3000,100);
    delay(100); 
}

void Display_pause(){
  display_paused=true;
  Timer4.reset();
}

void DisplayScreen(int screen){
  switch (screen)
  {
  case 1:
    //DisplayWATT();
    break;
  
  default:
    break;
  }  
}

void DisplayText(String &text){
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB18_tf);
  char charVar[ sizeof(text)];
  text.toCharArray(charVar, sizeof(text));
  int w=u8g2.getStrWidth(charVar);
  u8g2.setCursor(64-(w/2), 25);
  u8g2.print(text);
  u8g2.sendBuffer();
}

void Displayloging(String &loging){
  u8g2.clearBuffer();  
  messageBox[0]=messageBox[1];
  messageBox[1]=messageBox[2];
  messageBox[2]=loging;
  u8g2.setFont(u8g2_font_7x14B_tf);
  u8g2.setCursor(0, 10);
  u8g2.print(messageBox[0]);
  u8g2.setCursor(0, 21);
  u8g2.print(messageBox[1]);
  u8g2.setCursor(0, 32);
  u8g2.print(messageBox[2]);
  u8g2.sendBuffer();
}

void DisplayDEBUG(){
  u8g2.clearBuffer();    
  u8g2.setFont(u8g2_font_7x14B_tf);
  u8g2.setCursor(0, 10);
  u8g2.print("min: ");
  u8g2.setCursor(30, 10);
  u8g2.print(min_analog);
  u8g2.setCursor(0, 21);
  u8g2.print("max: ");
  u8g2.setCursor(30, 21);  
  u8g2.print(max_analog);
  //u8g2.print('W');
  u8g2.sendBuffer();
}

void DisplayWATT(int cur_watt){
  u8g2.clearBuffer();  
  u8g2.setFont(u8g2_font_logisoso20_tf);
  //u8g2.setCursor(0, 32);
  String disp=String(cur_watt)+'W';
  char charVar[ sizeof(disp)];
  disp.toCharArray(charVar, sizeof(disp));
  int w=u8g2.getStrWidth(charVar);
  u8g2.setCursor(64-(w/2), 32);
  u8g2.print(charVar);
  //u8g2.print('W');
  u8g2.sendBuffer();
}

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

void wifi_start(){
  Serial.println("Starting search of known WiFi.."); 
  loging="search known WiFi";
  DSPL_L;
  WiFi.mode(WIFI_STA);
  int time_to_connect = 10000;
  unsigned long start_time_to_connect = millis(); 
  int numberOfNetworks = WiFi.scanNetworks();
  loging=String(numberOfNetworks) + " networks found";
  DSPL_L;
  if (numberOfNetworks>0) {                         //Если найдены SSID
          for(int i =0; i<numberOfNetworks; i++){    //Перебираем все имена SSID
            for (int k = 0; k < (count_ssid); k++){   //Перебираем все имена SSID в массиве      
              if (WiFi.SSID(i)==ssid[k]){
                  Serial.print(WiFi.SSID(i));
                  Serial.print("->");
                  Serial.println((ssid[k])); 
                  WiFi.begin(ssid[k], password[k]);
                  while (millis()<(start_time_to_connect+time_to_connect) && (WiFi.status() != WL_CONNECTED)){ //пытаемся подключиться в течении времени                
                    delay(100);
                  }
               if (WiFi.status() == WL_CONNECTED) {
                 Serial.println("Connected!"); 
                 loging="Connected to" + WiFi.SSID(); DSPL_L; 
                 loging=WiFi.localIP().toString();    DSPL_L;
                 ring_start(); break;}  
              }
            }   
          }
  } else
  {
    Serial.println("nothing found.."); loging="no known WiFi found"; DSPL_L; ring_err(3);
  }  
  if (WiFi.status() != WL_CONNECTED){ loging="Work with no WiFi mode"; DSPL_L; ring_err(3);} 
}

void SendDataToDB(){
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
              httpRequestData+=lasthourWatt;
              resp_time=Get_Time();
              // Send HTTP POST request
              httpResponseCode = http.POST(httpRequestData);
              //DisplayText(5,100);
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
    u8g2.begin();
    Serial.begin(115200);
    Serial.println();
    Serial.println("Serial started at 115200");
    Serial.println();
    
    Timer1.setInterval(TIMER_RELAY); // Обработка реле
    Timer2.setInterval(TIMER_STAT);  // Сбор и отправка статистики
    Timer4.setInterval(TIMER_TEXT);  // Врем показа текстовых сообщений
    Timer3.setInterval(TIMER_WATT);   // Интервал опроса датчика тока
    Timer5.setInterval(TIMER_WIFI);   // Интервал переподключения WiFi

    pinMode(SPEAKER,OUTPUT);
    pinMode(RELAY_PIN,OUTPUT);
    pinMode(BUT1_PIN,INPUT_PULLUP);
    pinMode(BUT2_PIN,INPUT_PULLUP);

    digitalWrite(RELAY_PIN,LOW);
    sensor.setZeroPoint(555);
    wifi_start();
    // start a server
    httpUpdater.setup(&httpServer,"/update","admin","esp");
    httpServer.begin();
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
    text="Start OTA";
    Display_pause();
    DSPL_T;
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");  //  "Завершение OTA-апдейта"
    text="OTA DONE !";
    Display_pause();
    DSPL_T;
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100))); 

    //DisplayText('Start OTA');  
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
  delay(1000);
} 

void GET_WATT(){  
  currWatt=0;
  unsigned long time=millis();
  min_analog=1024;
  max_analog=0;
    while (time+250>millis())
    {
      int w=analogRead(A0);
      currWatt=max(w,currWatt);
      if (min_analog>w) {min_analog=w;}
      if (max_analog<w) {max_analog=w;}
    }  
  //((max_analog-min_analog)/2)*
  currWatt=map(currWatt,555,875,0,6600);  
  if (currWatt<0) currWatt=0;
  hourWatt=hourWatt + ((float)currWatt/ (3600.0/  ((float)TIMER_WATT/1000.0)  ) );  // 3600/(read_interval/1000)  
}


void loop() {
    BUT1_BUT.tick();  // обязательная функция отработки Кнопки1
    BUT2_BUT.tick();  // обязательная функция отработки Кнопки2

    if (Timer5.isReady()){  
        if (WiFi.status() != WL_CONNECTED) {      // проверяем соединение WIFI
          Serial.println("RECONECT to WIFI");
          text="RECONNECT WIFI";
          DSPL_T;
          Display_pause();
          wifi_start();
        }
    }
 
    httpServer.handleClient();
    ArduinoOTA.handle();                      // проверяем обновление для прошивки по воздуху 

    if ((display_paused==true) & (Timer4.isReady())){display_paused=false;}
    if (display_paused==false) 
      {
        if (Timer3.isReady()){
          GET_WATT(); 
          if (display_debug) {DisplayDEBUG();} else {DisplayWATT(currWatt);}
        }
      } 

    if (Timer2.isReady()){                    // Сбор и отправка статистики
        if (timeClient.getHours()!=last_update_time){      
            lasthourWatt=hourWatt;
            SendDataToDB();
            hourWatt=0;      
            last_update_time=timeClient.getHours();
          }
    }
    if  (BUT2_BUT.isHolded()) {
      relayState=!relayState;
      ring();      
    }
    if  (BUT1_BUT.isHolded()) {
      display_debug=!display_debug; 
      ring();     
    }
    if (relayState!=digitalRead(RELAY_PIN)) {
      ring_relay();
      if (relayState) {text="-ON-";} else {text="-OFF-";}
      Display_pause();
      digitalWrite(RELAY_PIN,relayState);
      DSPL_T;      
    }

    WiFiClient client = server.available();     // Check if a client has connected
    if (client)  
      {  
        request = client.readStringUntil('\r');     // Read the first line of the requestreturn;  
          isRequest=false;
          if ( request.indexOf("forceSend") > 0 ){ 
            isRequest=true;SendDataToDB(); 
            client.print( "okay" );          
          }
          if ( request.indexOf("getData") > 0 ){ 
                Serial.println("getData received");
                isRequest=true;

                if ( !isnan(currWatt) && !isnan(lasthourWatt))
                {
                    //client.print( header );
                    client.print( currWatt );   client.print( "|" ); 
                    client.print( lasthourWatt );   client.print( "|" ); 
                    client.print( relayState ); 
                }
                else
                {
                    //client.print( header );
                    client.print( "---" );    client.print( "|" );
                    client.print( "--" );     client.print( "|" );
                    client.print( "----" );  
                }
          }
          if ( request.indexOf("setData") > 0 ){
            isRequest=true;
            Serial.println("setData received");
            Serial.println(request);
            if (request.indexOf("relay=")>0){
                request=request.substring(request.lastIndexOf("relay=")+6,request.lastIndexOf("relay=")+8);
                relayState=(int)request.toInt();
                Serial.print("relayState is ");Serial.println(relayState);
            }
          }
          if (isRequest==false){ 
                client.flush();   
                client.print( header );
                client.print( html_1 ); 
                client.print( startup_time ); 
                client.print( "</br>" );
                client.print( " (resp: " );
                client.print(httpResponseCode);client.print( " | " );
                client.print(hourWatt);client.print( " | " );
                client.print(resp_time);client.print( " )" );
                client.print( html_2 ); 
          }            
      }   
}
