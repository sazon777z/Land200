#line 1 "C:\\Users\\JZX\\Documents\\Arduino\\Land_Cruiser\\DisplayDriver.h"
#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include "NetworkManager.h"
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
  void drawDashboard(const WatchStateSnapshot &snapshot);
  void drawConnecting(String ssid);
  void drawQRCode(String data, int startX, int startY, int scale);
  void drawAPInfo(String ssid, String pass, String ip);
  void setBrightness(uint8_t brightness);
  void clearMainSegments();

private:
  Arduino_DataBus *bus = nullptr;
  Arduino_GFX *gfx = nullptr;
  Arduino_Canvas *clockCanvas = nullptr;
  Arduino_Canvas *infoCanvas = nullptr;

  void drawBackground();
  void drawTimePanel(const WatchStateSnapshot &snapshot, bool showColon);
  void drawWeatherPanel(const WatchStateSnapshot &snapshot);
  void drawAlarmIcon(Arduino_GFX *target, int x, int y, bool enabled,
                     uint16_t color);
  String formatTemperature(float temp);
  String getWeekdayLabel(int weekdayIndex);
  String getMonthLabel(int monthIndex);
  String getConditionLabel(const String &condition, const String &iconCode);
};

#endif // DISPLAY_DRIVER_H
