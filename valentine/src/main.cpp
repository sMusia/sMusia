#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
// настройки ленты
#define NUM_LEDS 14    // количество светодиодов
#define BUT_PIN 2      // пин кнопки 
#define DATA_PIN 1 // Пин ленты
int mode;
// настройки Кнопок
#include "GyverButton.h"
GButton BUT(BUT_PIN,LOW_PULL,NORM_OPEN);

// This is an array of leds.  One item for each led in your strip.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, NEO_GRB);

void startup(){
      for (size_t i = 0; i < 10; i++)
      {
        strip.fill(strip.ColorHSV(0, 255, i*25),0,NUM_LEDS);
        strip.show();
        delay(50);
      }
      delay(100);
      for (size_t i = 0; i < 20; i++)
      {
        strip.fill(strip.ColorHSV(0, 255, 250-(i*12)),0,NUM_LEDS);
        strip.show();
        delay(85);
      } 
  strip.clear();
  strip.show();
}

void setup() {
  // put your setup code here, to run once:
  strip.begin();
  strip.setBrightness(50);
  strip.show();
  startup();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void Heart_tick(){
  for (size_t k = 0; k < 2; k++)
  {  
      for (size_t i = 0; i < 10; i++)
      {
        strip.fill(strip.ColorHSV(0, 255, i*25),0,NUM_LEDS);
        strip.show();
        delay(20);
      }
      delay(100);
      for (size_t i = 0; i < 5; i++)
      {
        strip.fill(strip.ColorHSV(0, 255, 250-(i*50)),0,NUM_LEDS);
        strip.show();
        delay(20);
      } 
  }
  //strip.fill(strip.Color(0, 0, 0),0,NUM_LEDS);
  strip.clear();
  strip.show();
}
void Game(){
  for (size_t i = 0; i < 2; i++)
  {
    strip.fill(strip.ColorHSV(random(65000), 255, 70*(i+1)),0,NUM_LEDS);
    strip.show();
    delay(500);
    strip.fill(strip.ColorHSV(random(65000), 255, 0),0,NUM_LEDS);
    strip.show();
    delay(500);
  }
  int c;
  int r=random(300)+50; 
  for (size_t i = 0; i < r; i++)
  {
    c++;
    if (c>NUM_LEDS-1) {c=0;}
    strip.fill(strip.ColorHSV(0, 0, 0),0,NUM_LEDS);
    strip.setPixelColor(c,strip.ColorHSV(i*100));
    strip.show();
    if (i+20>r){ delay(10*(20-(r-i)));}
    delay(i/2);
  }
  delay(5000);  
  strip.clear();
  strip.show();
}

void Effect2(){
  int col=random(65500);

strip.setPixelColor(6,strip.ColorHSV(col, 255, 255));
strip.show();
delay(100);

strip.setPixelColor(5,strip.ColorHSV(col, 255, 255));
strip.setPixelColor(7,strip.ColorHSV(col, 255, 255));
strip.show();
delay(100);

strip.setPixelColor(4,strip.ColorHSV(col, 255, 255));
strip.setPixelColor(8,strip.ColorHSV(col, 255, 255));
strip.show();
delay(100);

strip.setPixelColor(3,strip.ColorHSV(col, 255, 255));
strip.setPixelColor(9,strip.ColorHSV(col, 255, 255));
strip.show();
delay(100);

strip.setPixelColor(2,strip.ColorHSV(col, 255, 255));
strip.setPixelColor(10,strip.ColorHSV(col, 255, 255));
strip.show();
delay(100);

strip.setPixelColor(1,strip.ColorHSV(col, 255, 255));
strip.setPixelColor(11,strip.ColorHSV(col, 255, 255));
strip.show();
delay(100);

strip.setPixelColor(0,strip.ColorHSV(col, 255, 255));
strip.setPixelColor(12,strip.ColorHSV(col, 255, 255));
strip.show();
delay(100);

strip.setPixelColor(13,strip.ColorHSV(col, 255, 255));
strip.show();
delay(100);

strip.setPixelColor(6,strip.ColorHSV(col, 255, 0));
strip.show();
delay(100);

strip.setPixelColor(5,strip.ColorHSV(col, 255, 0));
strip.setPixelColor(7,strip.ColorHSV(col, 255, 0));
strip.show();
delay(100);

strip.setPixelColor(4,strip.ColorHSV(col, 255, 0));
strip.setPixelColor(8,strip.ColorHSV(col, 255, 0));
strip.show();
delay(100);

strip.setPixelColor(3,strip.ColorHSV(col, 255, 0));
strip.setPixelColor(9,strip.ColorHSV(col, 255, 0));
strip.show();
delay(100);

strip.setPixelColor(2,strip.ColorHSV(col, 255, 0));
strip.setPixelColor(10,strip.ColorHSV(col, 255, 0));
strip.show();
delay(100);

strip.setPixelColor(1,strip.ColorHSV(col, 255, 0));
strip.setPixelColor(11,strip.ColorHSV(col, 255, 0));
strip.show();
delay(100);

strip.setPixelColor(0,strip.ColorHSV(col, 255, 0));
strip.setPixelColor(12,strip.ColorHSV(col, 255, 0));
strip.show();
delay(100);

strip.setPixelColor(13,strip.ColorHSV(col, 255, 0));
strip.show();
delay(100);
}

void Effect1(){
  int k;
    for (size_t i = 0; i < NUM_LEDS*10; i++)
    {
      k++;
      if (k>NUM_LEDS){k=0;}
      strip.setPixelColor(k,strip.ColorHSV(i*460, 255, 255));
      strip.show();
      delay(20);
    }
    colorWipe(strip.Color(0,0,0),50);
}

void loop() {
  // put your main code here, to run repeatedly:

  BUT.tick();
  if (BUT.isHold()){
    Game();
  }
  if (BUT.isClick()){
    mode++;
    if (mode>3) {mode=0;}
    if (mode==1) {Heart_tick();}
    if (mode==0) {
      int r=random(255);
      int g=random(255);
      int b=random(255);
      colorWipe(strip.Color(r,g,b),50);
      delay(100);
      colorWipe(strip.Color(0,0,0),50);
    }
    if (mode==2) {Effect2();}
    if (mode==3) {Effect1();}
  } 
}

