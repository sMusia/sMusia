#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
//#include <ESP8266HTTPUpdateServer.h>
//#include <WiFiManager.h>
#include <ArduinoOTA.h>


// change these values to match your network
const char* ssid = "MyNet";                      // WIFI ssid  
const char* password = "romashka";   

ESP8266WebServer httpServer(80);
//ESP8266HTTPUpdateServer httpUpdater;
WiFiClient espClient;

#include <FastLED.h>
// настройки ленты
#define NUM_LEDS 190    // количество светодиодов
#define R1_BUT_PIN 12      // пин кнопки правой
#define R2_BUT_PIN 13      // пин кнопки левой
#define L1_BUT_PIN 4      // пин кнопки правой
#define L2_BUT_PIN 14      // пин кнопки левой
#define DATA_PIN 5 // Пин ленты

// Use qsuba for smooth pixel colouring and qsubd for non-smooth pixel colouring
#define qsubd(x, b)  ((x>b)?b:0)                              // Digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#define qsuba(x, b)  ((x>b)?x-b:0)                            // Analog Unsigned subtraction macro. if result <0, then => 0

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];
// настройки Кнопок
#include "GyverButton.h"
GButton R1_BUT(R1_BUT_PIN);
GButton R2_BUT(R2_BUT_PIN);
GButton L1_BUT(L1_BUT_PIN);
GButton L2_BUT(L2_BUT_PIN);

// ПЕРЕМЕННЫЕ И НАСТРОЙКИ ЕФФЕКТОВ
int rainb_time_stamp;
int rainb_delay=1000;
int rainb;
int cooling_fire2;
int prew_cooling_fire2;
int thismode = 0;
int auto_mode;
// Initialize changeable global variables.
uint8_t max_bright = 128;                                      // Overall brightness definition. It can be changed on the fly.

unsigned long previousMillis;                                 // Store last time the strip was updated.

// Define variables used by the sequences.
uint8_t  thisfade = 8;                                        // How quickly does it fade? Lower = slower fade rate.
int       thishue = 50;                                       // Starting hue.
uint8_t   thisinc = 1;                                        // Incremental value for rotating hues
uint8_t   thissat = 100;                                      // The saturation, where 255 = brilliant colours.
uint8_t   thisbri = 255;                                      // Brightness of a sequence. Remember, max_bright is the overall limiter.
int       huediff = 256;                                      // Range of random #'s to use for hue
uint8_t thisdelay = 5;                                        // We don't need much delay (if any)

// переменные для салюта
int s_start_pos[5];
int s_time[5]={random(30),random(30),random(30),random(30),random(30)};
int s_color[5];
int s_brightness[5];
int s_tick[5]={0,0,0,0,0};
int s_scale[5];


int counter = 0;
int led_mode = 0;
int new_star;
int count_ef3;
int eff4;
int eff4_color;
int eff4_i;
int eff5;
int eff5_color;
int eff6;
String cur_mode;
bool quene;
bool left_l;
bool right_l;
int set_bright;
int cur_bright;
uint8_t colorIndex;
// ленивая жопа
#define FOR_i(from, to) for(int i = (from); i < (to); i++)
#define FOR_j(from, to) for(int j = (from); j < (to); j++)
#define FOR_i_down(from, to) for(int i = (from); i > (to); i--)
#define FOR_j_down(from, to) for(int j = (from); j > (to); j--)

// настройки пламени
int HUE_GAP=30;    // заброс по hue
#define FIRE_STEP 20    // шаг огня
int HUE_START=0;         // начальный цвет огня (0 красный, 80 зелёный, 140 молния, 190 розовый)
#define MIN_BRIGHT 00       // мин. яркость огня
#define MAX_BRIGHT 255  // макс. яркость огня
#define MIN_SAT 245     // мин. насыщенность
#define MAX_SAT 255     // макс. насыщенность

#define COLOR_DEBTH 3   // цветовая глубина: 1, 2, 3 (в байтах)
// на меньшем цветовом разрешении скетч будет занимать в разы меньше места,
// но уменьшится и количество оттенков и уровней яркости!

CRGBPalette16 currentPalette = OceanColors_p;                                 // Palette definitions
CRGBPalette16 targetPalette;
TBlendType currentBlending = LINEARBLEND;

CRGBPalette16 pacifica_palette_1 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
CRGBPalette16 pacifica_palette_2 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
CRGBPalette16 pacifica_palette_3 = 
    { 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33, 
      0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };

//#include <colorMusic_v2.10.h>

