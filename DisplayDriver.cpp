#include "DisplayDriver.h"
#include "qrcode.h"

// Шрифты Adafruit GFX для красивого отображения (используем локальные копии)
#include "FreeSansBold24pt7b.h"
#include "FreeSansBold18pt7b.h"  // Для масштабирования x2 (~72px)
#include "WeatherIcons.h"        // Наши новые иконки погоды

// Предыдущие значения для предотвращения мерцания
static int prevHour = -1;
static int prevMinute = -1;
static float prevTemp = -999;
static String prevCondition = "";

DisplayDriver::DisplayDriver() {
    // Initialize ILI9341 on the specified pins
    // Hardware SPI used for improved performance
    bus = new Arduino_HWSPI(PIN_LCD_DC, PIN_LCD_CS, PIN_LCD_SCK, PIN_LCD_MOSI);
    
    // ILI9341 240x320
    gfx = new Arduino_ILI9341(bus, PIN_LCD_RST, 0 /* rotation */, false /* IPS */);
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
    if (hour == prevHour && minute == prevMinute) return;
    prevHour = hour;
    prevMinute = minute;
    
    // Очищаем ВЕРХНЮЮ половину экрана с запасом
    gfx->fillRect(0, 0, SCREEN_WIDTH, 175, BLACK);
    
    // Используем САМЫЙ КРУПНЫЙ шрифт
    gfx->setFont(&FreeSansBold24pt7b);
    gfx->setTextSize(2); 
    
    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", hour, minute);
    
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
    // Detect night mode from OpenWeatherMap icon code (suffixes like '01n', '02n', etc.)
    bool isNight = icon.endsWith("n");
    
    if (abs(temp - prevTemp) < 0.1 && condition == prevCondition) return;
    prevTemp = temp;
    prevCondition = condition;
    
    // Очищаем НИЖНЮЮ зону
    gfx->fillRect(0, 175, SCREEN_WIDTH, 145, BLACK);
    
    // Декоративная линия-разделитель (тонкая, глянцевая)
    gfx->drawLine(20, 180, SCREEN_WIDTH - 20, 180, CLR_DGRAY);
    
    // 1. ПРЕМИУМ ИКОНКА (Центрируем по вертикали в нижней части)
    int iconX = 10;
    int iconY = 195;
    
    if (condition == "Clear") {
        if (isNight) drawMoonIconPremium(gfx, iconX, iconY);
        else drawSunIconPremium(gfx, iconX, iconY);
    }
    else if (condition == "Clouds") drawCloudIconPremium(gfx, iconX, iconY);
    else if (condition == "Rain" || condition == "Drizzle") drawRainIconPremium(gfx, iconX, iconY);
    else if (condition == "Snow") drawSnowIconPremium(gfx, iconX, iconY);
    else drawCloudIconPremium(gfx, iconX, iconY);
    
    // 2. ТЕМПЕРАТУРА (Современный стиль с подложкой)
    gfx->setFont(&FreeSansBold24pt7b);
    gfx->setTextSize(1);
    
    char tempStr[10];
    sprintf(tempStr, "%.1f", temp);
    
    int16_t tx1, ty1;
    uint16_t tw, th;
    gfx->getTextBounds(tempStr, 0, 0, &tx1, &ty1, &tw, &th);
    
    int txPos = SCREEN_WIDTH - tw - 45; // Оставляем место для символа градуса
    int tyPos = 265;
    
    // Тень для температуры
    gfx->setTextColor(CLR_DEEP_BLUE);
    gfx->setCursor(txPos + 2, tyPos + 2);
    gfx->print(tempStr);
    
    // Основной цвет температуры
    gfx->setTextColor(CYAN);
    gfx->setCursor(txPos, tyPos);
    gfx->print(tempStr);
    
    // Символ градуса (кастомный рисунок для красоты)
    int degX = txPos + tw + 5;
    int degY = tyPos - th + 5;
    gfx->drawCircle(degX + 8, degY + 8, 5, CYAN);
    gfx->drawCircle(degX + 8, degY + 8, 4, CYAN);
    
    gfx->setFont(NULL);
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
    gfx->fillRect(startX - 5, startY - 5, (qrcode.size * scale) + 10, (qrcode.size * scale) + 10, WHITE);

    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                gfx->fillRect(startX + x * scale, startY + y * scale, scale, scale, BLACK);
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
    
    // QR Code - URL to IP
    // Note: User must be connected manually first!
    String qrData = "http://" + ip;
    drawQRCode(qrData);
    
    // Text Info under QR
    int textY = 230;
    gfx->setTextColor(WHITE, BLACK);
    gfx->setTextSize(1);
    
    gfx->setCursor(10, textY);
    gfx->print("1. Connect WiFi:"); 
    
    gfx->setCursor(10, textY + 15);
    gfx->setTextColor(YELLOW, BLACK);
    gfx->print("SSID: "); gfx->print(ssid);
    
    gfx->setCursor(10, textY + 30);
    gfx->print("Pass: "); gfx->print(pass);
    
    gfx->setCursor(10, textY + 50);
    gfx->setTextColor(GREEN, BLACK);
    gfx->print("2. Go to URL:");
    gfx->setCursor(10, textY + 65);
    gfx->print("artemLandCruiser.local");
}
