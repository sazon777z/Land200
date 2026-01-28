#include "DisplayDriver.h"
#include "qrcode.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>

// Шрифты
// Шрифты
#include "ClockFont.h"          // Векторные цифры
#include "FreeSans9pt7b.h"      // Для мелкого текста
#include "FreeSansBold18pt7b.h" // Для крупной температуры
#include "FreeSansBold24pt7b.h" // Для очень крупной температуры (опционально)

// Пользовательский шрифт для русского
#include "RusFont.h"

// Переменные состояния
static int prevHour = -1;
static int prevMinute = -1;
static int prevSecond = -1;
static float prevTemp = -999;
static String prevCondition = "";
static bool colonVisible = true;

// Helper function to decode UTF-8 to CP1251 (standard mapping for many GFX
// fonts)
String utf8ToWin1251(String source) {
  String target = "";
  unsigned char n;
  char c;
  for (int i = 0; i < source.length(); i++) {
    c = source.charAt(i);
    n = (unsigned char)c;
    if (n >= 192) {
      if (n == 208) {
        n = (unsigned char)source.charAt(++i);
        if (n == 129) {
          target += (char)168; // Yo
        } else {
          target += (char)(n + 48);
        }
      } else if (n == 209) {
        n = (unsigned char)source.charAt(++i);
        if (n == 145) {
          target += (char)184; // yo
        } else {
          target += (char)(n + 112);
        }
      }
    } else {
      target += c;
    }
  }
  return target;
}

DisplayDriver::DisplayDriver() {
  // ST7789 172x320
  // IPS is usually true for these modules
  // Rotation: 0 (Portrait) -> 172x320
  // Adjust pixel offsets if needed (0, 34) is common for 1.47" modules centered
  // in 240x320 controller memory
  bus = new Arduino_HWSPI(PIN_LCD_DC, PIN_LCD_CS, PIN_LCD_SCK, PIN_LCD_MOSI);

  // TZT 1.47" ST7789 often needs offset.
  // Common offsets for 172x320 in ST7789: (34, 0) or (0, 34).
  // Rotation 3 (Inverted Landscape).
  // Rotation 1 was cutting off Top (Red). Trying Rotation 3.
  gfx = new Arduino_ST7789(bus, PIN_LCD_RST, 3 /* rotation */, true /* IPS */,
                           172, 320, 34, 0);

  // Initialize Clock Canvas
  // Width 320, Height 80 (enough for digits)
  // Y Position: Center of screen (172/2 - 80/2) = 46
  clockCanvas = new Arduino_Canvas(320, 80, gfx, 0, 46);

  // Initialize Weather Canvas
  // Top canvas for Temp: drawn directly
  // Bottom canvas for Text: 320x45, y=127
  weatherCanvas = new Arduino_Canvas(320, 45, gfx, 0, 127);
}

void DisplayDriver::begin() {
  gfx->begin();
  gfx->fillScreen(BLACK);

#ifdef PIN_LCD_BL
  pinMode(PIN_LCD_BL, OUTPUT);
  ledcAttach(PIN_LCD_BL, 5000, 8);
  ledcWrite(PIN_LCD_BL, 255);
#endif

  clockCanvas->begin();
  clockCanvas->fillScreen(BLACK);

  weatherCanvas->begin();
  weatherCanvas->fillScreen(BLACK);

  drawBackground();
}

void DisplayDriver::setBrightness(uint8_t brightness) {
#ifdef PIN_LCD_BL
  ledcWrite(PIN_LCD_BL, brightness);
#endif
}

void DisplayDriver::drawBackground() { gfx->fillScreen(BLACK); }

void DisplayDriver::clearMainSegments() { gfx->fillScreen(BLACK); }

void DisplayDriver::drawClock(int hour, int minute, int second) {
  bool needRefresh =
      (hour != prevHour || minute != prevMinute || second != prevSecond);
  if (!needRefresh)
    return;

  prevHour = hour;
  prevMinute = minute;
  prevSecond = second;
  colonVisible = !colonVisible;

  clockCanvas->fillScreen(BLACK);

  // Центрирование часов
  // Используем векторный шрифт
  int h = 70; // Высота цифр (макс 80)
  int w = ClockDigits::getWidth(h);
  int colonW = ClockDigits::getColonWidth(h);
  int gap = 8; // Отступ между цифрами

  int totalWidth = (4 * w) + colonW + (4 * gap);
  int startX = (320 - totalWidth) / 2;
  int yPos = (80 - h) / 2;

  // Часы Tens
  ClockDigits::draw(clockCanvas, startX, yPos, h, WHITE, hour / 10);
  startX += w + gap;

  // Часы Units
  ClockDigits::draw(clockCanvas, startX, yPos, h, WHITE, hour % 10);
  startX += w + gap;

  // Двоеточие
  if (colonVisible) {
    ClockDigits::drawColon(clockCanvas, startX, yPos, h, WHITE);
  }
  startX += colonW + gap;

  // Минуты Tens
  ClockDigits::draw(clockCanvas, startX, yPos, h, WHITE, minute / 10);
  startX += w + gap;

  // Минуты Units
  ClockDigits::draw(clockCanvas, startX, yPos, h, WHITE, minute % 10);

  clockCanvas->flush();
}