// FFT - spectrum analyzer
#include "arduinoFFT.h"
#define SAMPLES     32            //Must be a power of 2
double vReal[SAMPLES];
double vImag[SAMPLES];
double vResult[NUM_LEDS/2];
//#define  xres 64
//char data_avgs[xres];
arduinoFFT FFT = arduinoFFT();   
int result_vu;

void reorg_to_led(){
    //Reorganize number of spectrum to number leds    
    for (int i = 0; i < NUM_LEDS/2; i++)
    {
      int iteration = map(i,0,NUM_LEDS/2,3,SAMPLES-5);
      vResult[i] = vReal[iteration];
    }
}

void sampling(){
// ++ Sampling
   for(int i=0; i<SAMPLES; i++)
    {
      //delay(10);
      int value = analogRead(A0) - 512 ;                 // Read from ADC and subtract DC offset caused value
      vReal[i]= value/4;                      // Copy to bins after compressing
      vImag[i] = 0;                         
    }
    // -- Sampling
}

void FFT_comp(){
    // ++ FFT
    FFT.DCRemoval(vReal,SAMPLES);
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_RECTANGLE, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    // -- FFT
}

void water_fall_vu(){
  //#define chanel_water 5;
  EVERY_N_MILLISECONDS(50){
    sampling();
    FFT_comp();       
  }
  int complex_wave = vReal[SAMPLES/8]+vReal[SAMPLES/2]+vReal[SAMPLES-(SAMPLES/8)];
  //int complex_wave =  vReal[5];
  complex_wave = constrain(map(complex_wave,0,1000,0,255),0,255);
  leds[(NUM_LEDS/2)-1]=CHSV(complex_wave,255,complex_wave);
  leds[(NUM_LEDS/2)+1]=CHSV(complex_wave,255,complex_wave);
  FOR_i(0,NUM_LEDS/2){
    leds[i]=leds[i+1];
    leds[NUM_LEDS-i]=leds[(NUM_LEDS-i)-1];
  }
  FastLED.show();
}

void VU_meter(){
  #define chanel 27
  EVERY_N_MILLISECONDS(25){
    sampling();
    FFT_comp();       
  }
  int input_vu = constrain(map(vReal[chanel],0,512,0,NUM_LEDS/2),0,255);
  if (result_vu<input_vu) {
    result_vu= result_vu + ((input_vu-result_vu)*.3);//((map(vReal[chanel],0,512,0,NUM_LEDS/2)-result_vu)/2);
  } else {
    result_vu= result_vu - ((result_vu-input_vu)*.3);//((map(vReal[chanel],0,512,0,NUM_LEDS/2)-result_vu)/2);
  }
  FOR_i(0,NUM_LEDS/2){ 
    if (result_vu>i) {
      leds[(NUM_LEDS/2)-i]=CHSV(beatsin8(3,0,255,0,i),255,255);
      leds[(NUM_LEDS/2)+i]=CHSV(beatsin8(3,0,255,0,i),255,255);
      } else {
      leds[(NUM_LEDS/2)-i]=CHSV(beatsin8(3,0,255,0,i),255,0);
      leds[(NUM_LEDS/2)+i]=CHSV(beatsin8(3,0,255,0,i),255,0);
      }
  }
  FastLED.show();
}

void audioLoop(){
  EVERY_N_MILLISECONDS(25){
    sampling();
    FFT_comp();    
    reorg_to_led();   
  }

    // -- re-arrange FFT result to match with no. of columns on display ( xres )
    fadeToBlackBy(leds, NUM_LEDS, 2);
    FOR_i(0,NUM_LEDS/2){      
      //data_avgs[i]=data_avgs[i]/2;
      int result = vResult[i];
      //result = constrain(vImag[i],0,255);
      result=constrain(leds[(NUM_LEDS/2)-i].getLuma()+result,0,255);
      //leds[(NUM_LEDS/2)+i]=constrain(leds[(NUM_LEDS/2)-i].getLuma()+result,0,255);
      //data_avgs[i] = data_avgs[i]+result;
      leds[(NUM_LEDS/2)-i]=CHSV(beatsin8(3,0,255,0,i),255,result);
      leds[(NUM_LEDS/2)+i]=CHSV(beatsin8(3,0,255,0,i),255,result);
    }
    FastLED.show();
}

void blink(int count, CRGB colour){
  Serial.println("BLINK");
  for (int k = 0; k < count; k++)
  {    
    leds[0]=colour;
    FastLED.show();
    delay(50);  
    leds[0]=CRGB::Black;
    FastLED.show();
    delay(50);
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
      if (leds[0]==CRGB(0,0,0)) {
          leds[0]=CRGB::Blue;
        } else {
          leds[0]=CRGB::Black;
        }
      LEDS.show();
      period++;
        if (period>1000){
        Serial.println("Не удалось подлючиться к WIFI - рестарт...");  
        blink(10,CRGB::Red);
        ESP.restart(); period=0;}
      }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  httpServer.begin();
  blink(5,CRGB::Green);
}
 // функция выдачи главного html  
