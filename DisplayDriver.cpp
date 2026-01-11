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
    // Initialize ST7789 on the specified pins
    // Hardware SPI used for improved performance
    // Parameters: bus, rotation, ips, width, height, col_offset1, row_offset1, col_offset2, row_offset2
    bus = new Arduino_HWSPI(PIN_LCD_DC, PIN_LCD_CS, PIN_LCD_SCK, PIN_LCD_MOSI);
    
    // ST7789 240x240
    gfx = new Arduino_ST7789(bus, PIN_LCD_RST, 0 /* rotation */, true /* IPS */);
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
    
    // Очищаем ВЕРХНЮЮ половину экрана
    gfx->fillRect(0, 0, SCREEN_WIDTH, 140, BLACK);
    
    // Уменьшаем шрифт с 24pt на 18pt для более аккуратного вида
    gfx->setFont(&FreeSansBold18pt7b);
    gfx->setTextSize(2); 
    gfx->setTextColor(WHITE);
    
    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", hour, minute);
    
    // Точный расчет границ для центрирования
    int16_t x1, y1;
    uint16_t w, h;
    gfx->getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
    
    // Центрируем с небольшим отступом от верха
    int xPos = (SCREEN_WIDTH - w) / 2;
    int yPos = 95; 
    
    gfx->setCursor(xPos, yPos);
    gfx->print(timeStr);
    
    gfx->setFont(NULL); // Сброс
    gfx->setTextSize(1);
}

void DisplayDriver::drawWeather(float temp, String condition, String icon) {
    if (abs(temp - prevTemp) < 0.1 && condition == prevCondition) return;
    prevTemp = temp;
    prevCondition = condition;
    
    // Очищаем НИЖНЮЮ зону (размер 140-240)
    gfx->fillRect(0, 140, SCREEN_WIDTH, 100, BLACK);
    
    // 1. ПРЕМИУМ ИКОНКА (Левый нижний угол)
    int iconX = 5;
    int iconY = 155;
    
    if (condition == "Clear") drawSunIconPremium(gfx, iconX, iconY);
    else if (condition == "Clouds") drawCloudIconPremium(gfx, iconX, iconY);
    else if (condition == "Rain" || condition == "Drizzle") drawRainIconPremium(gfx, iconX, iconY);
    else if (condition == "Snow") drawSnowIconPremium(gfx, iconX, iconY);
    else drawCloudIconPremium(gfx, iconX, iconY);
    
    // 2. ГИГАНТСКАЯ ТЕМПЕРАТУРА (Правый нижний угол)
    gfx->setFont(&FreeSansBold24pt7b); // Используем тот же крупный шрифт
    gfx->setTextColor(CYAN);
    gfx->setTextSize(1); // 24pt уже очень крупный
    
    char tempStr[10];
    sprintf(tempStr, "%.0fC", temp); 
    
    int16_t tx1, ty1;
    uint16_t tw, th;
    gfx->getTextBounds(tempStr, 0, 0, &tx1, &ty1, &tw, &th);
    
    int txPos = SCREEN_WIDTH - tw - 10;
    int tyPos = 210; // Подравниваем под иконку
    
    gfx->setCursor(txPos, tyPos);
    gfx->print(tempStr);
    
    gfx->setFont(NULL);
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
    int textY = 170;
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
    gfx->print("2. Scan QR or go to:");
    gfx->setCursor(10, textY + 65);
    gfx->print(ip);
}
