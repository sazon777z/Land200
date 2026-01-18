#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include "Digits_Data.h"
#include "FreeSansBold12pt7b.h"
#include "config.h"
#include <Arduino_GFX_Library.h>

// Color Definitions (16-bit RGB565)
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

class DisplayDriver {
public:
  DisplayDriver();
  void begin();
  void drawClock(int hour, int minute, int second);
  void drawWeather(float temp, String condition, String icon);
  void drawConnecting(String ssid);
  void drawQRCode(String data, int startX, int startY, int scale);
  void drawAPInfo(String ssid, String pass, String ip);
  void setBrightness(uint8_t brightness);

private:
  Arduino_DataBus *bus;
  Arduino_GFX *gfx;
  Arduino_Canvas *clockCanvas;   // Буфер для часов (без мерцания)
  Arduino_Canvas *weatherCanvas; // Буфер для погоды (без мерцания)

  // Helper methods for drawing UI elements
  void drawBackground();
};

#endif // DISPLAY_DRIVER_H