void handleRoot() {
  String message = R"=====(
<!DOCTYPE html>
<html>
<head>
	<META HTTP-EQUIV="Content-Type" CONTENT="text/html; charset=utf-8" />
	<meta name="viewport" content="width=device-width, initial-scale=1">
	<TITLE>BED Light</TITLE>
	<style>
		body {margin:0;padding:0;font: 12px Tahoma; background: #393939; color:#B1B1B1;}
		.header{
		background: #595959;
		display: grid;
		grid-auto-columns: auto;
		grid-template-columns: 4fr 2fr 1fr;
		padding: 5px;
		}
		.header div:not(:first-child){
			border-left:1px solid #787878;
			padding-left: 8px;
		}
    input#color {
    width: 100%;
    margin: 0 auto;
    padding: 0;
    border: 0;
    background-color: #5b5a5d;
    }
    .colorPicker {
    text-align: center;
    border: 1px solid #7d7d7d;
    margin: 10px;
    padding: 10px;
    border-radius: 10px;
    }
		a.button {
			padding: 20px;
			background: #464646;
			border: 1px solid #929292;
			border-radius: 8px;
			margin-left: 10px;
			text-align: center;
			font-size: 32px;
			color: #B1B1B1;
			text-decoration: none;
		}

		a.button:last-child,a.button_mini:last-child {
			margin-right: 10px;
		}
		.but_row1 {
			display: grid;
            grid-template-columns: 1fr 2fr 1fr;
            margin-top: 50px;}
		.but_row2 {
    margin: 10px 0 10px;
    display: grid;
}
		.but_row3 {
    display: grid;
    padding-left: 10px;
    grid-template-columns: 4fr 1fr;
}
		.but_row4 {}
		.button_mini{
    padding: 10px;
    background: #464646;
    border: 1px solid #929292;
    border-radius: 8px;
    margin-left: 10px;
    text-align: center;
    font-size: 14px;
    color: #B1B1B1;
    text-decoration: none;
    display: block;
    white-space: nowrap;
}.but_row {
    display: grid;
    grid-template-columns: 1fr 1fr 1fr;
    margin: 10px 0;
}hr{	
    margin: 20px 10px;
    border-color: #787878;
} 
	</style>
	<script type="text/javascript">
		function openUrl(url) {
		  var request = new XMLHttpRequest();
		  request.open('GET', url, true);
		  request.send(null);
		}
		function refreshData() {
			var request = new XMLHttpRequest();
			request.open('GET', '/data', true);
			request.onreadystatechange = function() {
				if (request.readyState == 4) {
				var data = JSON.parse(request.responseText);
				document.getElementById('cur_mode').innerHTML = data.cur_mode;
        document.getElementById('bright').value = data.cur_bright;
				}
      		}
      		request.send(null);
    	}
    	setInterval(refreshData, 1500);
	</script>
</head>
<body>
<div class="header"><div>BED Light - test version 2</div><div id="cur_mode">mode: 6</div><div>online</div></div>
<div class="but_row1">
    <a href="#" class="button"  onclick="openUrl('/switch?mode=L')">L</a><a href="#" class="button"  onclick="openUrl('/switch?mode=OFF')">OFF</a><a href="#" class="button" onclick="openUrl('/switch?mode=R')">R</a>
</div>
<div class="but_row2">
    <a href="#" class="button"  onclick="openUrl('/switch?mode=MAX')">MAX</a>
</div>
<span style="padding-left:10px;">BRIGHTNESS</span>
<div class="but_row3">
      <input type="range" id="bright" name="bright"  min="0" max="255"  onchange="openUrl('/bright?bright='+this.value)">
      <a href="#" class="button_mini" onclick="openUrl('/switch?mode=night')">night mode</a>
</div>
<hr>
<span style="padding-left:10px;">EFFECTS</span>
<div class="but_row">
    <a href="#" class="button_mini" onclick="openUrl('/switch?mode=fire')">fire</a>
    <a href="#" class="button_mini" onclick="openUrl('/switch?mode=rainbow')">rainbow</a>
    <a href="#" class="button_mini" onclick="openUrl('/switch?mode=stars')">stars</a>
</div>
<div class="but_row">
    <a href="#" class="button_mini" onclick="openUrl('/switch?mode=ocean')">ocean</a>
    <a href="#" class="button_mini" onclick="openUrl('/switch?mode=confetti')">confetti</a>
    <a href="#" class="button_mini" onclick="openUrl('/switch?mode=lava')">lava</a>
</div>
<div class="but_row">
    <a href="#" class="button_mini" onclick="openUrl('/switch?mode=fire2')">fire2</a>
    <a href="#" class="button_mini" onclick="openUrl('/switch?mode=meteor')">meteor</a>
    <a href="#" class="button_mini" onclick="openUrl('/switch?mode=sparkle')">sparkle</a>
</div>
<div class="but_row">
    <a href="#" class="button_mini" onclick="openUrl('/switch?mode=christmas1')">christmas1</a>
    <a href="#" class="button_mini" onclick="openUrl('/switch?mode=Lazy')">Lazy</a>
    <a href="#" class="button_mini" onclick="openUrl('/switch?mode=salut')">salut</a>
</div>
<div class="but_row">
    <a href="#" class="button_mini" onclick="openUrl('/switch?mode=MusicMode1')">MusicMode1</a>
    <a href="#" class="button_mini" onclick="openUrl('/switch?mode=MusicMode2')">MusicMode2</a>
    <a href="#" class="button_mini" onclick="openUrl('/switch?mode=MusicMode3')">MusicMode3</a>
</div>
<div class="colorPicker">CUSTOM COLOR
 	<input type="color" id="color" name="color" onchange="openUrl('/color?col='+this.value.replace('#',''))">
</div>
</body>
</html>
)=====";
 httpServer.send(200, F("text/html"), message);
}
//---------------Базовые функции---------------------
void setBrightness(int brightness){
  if (brightness!=FastLED.getBrightness()){
    if (brightness>FastLED.getBrightness()){
      FOR_i(FastLED.getBrightness(),brightness){
        FastLED.setBrightness(i);
        FastLED.delay(1);
        FastLED.show();
      }} else {
      FOR_i_down (FastLED.getBrightness(),brightness){ 
        FastLED.setBrightness(i);
        FastLED.delay(1);
        FastLED.show(); }
      }
  }
}
void LED_Black(){
  FOR_i(0,NUM_LEDS){
    leds[i]=CRGB::Black;
  }
};
void fadeall() { 
  fadeToBlackBy(leds, NUM_LEDS, 64);
  //FastLED.show();
}
void fill(CRGB color) { 
  FOR_i(0,NUM_LEDS/2){
      leds[(NUM_LEDS/2)-i]=color;
      leds[(NUM_LEDS/2)+i]=color; 
      FastLED.delay(20);
      FastLED.show();
  } 
}
void fadeoff() {
  FOR_i(0,50)
  {
    fadeToBlackBy(leds, NUM_LEDS, 20);
    FastLED.delay(2);
    FastLED.show();
  }
  LED_Black();
  FastLED.show();
}
void MAX_bright() {
  fadeoff();
  //FastLED.delay(20);
  setBrightness(255);
  FOR_i(0,NUM_LEDS/2){
      leds[(NUM_LEDS/2)-i]=CRGB::White;
      leds[(NUM_LEDS/2)+i]=CRGB::White; 
      FastLED.delay(20);
      FastLED.show();
  } 
}
void LEFT_light(){
  if ((!left_l)and(!right_l)){fadeoff();} 
  if (left_l) {
    left_l=false; 
      //FOR_j_down(256,0){
        FOR_i(120,190){
          leds[i]=0;
          FastLED.show();
          FastLED.delay(10);
        }     
      //}
    }
  else{
    setBrightness(255);
    left_l=true;
    FOR_j(0,255){
      FOR_i(120,190){
        leds[i]=CHSV(0,0,j);
      }
    FastLED.show();
    //FastLED.delay(1);
    }
  }
  }
