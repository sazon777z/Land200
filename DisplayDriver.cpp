#include "DisplayDriver.h"
#include "FreeSans12pt7b.h"
#include "FreeSans9pt7b.h"
#include "FreeSansBold18pt7b.h"
#include "Htc_Clock_Data.h"
#include "RusTinyFont.h"
#include "qrcode.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <math.h>

namespace {
constexpr int kDisplayWidth = 320;
constexpr int kDisplayHeight = 172;

constexpr int kTimeCanvasX = 8;
constexpr int kTimeCanvasY = 18;
constexpr int kTimeCanvasW = 214;
constexpr int kTimeCanvasH = 144; // Увеличен для вмещения даты снизу

constexpr int kWeatherPanelX = 225;
constexpr int kWeatherPanelY = 20;
constexpr int kWeatherPanelW = 80;
constexpr int kWeatherPanelH = 142;
constexpr uint16_t kColorWeatherBg = 0x1904; // Темно-серый из референса

constexpr int kQrDisplayHeight = 172;

constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return ((static_cast<uint16_t>(r) & 0xF8) << 8) |
         ((static_cast<uint16_t>(g) & 0xFC) << 3) |
         (static_cast<uint16_t>(b) >> 3);
}

constexpr uint16_t kColorBg = rgb565(3, 5, 9);
constexpr uint16_t kColorGlass = rgb565(10, 14, 20);
constexpr uint16_t kColorGlassSoft = rgb565(15, 21, 30);
constexpr uint16_t kColorFrame = rgb565(43, 54, 72);
constexpr uint16_t kColorDivider = rgb565(27, 36, 50);
constexpr uint16_t kColorText = rgb565(242, 247, 252);
constexpr uint16_t kColorMuted = rgb565(166, 176, 191);
constexpr uint16_t kColorSky = rgb565(128, 205, 255);
constexpr uint16_t kColorSkySoft = rgb565(67, 118, 164);
constexpr uint16_t kColorSun = rgb565(255, 208, 94);
constexpr uint16_t kColorMoon = rgb565(218, 228, 238);
constexpr uint16_t kColorRain = rgb565(116, 196, 255);
constexpr uint16_t kColorSnow = rgb565(235, 244, 255);
constexpr uint16_t kColorAlert = rgb565(255, 196, 72);
constexpr uint16_t kColorAlarmOff = rgb565(109, 120, 136);
constexpr uint16_t kColorTimeShadow = rgb565(28, 32, 40);

enum class WeatherKind { Clear, Clouds, Rain, Thunder, Snow, Mist, Unknown };

static int prevHour = -1;
static int prevMinute = -1;
static int prevSecond = -1;
static int prevWeekday = -1;
static int prevDayOfMonth = -1;
static int prevMonth = -1;
static float prevTemp = -999.0f;
static bool prevAlarmEnabled = false;
static String prevCondition;
static String prevIcon;
static bool dashboardPrimed = false;
static bool colonBright = true;

WeatherKind detectWeatherKind(const String &iconCode) {
  if (iconCode.length() < 2) {
    return WeatherKind::Unknown;
  }

  const String prefix = iconCode.substring(0, 2);
  if (prefix == "01") {
    return WeatherKind::Clear;
  }
  if (prefix == "02" || prefix == "03" || prefix == "04") {
    return WeatherKind::Clouds;
  }
  if (prefix == "09" || prefix == "10") {
    return WeatherKind::Rain;
  }
  if (prefix == "11") {
    return WeatherKind::Thunder;
  }
  if (prefix == "13") {
    return WeatherKind::Snow;
  }
  if (prefix == "50") {
    return WeatherKind::Mist;
  }
  return WeatherKind::Unknown;
}

bool isDayWeather(const String &iconCode) { return iconCode.endsWith("d"); }

void drawCloudShape(Arduino_GFX *target, int x, int y, uint16_t color) {
  target->fillCircle(x + 14, y + 18, 9, color);
  target->fillCircle(x + 26, y + 14, 12, color);
  target->fillCircle(x + 40, y + 18, 9, color);
  target->fillRoundRect(x + 10, y + 19, 34, 12, 6, color);
}

