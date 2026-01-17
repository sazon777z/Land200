#include "DisplayDriver.h"
#include "qrcode.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>

// Шрифты Adafruit GFX для красивого отображения (используем локальные копии)
#include "FreeSansBold18pt7b.h" // Для масштабирования x2 (~72px)
#include "FreeSansBold24pt7b.h"
#include "WeatherIcons_Bitmap.h" // Наши новые ОБЪЕМНЫЕ иконки погоды

// Предыдущие значения для предотвращения мерцания
static int prevHour = -1;
static int prevMinute = -1;
static int prevSecond = -1;
static float prevTemp = -999;
static String prevCondition = "";
static String prevIconStr = "";
static bool colonVisible = true;

DisplayDriver::DisplayDriver() {
  // Initialize ILI9341 on the specified pins
  // Hardware SPI used for improved performance
  bus = new Arduino_HWSPI(PIN_LCD_DC, PIN_LCD_CS, PIN_LCD_SCK, PIN_LCD_MOSI);

  // ILI9341 240x320
  gfx =
      new Arduino_ILI9341(bus, PIN_LCD_RST, 0 /* rotation */, false /* IPS */);
}

void DisplayDriver::begin() {
  gfx->begin();
  gfx->fillScreen(BLACK);

  // Initialize backlight
  pinMode(PIN_LCD_BL, OUTPUT);
  analogWrite(PIN_LCD_BL, 255); // Full brightness initially

  drawBackground();
}

void DisplayDriver::setBrightness(uint8_t brightness) {
  analogWrite(PIN_LCD_BL, brightness);
}

void DisplayDriver::drawBackground() {
  // Чистый фон без рамки для современного дизайна
  gfx->fillScreen(BLACK);
}

void DisplayDriver::drawClock(int hour, int minute, int second) {
  // Мигание двоеточия каждую секунду
  bool needRefresh =
      (hour != prevHour || minute != prevMinute || second != prevSecond);

  if (!needRefresh)
    return;

  prevHour = hour;
  prevMinute = minute;
  prevSecond = second;
  colonVisible = !colonVisible;

  // Очищаем ВЕРХНЮЮ половину экрана
  gfx->fillRect(0, 40, SCREEN_WIDTH, 110, BLACK);

  // Используем САМЫЙ КРУПНЫЙ шрифт
  gfx->setFont(&FreeSansBold24pt7b);
  gfx->setTextSize(2);

  char timeStr[6];
  if (colonVisible) {
    sprintf(timeStr, "%02d:%02d", hour, minute);
  } else {
    sprintf(timeStr, "%02d %02d", hour, minute);
  }

  // Точный расчет границ для центрирования
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);

  int xPos = (SCREEN_WIDTH - w) / 2;
  int yPos = 125;

  // 1. Эффект тени (глубина)
  gfx->setTextColor(CLR_DGRAY);
  gfx->setCursor(xPos + 4, yPos + 4);
  gfx->print(timeStr);

  // 2. Основной текст
  gfx->setTextColor(WHITE);
  gfx->setCursor(xPos, yPos);
  gfx->print(timeStr);

  gfx->setFont(NULL);
  gfx->setTextSize(1);
}

