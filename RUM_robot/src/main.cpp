#include <Arduino.h>
#define led_pin 15
#define relay_pin 13

void setup() {
  pinMode(led_pin,OUTPUT);
  pinMode(relay_pin,OUTPUT);
  // put your setup code here, to run once:
}

void loop() {
  int off_delay;
  int on_delay;
  off_delay = random(50)*10+50;
  on_delay = random(2000)+100;
  digitalWrite(led_pin,HIGH);
  digitalWrite(relay_pin,HIGH);
  delay(on_delay);
  digitalWrite(led_pin,LOW);
  digitalWrite(relay_pin,LOW);
  delay(off_delay);
  // put your main code here, to run repeatedly:
}