void drawSunShape(Arduino_GFX *target, int cx, int cy, int radius) {
  target->fillCircle(cx, cy, radius, kColorSun);
  target->drawFastVLine(cx, cy - radius - 7, 5, kColorSun);
  target->drawFastVLine(cx, cy + radius + 2, 5, kColorSun);
  target->drawFastHLine(cx - radius - 7, cy, 5, kColorSun);
  target->drawFastHLine(cx + radius + 2, cy, 5, kColorSun);
  target->drawLine(cx - radius - 5, cy - radius - 5, cx - radius - 2,
                   cy - radius - 2, kColorSun);
  target->drawLine(cx + radius + 2, cy - radius - 2, cx + radius + 5,
                   cy - radius - 5, kColorSun);
  target->drawLine(cx - radius - 5, cy + radius + 5, cx - radius - 2,
                   cy + radius + 2, kColorSun);
  target->drawLine(cx + radius + 2, cy + radius + 2, cx + radius + 5,
                   cy + radius + 5, kColorSun);
}

void drawMoonShape(Arduino_GFX *target, int cx, int cy, int radius) {
  target->fillCircle(cx, cy, radius, kColorMoon);
  target->fillCircle(cx + 5, cy - 2, radius, kColorBg);
}

void drawWeatherGlyph(Arduino_GFX *target, int x, int y,
                      const String &iconCode) {
  const WeatherKind kind = detectWeatherKind(iconCode);
  const bool day = isDayWeather(iconCode);

  switch (kind) {
  case WeatherKind::Clear:
    if (day) {
      drawSunShape(target, x + 28, y + 24, 10);
    } else {
      drawMoonShape(target, x + 28, y + 24, 10);
    }
    break;
  case WeatherKind::Clouds:
    if (day) {
      drawSunShape(target, x + 18, y + 18, 7);
    } else {
      drawMoonShape(target, x + 18, y + 18, 7);
    }
    drawCloudShape(target, x + 4, y + 14, kColorText);
    break;
  case WeatherKind::Rain:
    drawCloudShape(target, x + 4, y + 12, kColorText);
    target->drawLine(x + 18, y + 42, x + 15, y + 49, kColorRain);
    target->drawLine(x + 28, y + 42, x + 25, y + 49, kColorRain);
    target->drawLine(x + 38, y + 42, x + 35, y + 49, kColorRain);
    break;
  case WeatherKind::Thunder:
    drawCloudShape(target, x + 4, y + 12, kColorText);
    target->fillTriangle(x + 24, y + 38, x + 34, y + 38, x + 22, y + 54,
                         kColorSun);
    target->fillTriangle(x + 30, y + 46, x + 40, y + 46, x + 28, y + 62,
                         kColorSun);
    break;
  case WeatherKind::Snow:
    drawCloudShape(target, x + 4, y + 12, kColorText);
    target->drawFastVLine(x + 20, y + 42, 8, kColorSnow);
    target->drawFastHLine(x + 16, y + 46, 8, kColorSnow);
    target->drawFastVLine(x + 34, y + 42, 8, kColorSnow);
    target->drawFastHLine(x + 30, y + 46, 8, kColorSnow);
    break;
  case WeatherKind::Mist:
    target->drawFastHLine(x + 10, y + 18, 36, kColorText);
    target->drawFastHLine(x + 6, y + 28, 44, kColorMuted);
    target->drawFastHLine(x + 12, y + 38, 32, kColorText);
    break;
  case WeatherKind::Unknown:
  default:
    drawCloudShape(target, x + 4, y + 14, kColorText);
    break;
  }
}
} // namespace

DisplayDriver::DisplayDriver() {
  bus = new Arduino_HWSPI(PIN_LCD_DC, PIN_LCD_CS, PIN_LCD_SCK, PIN_LCD_MOSI);
  gfx = new Arduino_ST7789(bus, PIN_LCD_RST, 3 /* rotation */, true /* IPS */,
                           172, 320, 34, 0);

  clockCanvas = new Arduino_Canvas(kTimeCanvasW, kTimeCanvasH, gfx,
                                   kTimeCanvasX, kTimeCanvasY);
  infoCanvas = new Arduino_Canvas(kWeatherPanelW, kWeatherPanelH, gfx,
                                  kWeatherPanelX, kWeatherPanelY);
}

