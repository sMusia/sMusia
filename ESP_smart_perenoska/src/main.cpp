#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <GyverTimer.h>
#include <NTPClient.h>
#include "ESP8266HTTPClient.h"
#include <ESP8266HTTPUpdateServer.h>

ESP8266HTTPUpdateServer httpUpdater;
GTimer_ms Timer1;
GTimer_ms Timer2;
#define LED_PIN 12
#define RELAY_PIN 16



// change these values to match your network
const char* ssid = "IDEA-DESIGN";                      // WIFI ssid  
const char* password = "ochendlinniy";  
 
WiFiServer server(80);
String request = ""; 
ESP8266WebServer httpServer(8080);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

#define read_interval 1000  // интервал в милисек для опроса датчика тока
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
String dev_id="3492";
String serverName="http://iot.am-studio.com.ua/add.php";
String html_1 = R"=====(
<!DOCTYPE html>
<html>
 	<head>
   <title>SMART переноска</title>
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
		<h1>SMART переноска (ver0.1)<br></h1>
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

void LED_BLINK(int count,int delay_time){
  for (int i = 0; i < count; i++)
  {
    digitalWrite(LED_PIN,HIGH); delay(delay_time);
    digitalWrite(LED_PIN,LOW);  delay(delay_time);
  } 
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

void setup_wifi() {
  int period=0;
  Serial.println("Connecting to WiFi.."); 
  WiFi.mode(WIFI_STA);  
  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      LED_BLINK(1,50);
      period++;
        if (period>60){
        Serial.println("Не удалось подлючиться к WIFI - рестарт..."); 
        LED_BLINK(3,700); 
        ESP.restart(); period=0;}
      }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  timeClient.setTimeOffset(10800);
  startup_time=Get_Time();   
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
              LED_BLINK(5,100);
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
    
    Timer1.setInterval(read_interval);
    Timer2.setInterval(5000);      
    // Connect to a WiFi network
    setup_wifi();
    pinMode(RELAY_PIN,OUTPUT);
    pinMode(LED_PIN,OUTPUT);
    Serial.println("");
    Serial.println(F("[CONNECTED]"));
    Serial.print("[IP ");              
    Serial.print(WiFi.localIP()); 
    Serial.println("]");
 
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
    LED_BLINK(3,100);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");  //  "Завершение OTA-апдейта"
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100))); 
    LED_BLINK(1,10);   
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

void GET_WATT(){
  currWatt=0;
  unsigned long time=millis();
    while (time+200>millis())
    {
      int w=analogRead(A0);
      currWatt=max(w,currWatt);
    }  
  currWatt=map(currWatt,533,1024,0,6600);
  if (currWatt<0) currWatt=0;
  hourWatt=hourWatt + ((float)currWatt/ (3600.0/  ((float)read_interval/1000.0)  ) );  // 3600/(read_interval/1000)

}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {      // проверяем соединение WIFI
      Serial.println("RECONECT to WIFI");
      setup_wifi();
    }  
    httpServer.handleClient();
    ArduinoOTA.handle();                      // проверяем обновление для прошивки по воздуху 

    if (Timer2.isReady()){                    // Сбор и отправка статистики
        if (timeClient.getHours()!=last_update_time){      
            lasthourWatt=hourWatt;
            SendDataToDB();
            hourWatt=0;      
            last_update_time=timeClient.getHours();
          }
    }

    if (Timer1.isReady()){                    // Обработка реле
        GET_WATT();
        if (relayState==1) {digitalWrite(RELAY_PIN,HIGH);} else {digitalWrite(RELAY_PIN,LOW);}
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