void DisplayDriver::drawWeather(float temp, String condition, String icon) {
  bool isNight = icon.endsWith("n");

  // Check what changed
  bool tempChanged = (abs(temp - prevTemp) >= 0.1);
  bool conditionChanged = (condition != prevCondition || icon != prevIconStr);

  if (!tempChanged && !conditionChanged)
    return;

  // Draw Separator only if it might have been cleared or first run
  // (simplification: just draw it if anything changed, it's fast) Or better:
  // Draw it once? For now, we assume background is black. Let's protect the
  // separator line area (y=180). We draw below it.
  if (prevTemp == -999) { // First run
    gfx->drawLine(20, 180, SCREEN_WIDTH - 20, 180, CLR_DGRAY);
  }

  // 1. UPDATE ICON
  if (conditionChanged || prevTemp == -999) {
    prevCondition = condition;
    prevIconStr = icon;

    // Clear Icon Area (Left side)
    // x=0 to 120, y=185 to 320
    gfx->fillRect(0, 185, 120, 135, BLACK);

    int iconX = 10;
    int iconY = 195;

    if (condition == "Clear") {
      if (isNight)
        drawMoonVolumetric(gfx, iconX, iconY);
      else
        drawSunVolumetric(gfx, iconX, iconY);
    } else if (condition == "Clouds")
      drawCloudVolumetric(gfx, iconX, iconY);
    else if (condition == "Rain" || condition == "Drizzle")
      drawRainVolumetric(gfx, iconX, iconY);
    else if (condition == "Thunderstorm")
      drawThunderVolumetric(gfx, iconX, iconY);
    else if (condition == "Mist" || condition == "Fog" || condition == "Haze")
      drawFogVolumetric(gfx, iconX, iconY);
    else if (condition == "Snow")
      drawSnowVolumetric(gfx, iconX, iconY);
    else
      drawCloudVolumetric(gfx, iconX, iconY);
  }

  // 2. UPDATE TEMP
  if (tempChanged || prevTemp == -999) {
    prevTemp = temp;

    // Clear Temp Area (Right side)
    // x=120 to 240, y=185 to 320
    gfx->fillRect(120, 185, 120, 135, BLACK);

    gfx->setFont(&FreeSansBold24pt7b);
    gfx->setTextSize(1);

    char tempStr[10];
    sprintf(tempStr, "%.1f", temp);

    int16_t tx1, ty1;
    uint16_t tw, th;
    gfx->getTextBounds(tempStr, 0, 0, &tx1, &ty1, &tw, &th);

    int txPos = SCREEN_WIDTH - tw - 45; // Leave space for degree symbol
    int tyPos = 265;

    // Shadow
    gfx->setTextColor(CLR_DEEP_BLUE);
    gfx->setCursor(txPos + 2, tyPos + 2);
    gfx->print(tempStr);

    // Main Text
    gfx->setTextColor(CYAN);
    gfx->setCursor(txPos, tyPos);
    gfx->print(tempStr);

    // Degree Symbol
    int degX = txPos + tw + 8;
    int degY = tyPos - th + 5;
    gfx->fillCircle(degX, degY, 4, CYAN);
    gfx->drawCircle(degX, degY, 5, WHITE);

    gfx->setFont(NULL);
  }
}

void DisplayDriver::drawConnecting(String ssid) {
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);

  gfx->setCursor(20, 100);
  gfx->print("Connecting...");

  gfx->setTextSize(1);
  gfx->setCursor(20, 140);
  gfx->print("SSID: ");
  gfx->print(ssid);

  gfx->setCursor(20, 170);
  gfx->setTextColor(CYAN);
  gfx->print("Please wait up to 20s");
}

// Function to draw QR Code
// Requires 'qrcode' library by Richard Moore to be installed

void DisplayDriver::drawQRCode(String data) {
  QRCode qrcode;
  uint8_t version = 3;
  uint8_t qrcodeData[qrcode_getBufferSize(version)];
  qrcode_initText(&qrcode, qrcodeData, version, ECC_LOW, data.c_str());

  int scale = 4; // 29*4 = 116px size
  int startX = (SCREEN_WIDTH - qrcode.size * scale) / 2;
  int startY = (SCREEN_HEIGHT - qrcode.size * scale) / 2 - 20;

  // Draw background for QR
  gfx->fillRect(startX - 5, startY - 5, (qrcode.size * scale) + 10,
                (qrcode.size * scale) + 10, WHITE);

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        gfx->fillRect(startX + x * scale, startY + y * scale, scale, scale,
                      BLACK);
      }
    }
  }
}

void DisplayDriver::drawAPInfo(String ssid, String pass, String ip) {
  gfx->fillScreen(BLACK);

  // Header
  gfx->setTextColor(CYAN, BLACK);
  gfx->setTextSize(2);
  gfx->setCursor(20, 10);
  gfx->print("Scan to Config"); // Changed title

  // QR Code - WiFi Connection string
  // Format: WIFI:S:<SSID>;T:WPA;P:<PASS>;;
  String qrData = "WIFI:S:" + ssid + ";T:WPA;P:" + pass + ";;";
  drawQRCode(qrData);

  // Text Info under QR
  int textY = 230;
  gfx->setTextColor(WHITE, BLACK);
  gfx->setTextSize(1);

  gfx->setCursor(10, textY);
  gfx->print("1. Connect WiFi:");

  gfx->setCursor(10, textY + 15);
  gfx->setTextColor(YELLOW, BLACK);
  gfx->print("SSID: ");
  gfx->print(ssid);

  gfx->setCursor(10, textY + 30);
  gfx->print("Pass: ");
  gfx->print(pass);

  gfx->setCursor(10, textY + 50);
  gfx->setTextColor(GREEN, BLACK);
  gfx->print("2. Go to URL:");
  gfx->setCursor(10, textY + 65);
  gfx->print("artemLandCruiser.local");
}