void DisplayDriver::begin() {
  gfx->begin();
  gfx->fillScreen(kColorBg);

#ifdef PIN_LCD_BL
  pinMode(PIN_LCD_BL, OUTPUT);
  ledcAttach(PIN_LCD_BL, 5000, 8);
  ledcWrite(PIN_LCD_BL, 255);
#endif

  clockCanvas->begin();
  clockCanvas->fillScreen(kColorBg);

  infoCanvas->begin();
  infoCanvas->fillScreen(kColorBg);

  drawBackground();
}

void DisplayDriver::setBrightness(uint8_t brightness) {
  currentBrightness = brightness;
#ifdef PIN_LCD_BL
  ledcWrite(PIN_LCD_BL, brightness);
#endif
}

void DisplayDriver::drawBackground() { gfx->fillScreen(kColorBg); }

void DisplayDriver::clearMainSegments() {
  prevHour = -1;
  prevMinute = -1;
  prevSecond = -1;
  prevWeekday = -1;
  prevDayOfMonth = -1;
  prevMonth = -1;
  prevTemp = -999.0f;
  prevAlarmEnabled = false;
  prevCondition = "";
  prevIcon = "";
  dashboardPrimed = false;
  colonBright = true;

  if (clockCanvas != nullptr) {
    clockCanvas->fillScreen(kColorBg);
  }
  if (infoCanvas != nullptr) {
    infoCanvas->fillScreen(kColorBg);
  }

  drawBackground();
}

void DisplayDriver::drawDashboard(const WatchStateSnapshot &snapshot) {
  const bool infoChanged = !dashboardPrimed ||
                           fabsf(snapshot.temperature - prevTemp) >= 0.1f ||
                           snapshot.weekdayIndex != prevWeekday ||
                           snapshot.dayOfMonth != prevDayOfMonth ||
                           snapshot.monthIndex != prevMonth ||
                           snapshot.alarmEnabled != prevAlarmEnabled ||
                           snapshot.weatherCondition != prevCondition ||
                           snapshot.weatherIcon != prevIcon;

  const bool timeChanged = !dashboardPrimed || snapshot.hour != prevHour ||
                           snapshot.minute != prevMinute ||
                           snapshot.second != prevSecond;

  if (!infoChanged && !timeChanged) {
    return;
  }

  if (!dashboardPrimed) {
    colonBright = true;
  } else if (timeChanged) {
    colonBright = !colonBright;
  }

  if (infoChanged) {
    // drawBackground() удален для устранения мерцания
    drawWeatherPanel(snapshot);
  }

  if (infoChanged || timeChanged) {
    drawTimePanel(snapshot, colonBright);
  }

  prevHour = snapshot.hour;
  prevMinute = snapshot.minute;
  prevSecond = snapshot.second;
  prevWeekday = snapshot.weekdayIndex;
  prevDayOfMonth = snapshot.dayOfMonth;
  prevMonth = snapshot.monthIndex;
  prevTemp = snapshot.temperature;
  prevAlarmEnabled = snapshot.alarmEnabled;
  prevCondition = snapshot.weatherCondition;
  prevIcon = snapshot.weatherIcon;
  dashboardPrimed = true;
}

