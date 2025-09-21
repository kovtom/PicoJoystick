#include "LED.h"
#include <Arduino.h>

// LED Class constructors
LED::LED() : pin(LED_BUILTIN),
            state(false),
            procTickTime(1000),
            lastTickTime(millis()) {
    pinMode(pin, OUTPUT);
}

LED::LED(int pin) : pin(pin),
            state(false),
            procTickTime(1000),
            lastTickTime(millis()) {
    pinMode(pin, OUTPUT);
}

LED::LED(int pin, uint16_t procTickTime) : pin(pin),
            state(false),
            procTickTime(procTickTime),
            lastTickTime(millis()) {
    pinMode(pin, OUTPUT);
}

// LED Class methods
void LED::on() {
    digitalWrite(pin, HIGH);
    state = true;
}

void LED::off() {
    digitalWrite(pin, LOW);
    state = false;  
}

void LED::toggle() {
    state = !state;
    digitalWrite(pin, state ? HIGH : LOW);
}

void LED::refresh() {
    uint32_t currentTime = millis();
    if (currentTime - lastTickTime >= procTickTime) {
        toggle();
        lastTickTime = currentTime;
    }
}

void LED::refresh(uint16_t interval) {
    procTickTime = interval;
    refresh();
}