void RIGHT_light(){
  if ((!left_l)and(!right_l)){fadeoff();}
  
  if (right_l) {
    right_l=false;    
    //FOR_j_down(256,0){
      FOR_i_down(71,0){
        leds[i-1]=0;
        FastLED.delay(10);
        FastLED.show();
      }
    }
    //}
  else{
    setBrightness(255);
    right_l=true;
    FOR_j(0,255){
      FOR_i(0,70){
        leds[i]=CHSV(0,0,j);
      }
    FastLED.show();
    //FastLED.delay(1);
    }
  }
  }

// функция обработки Data
void handleData() {
  String message = F("{\"");
  message += F("cur_mode");
  message += F("\":\"");
  message += String(cur_mode);
  message += F("\",\"");
  message += F("cur_bright");
  message += F("\":\"");
  message += String(FastLED.getBrightness());
  message += F("\"}");

  httpServer.send(200, F("text/html"), message);
}
// функция обработки MODE
void handleSwitch() {
  String mode = httpServer.arg("mode");
  if (mode) {cur_mode=mode;}
  String message = "OK";
  quene=true;
  httpServer.send(200, F("text/html"), message);
}
// функция обработки BRIGHT
void handleBright() {
  String bright = httpServer.arg("bright");
  setBrightness((int)bright.toInt());
  String message = "OK";
  httpServer.send(200, F("text/html"), message);
}
// функция обработки COLOR
void handleColor() {
  String color = httpServer.arg("col");
  uint32_t data = (uint32_t) strtol((const char *) &color[0], NULL, 16);
  
    long r = ((data >> 16) & 0xFF);
    long g = ((data >> 8) & 0xFF);
    long b = ((data >> 0) & 0xFF);
    fill(CRGB(r,g,b));

  cur_mode = "Color "+color;
  String message = "OK";
  httpServer.send(200, F("text/html"), message);
}
String quoteEscape(const String& str) {
  String result = "";
  int start = 0, pos;

  while (start < str.length()) {
    pos = str.indexOf('"', start);
    if (pos != -1) {
      result += str.substring(start, pos) + F("&quot;");
      start = pos + 1;
    } else {
      result += str.substring(start);
      break;
    }
  }
  return result;
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  //sbi(ADCSRA, ADPS2);
  //cbi(ADCSRA, ADPS1);
  //sbi(ADCSRA, ADPS0);
  LEDS.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  //FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  
  //FastLED.setBrightness(0);

  LEDS.setBrightness(255);  
  // Initialize a server
  //httpUpdater.setup(&httpServer,"/update","admin","157953");
  //httpServer.begin();
  
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

  httpServer.onNotFound([]() {
    httpServer.send(404, F("text/plain"), F("FileNotFound"));
  });
  httpServer.on("/", handleRoot);
  httpServer.on("/index.html", handleRoot);
  httpServer.on("/switch", handleSwitch);
  httpServer.on("/data", handleData);
  httpServer.on("/bright", handleBright);
  httpServer.on("/color", handleColor);

  }