void DisplayDriver::drawTimePanel(const WatchStateSnapshot &snapshot,
                                  bool showColon) {
  // Для часов HTC Sense нам не нужно мигающее двоеточие, оно просто съедает
  // место.
  clockCanvas->fillScreen(kColorBg);

  // Отрисовка левой карточки (ЧАСЫ)
  if (snapshot.hour >= 0 && snapshot.hour < 24) {
    clockCanvas->draw16bitRGBBitmap(4, 4,
                                    (uint16_t *)digits_left[snapshot.hour / 10],
                                    CLOCK_HALF_W, CLOCK_H);
    clockCanvas->draw16bitRGBBitmap(
        4 + CLOCK_HALF_W, 4, (uint16_t *)digits_right[snapshot.hour % 10],
        CLOCK_HALF_W, CLOCK_H);
  }

  // Отрисовка правой карточки (МИНУТЫ)
  if (snapshot.minute >= 0 && snapshot.minute < 60) {
    int startMinX = 4 + CLOCK_HALF_W * 2 + 10;
    clockCanvas->draw16bitRGBBitmap(
        startMinX, 4, (uint16_t *)digits_left[snapshot.minute / 10],
        CLOCK_HALF_W, CLOCK_H);
    clockCanvas->draw16bitRGBBitmap(
        startMinX + CLOCK_HALF_W, 4,
        (uint16_t *)digits_right[snapshot.minute % 10], CLOCK_HALF_W, CLOCK_H);
  }

  // Отрисовка даты снизу (день недели, число и месяц) - Укрупненный шрифт
  const String dayLabel = snapshot.dayOfMonth < 10
                              ? "0" + String(snapshot.dayOfMonth)
                              : String(snapshot.dayOfMonth);
  String dateLine = getWeekdayLabel(snapshot.weekdayIndex) + ", " + dayLabel +
                    " " + getMonthLabel(snapshot.monthIndex);

  int dateWidth = RusTinyFont::measureText(dateLine, 2);
  int dateScale = 2;
  int dateX = (kTimeCanvasW - dateWidth) / 2;
  RusTinyFont::drawText(clockCanvas, dateX, CLOCK_H + 12, dateLine, kColorText,
                        dateScale);

  clockCanvas->flush();
}

void DisplayDriver::drawWeatherPanel(const WatchStateSnapshot &snapshot) {
  infoCanvas->fillScreen(kColorBg);

  // 1. Погодная панель (Верхняя, высота 73)
  infoCanvas->fillRoundRect(0, 0, kWeatherPanelW, 73, 8, kColorWeatherBg);

  // 2. Панель будильника (Нижняя, высота 63)
  infoCanvas->fillRoundRect(0, 79, kWeatherPanelW, 63, 8, kColorWeatherBg);

  // --- Контент погодной панели (Температура с красивым знаком) ---
  const String numPart = formatTemperature(snapshot.temperature);
  int16_t x1, y1;
  uint16_t wNum, hNum;
  infoCanvas->setFont(&FreeSans12pt7b);
  infoCanvas->getTextBounds(numPart.c_str(), 0, 0, &x1, &y1, &wNum, &hNum);

  uint16_t wUnit, hUnit;
  infoCanvas->setFont(&FreeSans9pt7b);
  infoCanvas->getTextBounds("C", 0, 0, &x1, &y1, &wUnit, &hUnit);

  // Общая ширина: число + зазор под градус + буква C
  int totalW = wNum + 15 + wUnit;
  int startX = (kWeatherPanelW - totalW) / 2;

  // Рисуем само число (12pt)
  infoCanvas->setFont(&FreeSans12pt7b);
  infoCanvas->setTextColor(kColorText);
  infoCanvas->setCursor(startX - x1, 35);
  infoCanvas->print(numPart);

  // Рисуем символ градуса (Геометрический кружок для красоты)
  int degreeX = startX + wNum + 6; // Увеличен зазор до 6
  int degreeY = 35 - 15;
  infoCanvas->drawCircle(degreeX, degreeY, 2, kColorText);

  // Рисуем C (Уменьшенная до 9pt)
  infoCanvas->setFont(&FreeSans9pt7b);
  infoCanvas->setCursor(startX + wNum + 15 - x1, 35); // Сдвинуто на 15
  infoCanvas->print("C");

  String conditionLabel =
      getConditionLabel(snapshot.weatherCondition, snapshot.weatherIcon);
  int condW = RusTinyFont::measureText(conditionLabel, 1);
  // Размещаем под температурой
  RusTinyFont::drawText(infoCanvas, (kWeatherPanelW - condW) / 2, 54,
                        conditionLabel, kColorText, 1);

  // --- Контент панели будильника ---
  char alarmBuf[8];
  snprintf(alarmBuf, sizeof(alarmBuf), "%02d:%02d", snapshot.alarmHour,
           snapshot.alarmMinute);
  uint16_t alarmColor = snapshot.alarmEnabled ? kColorText : kColorMuted;

  uint16_t w, h; // Добавлена декларация w и h
  infoCanvas->setFont(&FreeSans12pt7b);
  infoCanvas->setTextColor(alarmColor);
  infoCanvas->getTextBounds(alarmBuf, 0, 0, &x1, &y1, &w, &h);
  // Точное вертикальное центрирование в нижней панели (y=79, H=63)
  // middle_y = 79 + 63/2 = 110.5. baseline = middle_y - y1 - h/2.
  int y_baseline = 79 + (63 / 2) - y1 - (h / 2);
  infoCanvas->setCursor((kWeatherPanelW - w) / 2 - x1, y_baseline);
  infoCanvas->print(alarmBuf);

  infoCanvas->setFont(NULL);
  infoCanvas->flush();
}

