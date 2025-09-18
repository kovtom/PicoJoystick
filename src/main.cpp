#include <Arduino.h>

int LED = LED_BUILTIN;
int counter = 0;

void setup() {
  pinMode(LED, OUTPUT);
}

void loop() {
  digitalWrite(LED, HIGH);

  digitalWrite(LED, LOW);

  counter++;
}