//-------EFFECTs-------------------------------------------
// возвращает цвет огня для одного пикселя
CHSV getFireColor(int val) {
  // чем больше val, тем сильнее сдвигается цвет, падает насыщеность и растёт яркость
  return CHSV(
           HUE_START + map(val, 0, 255, 0, HUE_GAP),                    // H
           constrain(map(val, 0, 255, MAX_SAT, MIN_SAT), 0, 255),       // S
           constrain(map(val, 0, 255, MIN_BRIGHT, MAX_BRIGHT), 0, 255)  // V
         );
}
void fireTick() {
  static uint32_t prevTime;
  HUE_START=0;
  HUE_GAP=30;    // заброс по hue
  // двигаем пламя
  if (millis() - prevTime > 40) {
    prevTime = millis();
    FOR_i(0, NUM_LEDS) {
      leds[i] = getFireColor((inoise8(i * FIRE_STEP, counter)));
    }
    counter += 20;
    FastLED.show();
  }
}
void lavaTick() {
  static uint32_t prevTime;
  HUE_START=130;
  HUE_GAP=200;
  // двигаем лаву
  if (millis() - prevTime > 10) {
    prevTime = millis();
    FOR_i(0, NUM_LEDS) {
      leds[i] = getFireColor((inoise8(i * 5, counter)));
    }
    counter += 1;
    FastLED.show();
  }
}
void LazyTick() {
  static uint32_t prevTime;
  HUE_START=130;
  HUE_GAP=200;
  // двигаем Lazy
  if (millis() - prevTime > 4) {
    prevTime = millis();
    FOR_i(0, NUM_LEDS) {
      leds[i] = getFireColor((inoise8(i * 1, counter)));
    }
    counter += 1;
    FastLED.show();
  }
}
void confetti() {                                             // random colored speckles that blink in and fade smoothly
  fadeToBlackBy(leds, NUM_LEDS, thisfade);                    // Low values = slower fade.
  int pos = random16(NUM_LEDS);                               // Pick an LED at random.
  leds[pos] += CHSV((thishue + random16(huediff))/4 , thissat, thisbri);  // I use 12 bits for hue so that the hue increment isn't too quick.
  thishue = thishue + thisinc;                                // It increments here.
} // confetti()
void ChangeMe() {                                             // A time (rather than loop) based demo sequencer. This gives us full control over the length of each sequence.
  uint8_t secondHand = (millis() / 1000) % 15;                // IMPORTANT!!! Change '15' to a different value to change duration of the loop.
  static uint8_t lastSecond = 99;                             // Static variable, means it's only defined once. This is our 'debounce' variable.
  if (lastSecond != secondHand) {                             // Debounce to make sure we're not repeating an assignment.
    lastSecond = secondHand;
    switch(secondHand) {
      case  0: thisinc=1; thishue=192; thissat=255; thisfade=2; huediff=256; break;  // You can change values here, one at a time , or altogether.
      case  5: thisinc=2; thishue=128; thisfade=8; huediff=64; break;
      case 10: thisinc=1; thishue=random16(255); thisfade=1; huediff=16; break;      // Only gets called once, and not continuously for the next several seconds. Therefore, no rainbows.
      case 15: break;                                                                // Here's the matching 15 for the other one.
    }
  }
} // ChangeMe()
void effect1(){
  new_star++;
  fadeToBlackBy(leds, NUM_LEDS, 8);
  if (new_star>1) {
    leds[random(NUM_LEDS)]=CRGB::White;
    new_star=0;
  }
  
  FastLED.delay(30);
  FastLED.show();
}
void setPixelHeatColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);
 
  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    leds[Pixel].setRGB(255,255,heatramp);
  } else if( t192 > 0x40 ) {             // middle
    leds[Pixel].setRGB(255,heatramp,0);
  } else {                               // coolest
    leds[Pixel].setRGB(heatramp,0,0);
  }
}
void Fire(int Cooling, int Sparking, int SpeedDelay) {
  static byte heat[NUM_LEDS];
  int cooldown;
 
  // Step 1.  Cool down every cell a little
  for( int i = 0; i < NUM_LEDS; i++) {
    cooldown = random(0, ((Cooling * 10) / NUM_LEDS) + 2);
   
    if(cooldown>heat[i]) {
      heat[i]=0;
    } else {
      heat[i]=heat[i]-cooldown;
    }
  }
 
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= NUM_LEDS/2 - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    heat[NUM_LEDS-k] = (heat[NUM_LEDS-k + 1] + heat[NUM_LEDS-k + 2] + heat[NUM_LEDS-k + 2]) / 3;
  }
   
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(255) < Sparking ) {
    int y = random(7);
    int y2 = NUM_LEDS-random(7);
    heat[y] = heat[y] + random(160,255);
    heat[y2] = heat[y2] + random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < NUM_LEDS; j++) {
    setPixelHeatColor(j, heat[j] );
  }

  FastLED.show();
  FastLED.delay(SpeedDelay);
}
void Rainbow(){
  if (millis()>rainb_time_stamp+rainb_delay) {
      rainb++;
      FOR_i(0,NUM_LEDS){
        if (i+rainb>255){ leds[i]=CHSV((i+rainb)-255,255,255); }
          else          { leds[i]=CHSV(i+rainb,255,255); }
        
        if (rainb>255){rainb=0;}
      }
  rainb_time_stamp=millis();
  }
}
// Add one layer of waves into the led array
void pacifica_one_layer( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}
// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps()
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 40 );
  
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}
// Deepen the blues and greens
void pacifica_deepen_colors()
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].blue = scale8( leds[i].blue,  145); 
    leds[i].green= scale8( leds[i].green, 200); 
    leds[i] |= CRGB( 2, 5, 7);
  }
}
void pacifica_loop()
{
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011,10,13));
  sCIStart2 -= (deltams21 * beatsin88(777,8,11));
  sCIStart3 -= (deltams1 * beatsin88(501,5,7));
  sCIStart4 -= (deltams2 * beatsin88(257,4,6));

  // Clear out the LED array to a dim background blue-green
  fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_one_layer( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301) );
  pacifica_one_layer( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
  pacifica_one_layer( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503));
  pacifica_one_layer( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601));

  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps();

  // Deepen the blues and greens a bit
  pacifica_deepen_colors();
}
void plasma() {                                                 // This is the heart of this program. Sure is short. . . and fast.

  int thisPhase = beatsin8(6,-64,64);                           // Setting phase change for a couple of waves.
  int thatPhase = beatsin8(7,-64,64);

  for (int k=0; k<NUM_LEDS; k++) {                              // For each of the LED's in the strand, set a brightness based on a wave as follows:

    int colorIndex = cubicwave8((k*23)+thisPhase)/2 + cos8((k*15)+thatPhase)/2;           // Create a wave and add a phase change and add another wave with its own phase change.. Hey, you can even change the frequencies if you wish.
    int thisBright = qsuba(colorIndex, beatsin8(7,0,96));                                 // qsub gives it a bit of 'black' dead space by setting sets a minimum value. If colorIndex < current value of beatsin8(), then bright = 0. Otherwise, bright = colorIndex..

    leds[k] = ColorFromPalette(currentPalette, colorIndex, thisBright, currentBlending);  // Let's now add the foreground colour.
  }

} // plasma()
void salut(int s_count){
    static uint32_t prevTime;
    // стартовая позиция
    if (s_time[s_count]<s_tick[s_count]) {
      s_time[s_count]=random(80)+20;
      s_tick[s_count]=0;
      s_start_pos[s_count]=random(NUM_LEDS);
      s_color[s_count]=random(255);
      s_brightness[s_count]=255-random(100);
      s_scale[s_count]=random(25)+1;
    }
    
    
      s_tick[s_count]++; 
      s_brightness[s_count]=s_brightness[s_count]-(s_brightness[s_count]/12);
      s_color[s_count]=s_color[s_count]+1;
      FOR_i(1,s_scale[s_count]){
      if ((s_start_pos[s_count]+s_tick[s_count]+i)<NUM_LEDS) {leds[s_start_pos[s_count]+s_tick[s_count]+i] = CHSV(s_color[s_count],255,s_brightness[s_count]);}
      if ((s_start_pos[s_count]-s_tick[s_count]-i)>0) {leds[s_start_pos[s_count]-s_tick[s_count]-i] = CHSV(s_color[s_count],255,s_brightness[s_count]);}
      }

}

