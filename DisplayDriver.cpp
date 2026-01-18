#include "DisplayDriver.h"
#include "qrcode.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>

// Шрифты Adafruit GFX для красивого отображения (используем локальные копии)
#include "Digits_Data.h"
#include "FreeSans9pt7b.h"      // Новый уменьшенный шрифт для AP Info
#include "FreeSansBold18pt7b.h" // Для масштабирования x2 (~72px)
#include "FreeSansBold24pt7b.h"
#include "WeatherIcons_Bitmap.h" // Наши новые ОБЪЕМНЫЕ иконки погоды

// Определение массива указателей на цифры (было перенесено из Digits_Data.h для
// устранения multiple definition)
const uint16_t *digits_all[] = {digit_0, digit_1, digit_2, digit_3, digit_4,
                                digit_5, digit_6, digit_7, digit_8, digit_9};

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

  // Initialize Clock Canvas (Buffer)
  // Position y=40 on hardware, size 240x100
  clockCanvas = new Arduino_Canvas(240, 100, gfx, 0, 40);

  // Initialize Weather Canvas (Buffer)
  // Position y=180 on hardware, size 240x135
  weatherCanvas = new Arduino_Canvas(240, 135, gfx, 0, 180);
}

// PWM Channel for Backlight (0-15)
// PWM Channel definition removed for ESP32 Core 3.x compatibility

void DisplayDriver::begin() {
  gfx->begin();
  gfx->fillScreen(BLACK);

// Initialize backlight with PWM
#ifdef PIN_LCD_BL
  pinMode(PIN_LCD_BL, OUTPUT);
  ledcAttach(PIN_LCD_BL, 5000, 8); // 5 kHz, 8 bit resolution
  ledcWrite(PIN_LCD_BL, 255);      // Full brightness initially
  Serial.println("Display: Backlight PWM initialized");
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
  // gfx->flush() не нужен для подсветки, так как это аппаратный PWM
#endif
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

  // 1. Очищаем буфер часов
  clockCanvas->fillScreen(BLACK);

  // 2. Рисуем цифры в буфер
  int totalWidth = (4 * DIGIT_WIDTH) + COLON_WIDTH + (4 * 2);
  int startX = (240 - totalWidth) / 2;
  int yPos = (100 - DIGIT_HEIGHT) / 2;

  // Часы
  clockCanvas->draw16bitRGBBitmap(startX, yPos,
                                  (uint16_t *)digits_all[hour / 10],
                                  DIGIT_WIDTH, DIGIT_HEIGHT);
  startX += DIGIT_WIDTH + 2;
  clockCanvas->draw16bitRGBBitmap(startX, yPos,
                                  (uint16_t *)digits_all[hour % 10],
                                  DIGIT_WIDTH, DIGIT_HEIGHT);
  startX += DIGIT_WIDTH + 2;

  // Двоеточие
  if (colonVisible) {
    clockCanvas->draw16bitRGBBitmap(startX, yPos, (uint16_t *)digit_colon,
                                    COLON_WIDTH, DIGIT_HEIGHT);
  }
  startX += COLON_WIDTH + 2;

  // Минуты
  clockCanvas->draw16bitRGBBitmap(startX, yPos,
                                  (uint16_t *)digits_all[minute / 10],
                                  DIGIT_WIDTH, DIGIT_HEIGHT);
  startX += DIGIT_WIDTH + 2;
  clockCanvas->draw16bitRGBBitmap(startX, yPos,
                                  (uint16_t *)digits_all[minute % 10],
                                  DIGIT_WIDTH, DIGIT_HEIGHT);

  // 3. Выводим буфер на экран
  clockCanvas->flush();
}

