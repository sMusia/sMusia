// настройки ленты
#define NUM_LEDS 28    // количество светодиодов
#define LED_PIN 2      // пин ленты
#define BUT_PIN 3      // пин кнопки

#include "GyverButton.h"
GButton butt1(BUT_PIN);

// настройки пламени
#define HUE_GAP 30      // заброс по hue
#define FIRE_STEP 50    // шаг огня
#define HUE_START 0     // начальный цвет огня (0 красный, 80 зелёный, 140 молния, 190 розовый)
#define MIN_BRIGHT 10   // мин. яркость огня
#define MAX_BRIGHT 255  // макс. яркость огня
#define MIN_SAT 245     // мин. насыщенность
#define MAX_SAT 255     // макс. насыщенность

// для разработчиков
#include <FastLED.h>
#define ORDER_GRB       // порядок цветов ORDER_GRB / ORDER_RGB / ORDER_BRG
#define COLOR_DEBTH 3   // цветовая глубина: 1, 2, 3 (в байтах)
// на меньшем цветовом разрешении скетч будет занимать в разы меньше места,
// но уменьшится и количество оттенков и уровней яркости!

// ВНИМАНИЕ! define настройки (ORDER_GRB и COLOR_DEBTH) делаются до подключения библиотеки!
#include <microLED.h>

LEDdata leds[NUM_LEDS];  // буфер ленты типа LEDdata (размер зависит от COLOR_DEBTH)
microLED strip(leds, NUM_LEDS, LED_PIN);  // объект лента
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
// ленивая жопа
#define FOR_i(from, to) for(int i = (from); i < (to); i++)
#define FOR_j(from, to) for(int j = (from); j < (to); j++)

void setup() {
  strip.setBrightness(255);
}

// возвращает цвет огня для одного пикселя
LEDdata getFireColor(int val) {
  // чем больше val, тем сильнее сдвигается цвет, падает насыщеность и растёт яркость
  return mHSV(
           HUE_START + map(val, 0, 255, 0, HUE_GAP),                    // H
           constrain(map(val, 0, 255, MAX_SAT, MIN_SAT), 0, 255),       // S
           constrain(map(val, 0, 255, MIN_BRIGHT, MAX_BRIGHT), 0, 255)  // V
         );
}

void fireTick() {
  static uint32_t prevTime;

  // двигаем пламя
  if (millis() - prevTime > 20) {
    prevTime = millis();
    int thisPos = 0, lastPos = 0;
    FOR_i(0, NUM_LEDS) {
      leds[i] = getFireColor((inoise8(i * FIRE_STEP, counter)));
    }
    counter += 20;
    strip.show();
  }
}

void effect1(){
  new_star++;
  if (new_star>50) {
    leds[random(28)]=mRGB(255,255,255);
    new_star=0;
  }
  FOR_i(0,NUM_LEDS){
    if (strip.getColorHEX(i)>0x0){
      strip.fade(i,1);
    } 
    //leds[i]= mRGB(random(255),random(255),random(255));
  }
  strip.show();
}

void effect2(){ 
  if (strip.getColor(27)== mRGB(255,255,255)){} else
  {
    strip.clear();
    FOR_i(0,NUM_LEDS){
      leds[i]= mRGB(255,255,255);
      delay(20);
      strip.show();
    }
  }
  //strip.show();
}

void effect3(){ 
  delay(50);
  strip.setHSV(random(28),random(255),255,255);
  strip.fade(random(28),50);
  strip.show();
}

void effect4(){ 
  delay(20);
  eff4++;
  long rnd=random(3,20);
  if (eff4>NUM_LEDS*rnd){
    eff4_color=random(255);
    eff4=0;
  }
  if (eff4<NUM_LEDS) {strip.setHSV(eff4,eff4_color,255,255);} 
  
  if (eff4_i<NUM_LEDS) {eff4_i++;}  
  FOR_i(0,eff4_i){
    strip.fade(i,2);
  }
  strip.show();
}

void effect5(){ 
  delay(20);
  //long rnd=random(3,20);
  eff5++;
  if (eff5>10){
    eff5_color=random(255);
    eff5=0;
  }
  strip.setHSV(0,eff5_color,255,255);
  FOR_i(1,NUM_LEDS){
    //leds[i+1]=LEDbuffer[i];
    strip.setHEX(NUM_LEDS-i,strip.getColorHEX(NUM_LEDS-i-1));
  }
  strip.show();
}

void effect6(){ 
  delay(100);
  //long rnd=random(3,20);
  eff6++;
  if (eff6>255){
    eff6=0;
  }
  //strip.setHSV(0,eff6,255,255);
  FOR_i(0,NUM_LEDS){
    //leds[i+1]=LEDbuffer[i];
    strip.setHSV(i,eff6,255,255);
  }
  strip.show();
}

void loop(){

  butt1.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
  if (butt1.isSingle()) {led_mode++;}     // проверка на один клик
  if (led_mode>6) {led_mode=0;}

  switch (led_mode)
  {
  case 0:
    fireTick();
    break;
  
  case 1:
    effect1();
    break;

  case 2:
    effect2();
    break;

  case 3:
    effect3();
    break;

  case 4:
    effect4();
    break;

  case 5:
    effect5();
    break;

  case 6:
    effect6();
    break;

  default:
    break;
  }  
}