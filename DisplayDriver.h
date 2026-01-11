#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <Arduino_GFX_Library.h>
#include "config.h"

// Color Definitions (16-bit RGB565)
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

class DisplayDriver {
public:
    DisplayDriver();
    void begin();
    void drawClock(int hour, int minute, int second);
    void drawWeather(float temp, String condition, String icon);
    void drawQRCode(String data); // New QR Code method
    void drawAPInfo(String ssid, String pass, String ip); // New AP Info method
    void setBrightness(uint8_t brightness);

private:
    Arduino_DataBus *bus;
    Arduino_GFX *gfx;
    
    // Helper methods for drawing UI elements
    void drawBackground();
};

#endif // DISPLAY_DRIVER_H