void DisplayDriver::drawAlarmIcon(Arduino_GFX *target, int x, int y,
                                  bool enabled, uint16_t color) {
  target->drawCircle(x + 10, y + 6, 2, color);
  target->drawCircle(x + 18, y + 6, 2, color);
  target->drawLine(x + 12, y + 8, x + 8, y + 12, color);
  target->drawLine(x + 16, y + 8, x + 20, y + 12, color);
  target->drawRoundRect(x + 7, y + 10, 14, 10, 5, color);
  target->drawFastHLine(x + 9, y + 20, 10, color);
  target->fillCircle(x + 14, y + 22, 1, color);
  target->drawLine(x + 10, y + 21, x + 8, y + 23, color);
  target->drawLine(x + 18, y + 21, x + 20, y + 23, color);

  if (!enabled) {
    target->drawLine(x + 6, y + 23, x + 22, y + 5, color);
  }
}

String DisplayDriver::formatTemperature(float temp) {
  const int rounded = temp >= 0.0f ? static_cast<int>(temp + 0.5f)
                                   : static_cast<int>(temp - 0.5f);
  if (rounded > 0) {
    return "+" + String(rounded);
  }
  return String(rounded);
}

String DisplayDriver::getWeekdayLabel(int weekdayIndex) {
  static const char *const kWeekdays[] = {
      "\xD0\x92\xD0\x9E\xD0\xA1\xD0\x9A\xD0\xA0\xD0\x95\xD0\xA1\xD0\x95\xD0\x9D"
      "\xD0\xAC\xD0\x95", // ВОСКРЕСЕНЬЕ
      "\xD0\x9F\xD0\x9E\xD0\x9D\xD0\x95\xD0\x94\xD0\x95\xD0\x9B\xD0\xAC\xD0\x9D"
      "\xD0\x98\xD0\x9A",                                         // ПОНЕДЕЛЬНИК
      "\xD0\x92\xD0\xA2\xD0\x9E\xD0\xA0\xD0\x9D\xD0\x98\xD0\x9A", // ВТОРНИК
      "\xD0\xA1\xD0\xA0\xD0\x95\xD0\x94\xD0\x90",                 // СРЕДА
      "\xD0\xA7\xD0\x95\xD0\xA2\xD0\x92\xD0\x95\xD0\xA0\xD0\x93", // ЧЕТВЕРГ
      "\xD0\x9F\xD0\xAF\xD0\xA2\xD0\x9D\xD0\x98\xD0\xA6\xD0\x90", // ПЯТНИЦА
      "\xD0\xA1\xD0\xA3\xD0\x91\xD0\x91\xD0\x9E\xD0\xA2\xD0\x90", // СУББОТА
  };

  if (weekdayIndex < 0 || weekdayIndex > 6) {
    weekdayIndex = 3;
  }
  return String(kWeekdays[weekdayIndex]);
}