void meteorRain( int SpeedDelay) {  
  static int wave;
  EVERY_N_MILLIS(8000){
    wave=random(10);
  }
  leds[NUM_LEDS/2].setHSV(beatsin8(wave),255,255);
  leds[NUM_LEDS/2-1].setHSV(beatsin8(wave),255,255);

  FOR_i(0,NUM_LEDS/2){
    leds[i]=leds[i+1];
    leds[i].r-=random(10);
    leds[i].g-=random(10);
    leds[i].b-=random(10);
    leds[NUM_LEDS-i]=leds[NUM_LEDS-i-1];
    leds[NUM_LEDS-i].r-=random(10);
    leds[NUM_LEDS-i].g-=random(10);
    leds[NUM_LEDS-i].b-=random(10);
  }

    FastLED.show();
    FastLED.delay(SpeedDelay);

}
//-------------------------------Sparkle---------------------------------------
void Sparkle() {
  int pix=random(NUM_LEDS);

  FOR_i(0,NUM_LEDS){
     leds[i].setHSV(beat8(1),255,beatsin8(3,10,120));
     //FastLED.delay(1); 
  }
  leds[pix].setHSV(255,0,beatsin8(3,30,180));
  FastLED.show();
  }
void christmas1(){
  int color=0;
  int size=10;
  int phase;
  phase=beatsin8(3,0,60);
  //fadeToBlackBy(leds, NUM_LEDS, 24);
  if (phase>(size*3)) {phase=phase-(size*3);}
  for (int i=phase;i<NUM_LEDS;i=i+size){
    if (color>255) {color=0;}
        FOR_j(0,size){
          if (j<2) { leds[i+j].setHSV(color,255,255); }
              else { leds[i+j].setHSV(0,0,0);         }
        }
    color=color+86;
  }  
  FastLED.show();
}

