#include "DisplayDriver.h"
#include "qrcode.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>

// Шрифты Adafruit GFX для красивого отображения (используем локальные копии)
#include "Digits_Data.h"
#include "FreeSans9pt7b.h"      // Новый уменьшенный шрифт для AP Info
#include "FreeSansBold18pt7b.h" // Для масштабирования x2 (~72px)
#include "FreeSansBold24pt7b.h"
#include "WeatherIcons_120.h"

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

void DisplayDriver::clearMainSegments() { gfx->fillScreen(BLACK); }

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

  // Центрирование по вертикали для иконки высотой 150 в канвасе 135?
  // Канвас высотой 135 (DisplayDriver.cpp L42: weatherCanvas = new
  // Arduino_Canvas(240, 135, gfx, 0, 180);) Иконка 150x150 не влезает по высоте
  // в 135 пикселей. Нам нужно либо увеличить канвас, либо уменьшить иконку,
  // либо обрезать. Так как ТЗ строго 150x150, предположим, что мы сдвинем
  // канвас или увеличим его. Но экран всего 320x240 (или 240x320?).
  // Инициализация: Arduino_ILI9341(bus, PIN_LCD_RST, 0, false). Rotation 0
  // обычно 240x320 (Portrait). Часы занимают y=40, h=100. (40+100=140). Погода
  // y=180. (320 - 180 = 140 пикселей доступно до низу). Значит, канвас высотой
  // 135 вполне логичен, но 150 туда не влезет полностью. ТЕМ НЕ МЕНЕЕ, если
  // пользователь просит 150x150, мы должны попытаться это вместить. Допустим,
  // мы рисуем начиная с Y=0 в канвасе. Если канвас 135, то 15 пикселей
  // обрежутся. Или пользователь изменит размер канваса. Давайте поменяем размер
  // канваса в конструкторе позже. Пока пишем логику рисования.

  const uint16_t *currentIconPtr = icon_weather_clouds_day; // Default

  if (condition == "Clear") {
    currentIconPtr =
        isNight ? icon_weather_clear_night : icon_weather_clear_day;
  } else if (condition == "Clouds") {
    // OpenWeatherMap: 801-804.
    // 801 (few clouds), 802 (scattered) might be "clouds_day/night"
    // 803, 804 (broken, overcast) might be "overcast"
    // Упростим логику: если просто Clouds, используем overcast или clouds_day
    currentIconPtr =
        isNight ? icon_weather_clouds_night : icon_weather_clouds_day;
  } else if (condition == "Overcast") { // Specific check if available
    currentIconPtr = icon_weather_overcast;
  } else if (condition == "Rain" || condition == "Drizzle" ||
             condition == "Shower Rain") {
    currentIconPtr = icon_weather_rain;
  } else if (condition == "Thunderstorm") {
    currentIconPtr = icon_weather_thunder;
  } else if (condition == "Snow") {
    currentIconPtr = icon_weather_snow;
  } else if (condition == "Mist" || condition == "Fog" || condition == "Haze") {
    currentIconPtr = icon_weather_mist;
  } else {
    currentIconPtr = icon_weather_overcast;
  }

  // Координаты: Иконка слева внизу.
  // В канвасе (0,0) - это левый верхний угол канваса.
  // Иконка 150px шириной.
  // Температура справа.

  // Рисуем иконку
  // Используем draw16bitRGBBitmap так как иконки в PROGMEM
  // x=0, y= -15 (чтобы центрировать по вертикали, если канвас меньше иконки?
  // Или просто 0, пусть обрезается с низу/верху?
  // Канвас 135px. Иконка 150px. Разница 15px.
  // Сдвинем вверх на -7? Нет, Canvas не умеет рисовать в минус.
  // Просто рисуем в 0,0, низ обрежется. Или переопределим канвас.
  weatherCanvas->draw16bitRGBBitmap(0, 7, (uint16_t *)currentIconPtr, 120, 120);

  // Рисуем температуру справа
  weatherCanvas->setFont(&FreeSansBold24pt7b);
  weatherCanvas->setTextSize(1);
  weatherCanvas->setTextColor(WHITE);

  char tempStr[10];
  sprintf(tempStr, "%.0f", temp); // Без дробной части для эстетики или %.1f

  int16_t tx1, ty1;
  uint16_t tw, th;
  weatherCanvas->getTextBounds(tempStr, 0, 0, &tx1, &ty1, &tw, &th);

  // Позиция X: Справа от иконки (120) + отступ (5) + сдвиг пользователя (20)
  int textX = 120 + 5 + 20;

  // Позиция Y: Центр канваса + сдвиг пользователя вверх (-10)
  int textY = (135 / 2) + (th / 2) - 10;

  weatherCanvas->setCursor(textX, textY);
  weatherCanvas->print(tempStr);

  // Символ градуса (белый, жирный, с увеличенным отступом)
  int degreeX = textX + tw + 10; // Отступ от цифр увеличен до 10
  int degreeY = textY - th + 5;  // Позиция по высоте
  weatherCanvas->drawCircle(degreeX, degreeY, 4, WHITE); // Основной круг
  weatherCanvas->drawCircle(degreeX, degreeY, 5,
                            WHITE); // Второй круг для толщины

  weatherCanvas->setFont(NULL);

  // 5. Выводим буфер погоды на экран
  weatherCanvas->flush();
}

void DisplayDriver::drawConnecting(String ssid) {
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE);
  gfx->setFont(&FreeSans9pt7b);

  String msg = "Connecting...";
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(msg.c_str(), 0, 0, &x1, &y1, &w, &h);

  gfx->setCursor((240 - w) / 2, 160);
  gfx->print(msg);

  gfx->setFont(NULL);
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
  String url = "artemlandcruiser.local";
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