void DisplayDriver::drawWeather(float temp, String condition) {
  bool tempChanged = (abs(temp - prevTemp) >= 0.1);
  bool conditionChanged = (condition != prevCondition);

  if (!tempChanged && !conditionChanged && prevTemp != -999)
    return;

  prevTemp = temp;
  prevCondition = condition;

  // 1. Рисуем температуру СВЕРХУ (напрямую в GFX, т.к. редко обновляется)
  // Очищаем область температуры: 0, 0, 320, 45 (не перекрывая часы)
  gfx->fillRect(0, 0, 320, 45, BLACK);

  gfx->setFont(&FreeSansBold24pt7b);
  gfx->setTextColor(WHITE);

  String tempStr = String((int)temp);

  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(tempStr, 0, 0, &x1, &y1, &w, &h);

  // Центрируем
  int tx = (320 - w) / 2 - x1;
  int ty = 38; // Baseline ближе к верху

  gfx->setCursor(tx, ty);
  gfx->print(tempStr);

  // Градус
  gfx->drawCircle(tx + w + 5, ty - h + 10, 3, WHITE);
  gfx->drawCircle(tx + w + 5, ty - h + 10, 4, WHITE);

  // 2. Рисуем описание погоды СНИЗУ (через canvas, так как может быть скролл
  // или длинный текст)
  weatherCanvas->fillScreen(BLACK);

  // Используем шрифт с поддержкой русского
  weatherCanvas->setFont(&RusFont);
  weatherCanvas->setTextColor(WHITE);
  weatherCanvas->setTextSize(1);

  // Конвертация UTF-8 в Win1251 (если шрифт требует)
  // String rusCond = utf8ToWin1251(condition);
  // Если пользователь сделал шрифт Unicode, то просто condition.
  // Попробуем конвертировать, так как 'RusFont' часто CP1251.
  String rusCond = utf8ToWin1251(condition);

  // Центрируем и переносим по словам
  // Простая реализация - просто выводим по центру, если не влезает - обрезается

  weatherCanvas->getTextBounds(rusCond, 0, 0, &x1, &y1, &w, &h);
  int cx = (320 - w) / 2 - x1;
  int cy = 30; // Center of canvas (height 45)

  weatherCanvas->setCursor(cx, cy);
  weatherCanvas->print(rusCond);

  weatherCanvas->flush();

  // Сбрасываем шрифт
  gfx->setFont(NULL);
}

void DisplayDriver::drawConnecting(String ssid) {
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE);
  gfx->setFont(&FreeSans9pt7b);

  String msg = "Connecting...";
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(msg.c_str(), 0, 0, &x1, &y1, &w, &h);

  gfx->setCursor((320 - w) / 2, 86);
  gfx->print(msg);
  gfx->setFont(NULL);
}

void DisplayDriver::drawQRCode(String data, int startX, int startY, int scale) {
  QRCode qrcode;
  uint8_t version = 3;
  uint8_t qrcodeData[qrcode_getBufferSize(version)];
  qrcode_initText(&qrcode, qrcodeData, version, ECC_LOW, data.c_str());

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

  String qrData = "WIFI:S:" + ssid + ";T:WPA;P:" + pass + ";;";

  // QR Scaled to fit 320x172 side by side
  // Ver 3 = 29 modules. 29 * 4 = 116px.
  // 320 width allows Side-by-Side.
  int qrScale = 4;
  int qrSize = 29 * qrScale;
  int qrX = 20; // Немного отступа слева
  int qrY = (172 - qrSize) / 2;

  drawQRCode(qrData, qrX, qrY, qrScale);

  gfx->setFont(&FreeSans9pt7b);
  gfx->setTextSize(1);

  String url = "lc200.local";
  int16_t tx1, ty1;
  uint16_t tw, th;
  gfx->getTextBounds(url.c_str(), 0, 0, &tx1, &ty1, &tw, &th);

  // Text on the right
  // Центрируем текст по середине оставшегося пространства справа
  int remainingWidth = 320 - (qrX + qrSize);
  int tx = (qrX + qrSize) + (remainingWidth - tw) / 2;

  // Вертикально центрируем относительно QR кода (Center-to-Center)
  // Center Y of QR matches Center Y of screen (172/2 = 86)
  // Text Baseline Y = CenterY - th/2 - ty1;
  int centerY = 86;
  int ty = centerY - (th / 2) - ty1;

  gfx->setTextColor(CYAN);
  gfx->setCursor(tx, ty);
  gfx->print(url);

  // Also print SSID/IP if needed? maybe just URL is enough as per previous req.
  gfx->setFont(NULL);
}