void DisplayDriver::drawWeather(float temp, String condition, String icon) {
  bool isNight = icon.endsWith("n");

  // Проверка изменений
  bool tempChanged = (abs(temp - prevTemp) >= 0.1);
  bool conditionChanged = (condition != prevCondition || icon != prevIconStr);

  if (!tempChanged && !conditionChanged && prevTemp != -999)
    return;

  // Сохраняем значения
  prevTemp = temp;
  prevCondition = condition;
  prevIconStr = icon;

  // 1. Очищаем буфер погоды
  weatherCanvas->fillScreen(BLACK);

  // 2. Рисуем разделительную линию (в буфере y=0 соответствует y=180 на экране)
  weatherCanvas->drawLine(20, 0, 220, 0, CLR_DGRAY);

  // 3. Рисуем иконку (в буфере)
  int iconX = 10;
  int iconY = 15;

  if (condition == "Clear") {
    if (isNight)
      drawMoonVolumetric(weatherCanvas, iconX, iconY);
    else
      drawSunVolumetric(weatherCanvas, iconX, iconY);
  } else if (condition == "Clouds")
    drawCloudVolumetric(weatherCanvas, iconX, iconY);
  else if (condition == "Rain" || condition == "Drizzle")
    drawRainVolumetric(weatherCanvas, iconX, iconY);
  else if (condition == "Thunderstorm")
    drawThunderVolumetric(weatherCanvas, iconX, iconY);
  else if (condition == "Mist" || condition == "Fog" || condition == "Haze")
    drawFogVolumetric(weatherCanvas, iconX, iconY);
  else if (condition == "Snow")
    drawSnowVolumetric(weatherCanvas, iconX, iconY);
  else
    drawCloudVolumetric(weatherCanvas, iconX, iconY);

  // 4. Рисуем температуру (в буфере)
  weatherCanvas->setFont(&FreeSansBold24pt7b);
  weatherCanvas->setTextSize(1);

  char tempStr[10];
  sprintf(tempStr, "%.1f", temp);

  int16_t tx1, ty1;
  uint16_t tw, th;
  weatherCanvas->getTextBounds(tempStr, 0, 0, &tx1, &ty1, &tw, &th);

  int txPos = 240 - tw - 20;
  int tyPos = 80;

  // Тень
  weatherCanvas->setTextColor(CLR_DEEP_BLUE);
  weatherCanvas->setCursor(txPos + 2, tyPos + 2);
  weatherCanvas->print(tempStr);

  // Основной текст
  weatherCanvas->setTextColor(CYAN);
  weatherCanvas->setCursor(txPos, tyPos);
  weatherCanvas->print(tempStr);

  // Символ градуса
  int degX = txPos + tw + 8;
  int degY = tyPos - th + 5;
  weatherCanvas->fillCircle(degX, degY, 4, CYAN);
  weatherCanvas->drawCircle(degX, degY, 5, WHITE);

  weatherCanvas->setFont(NULL);

  // 5. Выводим буфер погоды на экран
  weatherCanvas->flush();
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

void DisplayDriver::drawQRCode(String data, int startX, int startY, int scale) {
  QRCode qrcode;
  uint8_t version = 3; // 29x29 modules
  uint8_t qrcodeData[qrcode_getBufferSize(version)];
  qrcode_initText(&qrcode, qrcodeData, version, ECC_LOW, data.c_str());

  // Draw white background for QR with padding
  gfx->fillRect(startX - 10, startY - 10, (qrcode.size * scale) + 20,
                (qrcode.size * scale) + 20, WHITE);

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

  // WiFi:S:<SSID>;T:WPA;P:<PASS>;;
  String qrData = "WIFI:S:" + ssid + ";T:WPA;P:" + pass + ";;";

  // QR Version 3 is 29x29. Scale x6 = 174px.
  // SCREEN_WIDTH is 240. (240 - 174) / 2 = 33px margin.
  int qrScale = 6;
  int qrSize = 29 * qrScale;
  int qrX = (SCREEN_WIDTH - qrSize) / 2;
  int qrY = 40; // Positioned for balance

  drawQRCode(qrData, qrX, qrY, qrScale);

  // Footer text - Using 9pt to fit within ~180px (QR width)
  String url = "artem.landcruiser.local";
  gfx->setFont(&FreeSans9pt7b);
  gfx->setTextSize(1);

  int16_t tx1, ty1;
  uint16_t tw, th;
  gfx->getTextBounds(url.c_str(), 0, 0, &tx1, &ty1, &tw, &th);

  // Centering text horizontally
  int txPos = (SCREEN_WIDTH - tw) / 2;

  // Vertical position in the area below QR
  // Area starts from QR end (40 + 174 = 214) + 15px padding (229) to 320.
  int tyPos = 229 + (320 - 229 + th) / 2;

  gfx->setTextColor(CYAN);
  gfx->setCursor(txPos, tyPos);
  gfx->print(url);

  gfx->setFont(NULL); // Reset font
}
