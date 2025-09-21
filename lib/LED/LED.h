
#ifndef LED_H
#define LED_H

#include <stdint.h>

class LED {
public:
    LED();
    LED(int pin);
    LED(int pin, uint16_t procTickTime);
    void on();
    void off();
    void toggle();
    void refresh();
    void refresh(uint16_t interval);
    void setInterval(uint16_t interval) { procTickTime = interval; }
private:
    int pin;
    bool state;
    uint16_t procTickTime;
    uint32_t lastTickTime;
};

#endif // LED_H