String DisplayDriver::getMonthLabel(int monthIndex) {
  static const char *const kMonths[] = {
      "\xD0\xAF\xD0\x9D\xD0\x92\xD0\x90\xD0\xA0\xD0\xAF",         // ЯНВАРЯ
      "\xD0\xA4\xD0\x95\xD0\x92\xD0\xA0\xD0\x90\xD0\x9B\xD0\xAF", // ФЕВРАЛЯ
      "\xD0\x9C\xD0\x90\xD0\xA0\xD0\xA2\xD0\x90",                 // МАРТА
      "\xD0\x90\xD0\x9F\xD0\xA0\xD0\x95\xD0\x9B\xD0\xAF",         // АПРЕЛЯ
      "\xD0\x9C\xD0\x90\xD0\xAF",                                 // МАЯ
      "\xD0\x98\xD0\xAE\xD0\x9D\xD0\xAF",                         // ИЮНЯ
      "\xD0\x98\xD0\xAE\xD0\x9B\xD0\xAF",                         // ИЮЛЯ
      "\xD0\x90\xD0\x92\xD0\x93\xD0\xA3\xD0\xA1\xD0\xA2\xD0\x90", // АВГУСТА
      "\xD0\xA1\xD0\x95\xD0\x9D\xD0\xA2\xD0\xAF\xD0\x91\xD0\xA0\xD0\xAF", // СЕНТЯБРЯ
      "\xD0\x9E\xD0\x9A\xD0\xA2\xD0\xAF\xD0\x91\xD0\xA0\xD0\xAF", // ОКТЯБРЯ
      "\xD0\x9D\xD0\x9E\xD0\xAF\xD0\x91\xD0\xA0\xD0\xAF",         // НОЯБРЯ
      "\xD0\x94\xD0\x95\xD0\x9A\xD0\x90\xD0\x91\xD0\xA0\xD0\xAF", // ДЕКАБРЯ
  };

  if (monthIndex < 1 || monthIndex > 12) {
    monthIndex = 1;
  }
  return String(kMonths[monthIndex - 1]);
}

String DisplayDriver::getConditionLabel(const String &condition,
                                        const String &iconCode) {
  const WeatherKind kind = detectWeatherKind(iconCode);

  switch (kind) {
  case WeatherKind::Clear:
    return "\xD0\xAF\xD0\xA1\xD0\x9D\xD0\x9E"; // ЯСНО
  case WeatherKind::Clouds:
    return "\xD0\x9E\xD0\x91\xD0\x9B\xD0\x90\xD0\xA7\xD0\x9D\xD0\x9E"; // ОБЛАЧНО
  case WeatherKind::Rain:
    return "\xD0\x94\xD0\x9E\xD0\x96\xD0\x94\xD0\xAC"; // ДОЖДЬ
  case WeatherKind::Thunder:
    return "\xD0\x93\xD0\xA0\xD0\x9E\xD0\x97\xD0\x90"; // ГРОЗА
  case WeatherKind::Snow:
    return "\xD0\xA1\xD0\x9D\xD0\x95\xD0\x93"; // СНЕГ
  case WeatherKind::Mist:
    return "\xD0\xA2\xD0\xA3\xD0\x9C\xD0\x90\xD0\x9D"; // ТУМАН
  case WeatherKind::Unknown:
  default:
    break;
  }

  if (condition.length() > 0 && condition != "--") {
    return "\xD0\x9F\xD0\x9E\xD0\x93\xD0\x9E\xD0\x94\xD0\x90"; // ПОГОДА
  }

  return "\xD0\x9F\xD0\x9E\xD0\x93\xD0\x9E\xD0\x94\xD0\x90"; // ПОГОДА
}

void DisplayDriver::drawConnecting(String ssid) {
  (void)ssid;
  drawBackground();
  gfx->setTextColor(kColorText);
  gfx->setFont(&FreeSans9pt7b);

  String msg = "Connecting...";
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(msg.c_str(), 0, 0, &x1, &y1, &w, &h);

  gfx->setCursor((kDisplayWidth - w) / 2 - x1, 88);
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
  (void)ip;
  drawBackground();

  String qrData = "WIFI:S:" + ssid + ";T:WPA;P:" + pass + ";;";

  const int qrScale = 4;
  const int qrSize = 29 * qrScale;
  const int qrX = 20;
  const int qrY = (kQrDisplayHeight - qrSize) / 2;

  drawQRCode(qrData, qrX, qrY, qrScale);

  gfx->setFont(&FreeSans9pt7b);
  gfx->setTextSize(1);

  String url = "lc200.local";
  int16_t tx1, ty1;
  uint16_t tw, th;
  gfx->getTextBounds(url.c_str(), 0, 0, &tx1, &ty1, &tw, &th);

  const int remainingWidth = kDisplayWidth - (qrX + qrSize);
  const int tx = (qrX + qrSize) + (remainingWidth - tw) / 2;
  const int centerY = kQrDisplayHeight / 2;
  const int ty = centerY - (th / 2) - ty1;

  gfx->setTextColor(kColorSky);
  gfx->setCursor(tx, ty);
  gfx->print(url);
  gfx->setFont(NULL);
}