void christmas2(){
  int red=beatsin8(5,0,255);
  int blue=beatsin8(4,0,255);
  int green=beatsin8(6,0,255);
  int size=10;

  for (int i=0;i<NUM_LEDS;i=i+size*3){
        FOR_j(0,size){
          if (j<2) { leds[i+j].setHSV(0,255,red); }
              else { leds[i+j].setHSV(0,0,0);         }
        }
        FOR_j(size,size*2){
          if (j<(size+2)) { leds[i+j].setHSV(86,255,blue); }
              else { leds[i+j].setHSV(0,0,0);         }
        }
        FOR_j(size*2,size*3){
          if (j<((size*2)+2)) { leds[i+j].setHSV(172,255,green); }
              else { leds[i+j].setHSV(0,0,0);         }
        }
        }
  FastLED.show();
}


//-------------------------LOOP------------------
void loop(){
  httpServer.handleClient();                   // Слушаем клиентов для обновления
  ArduinoOTA.handle();                      // проверяем обновление для прошивки по воздуху 

  R1_BUT.tick();  // обязательная функция отработки Кнопки
  R2_BUT.tick();  // обязательная функция отработки Кнопки
  L1_BUT.tick();  // обязательная функция отработки Кнопки
  L2_BUT.tick();  // обязательная функция отработки Кнопки

  if (WiFi.status() != WL_CONNECTED) {      // проверяем соединение WIFI
      Serial.println("RECONECT to WIFI");
      blink(5,CRGB::Yellow);
      setup_wifi();
    }
  if (R1_BUT.isSingle()) {quene=true;cur_mode="R";}     // проверка на один клик
  if (L1_BUT.isSingle()) {quene=true;cur_mode="L";}     // проверка на один клик
  if ((L1_BUT.isHold()) or (R1_BUT.isHold())) {quene=true;cur_mode="OFF";}
  if ((L1_BUT.isDouble()) or (R1_BUT.isDouble())) {quene=true;cur_mode="MAX";}

  if ((R2_BUT.isSingle()) or (L2_BUT.isSingle())){
    if ((left_l)or(right_l)){fadeoff();left_l=false;right_l=false; }
    auto_mode++; if (auto_mode>6) auto_mode=0;
    quene=true;
    switch (auto_mode)
    {
    case 1:
      cur_mode="fire";
      break;
    case 2:
      cur_mode="ocean";
      break;
    case 3:
      cur_mode="lava";
      break;
    case 4:
      cur_mode="fire2";
      break;
    case 5:
      cur_mode="Lazy";
      break;
    case 6:
      cur_mode="MusicMode3";
      break;
    default:
    cur_mode="OFF";
      break;
    }
  }

//if (R1_BUT.isSingle()) {blink(5,CRGB::Yellow);} 
//if (R2_BUT.isSingle()) {blink(5,CRGB::Red);}
//if (L1_BUT.isSingle()) {blink(5,CRGB::Blue);}
//if (L2_BUT.isSingle()) {blink(5,CRGB::White);}
  //if (led_mode>2) {led_mode=0;}
  
  if (quene){
      if (cur_mode=="night"){
        setBrightness(20);
        fill(CRGB::Red);        
        LEDS.show();
        quene=false;      
      }
      if (cur_mode=="MAX"){        
        MAX_bright();
        quene=false; 
      }
      if (cur_mode=="OFF"){
        fadeoff();
        //setBrightness(0);
        quene=false;
        left_l=false;
        right_l=false;
      }        
      if (cur_mode=="fire"){        
        fireTick();
      }
      if (cur_mode=="fire2"){ 
        EVERY_N_MILLISECONDS(5000) { 
          cooling_fire2=30+random(130);
        }     
        EVERY_N_MILLISECONDS(50) { 
          if (prew_cooling_fire2>cooling_fire2) {prew_cooling_fire2--;};
          if (prew_cooling_fire2<cooling_fire2) {prew_cooling_fire2++;};
        }   
        Fire(prew_cooling_fire2,120,15);
      }
      if (cur_mode=="L"){
        LEFT_light();
        quene=false;
      }       
      if (cur_mode=="sparkle"){   
        EVERY_N_MILLISECONDS(20) {  
          Sparkle();
        }
      }   
      if (cur_mode=="ocean"){     
        pacifica_loop();
        FastLED.show();
      }      
      if (cur_mode=="lava"){     
        lavaTick();
      }
      if (cur_mode=="rainbow"){
        Rainbow();
        FastLED.show();
      }   
      if (cur_mode=="meteor"){
          meteorRain( 30);
      } 
      if (cur_mode=="R"){
        RIGHT_light();
        quene=false;
      }  
      if (cur_mode=="christmas1"){
        christmas1();
      }       
      if (cur_mode=="Lazy"){
        LazyTick();
      } 
      if (cur_mode=="MusicMode1"){
        //this_mode = 0;
          audioLoop();       // главный цикл обработки и отрисовки
      }
      if (cur_mode=="MusicMode2"){
        //this_mode = 1;
          VU_meter();       // главный цикл обработки и отрисовки
      }
      if (cur_mode=="MusicMode3"){
        //this_mode = 2;
          water_fall_vu();       // главный цикл обработки и отрисовки
      }
      if (cur_mode=="salut"){
        EVERY_N_MILLISECONDS(20) {  
          fadeall();
          FOR_j(1,5){
            salut(j);
          }
          FastLED.show();
        }
      } 
      if (cur_mode=="confetti"){
          EVERY_N_MILLISECONDS(50) {                                  // FastLED based non-blocking delay to update/display the sequence.
            plasma();
          }
          EVERY_N_MILLISECONDS(100) {
            uint8_t maxChanges = 24; 
            nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);   // AWESOME palette blending capability.
          }
          EVERY_N_SECONDS(5) {                                 // Change the target palette to a random one every 5 seconds.
            uint8_t baseC = random8();                         // You can use this as a baseline colour if you want similar hues in the next line.
            targetPalette = CRGBPalette16(CHSV(baseC+random8(32), 192, random8(128,255)), CHSV(baseC+random8(32), 255, random8(128,255)), CHSV(baseC+random8(32), 192, random8(128,255)), CHSV(baseC+random8(32), 255, random8(128,255)));
          }
          FastLED.show(); 
      }   
       
      if (cur_mode=="stars"){
        effect1();
      }
  } 
}
// ccccc