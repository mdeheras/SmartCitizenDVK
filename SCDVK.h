#ifndef __SCDVK_H__
#define __SCDVK_H__

#include <Arduino.h>

class SCDVK {
public:
    void begin();
    void disable_all();
    void enable_all();
    void bootloader_flash();
    void firmware_flash();
    void terminal_mode();
    void normal_mode();
    void blink_led_blue(unsigned long interval);
    void blink_led_green(unsigned long interval);
    uint16_t readSHT21(uint8_t type);
    float getTemperature();
    float getHumidity();
    void check_MMC();
    void execute();
    void echo();
    void main();
private:
    
};
#endif
