#ifndef WEATHER_ICONS_H
#define WEATHER_ICONS_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

// Премиум палитра для Версии 6
#define CLR_SUN_BRIGHT 0xFFE0 // Ярко-желтый
#define CLR_SUN_GLOSS  0xFFFF // Белый блик
#define CLR_SUN_SHADOW 0xD620 // Темно-оранжевый
#define CLR_SKY_BLUE   0x5ADF // Небесно-голубой
#define CLR_DEEP_BLUE  0x0019 // Глубокий синий
#define CLR_WHITE      0xFFFF // Чистый белый
#define CLR_LGRAY      0xDEFB // Светло-серый
#define CLR_DGRAY      0x8410 // Серый (тень)
#define CLR_ICE        0x9FFF // Лед

// Рисует глянцевое СОЛНЦЕ (Версия 6)
void drawSunIconPremium(Arduino_GFX *gfx, int x, int y) {
    int cx = x + 35;
    int cy = y + 35;
    
    // 1. Свечение (большая размытая область)
    gfx->drawCircle(cx, cy, 32, CLR_SUN_SHADOW);
    
    // 2. Лучи (треугольные или линии с разной толщиной)
    for(int i=0; i<360; i+=45) {
        float rad = i * 3.14159 / 180;
        int x1 = cx + cos(rad) * 20;
        int y1 = cy + sin(rad) * 20;
        int x2 = cx + cos(rad) * 35;
        int y2 = cy + sin(rad) * 35;
        gfx->drawLine(x1, y1, x2, y2, CLR_SUN_BRIGHT);
        gfx->drawLine(x1+1, y1, x2+1, y2, CLR_SUN_SHADOW); // Двойной луч для объема
    }
    
    // 3. Тело солнца
    gfx->fillCircle(cx, cy, 22, CLR_SUN_SHADOW);
    gfx->fillCircle(cx, cy, 20, CLR_SUN_BRIGHT);
    
    // 4. Глянцевый блик
    gfx->fillCircle(cx - 8, cy - 8, 7, CLR_SUN_GLOSS);
    gfx->fillCircle(cx - 8, cy - 8, 5, CLR_SUN_BRIGHT);
}

// Рисует Объемное ОБЛАКО (Версия 6)
void drawCloudIconPremium(Arduino_GFX *gfx, int x, int y) {
    int ox = x + 10;
    int oy = y + 10;
    
    // 1. Нижняя тень (самый темный слой)
    gfx->fillCircle(ox + 20, oy + 40, 15, CLR_DGRAY);
    gfx->fillCircle(ox + 40, oy + 45, 18, CLR_DGRAY);
    
    // 2. Основное тело (средний серый)
    gfx->fillCircle(ox + 15, oy + 35, 15, CLR_LGRAY);
    gfx->fillCircle(ox + 35, oy + 25, 20, CLR_LGRAY);
    gfx->fillCircle(ox + 55, oy + 35, 16, CLR_LGRAY);
    gfx->fillRect(ox + 15, oy + 35, 40, 15, CLR_LGRAY);
    
    // 3. Верхний глянцевый слой (белый)
    gfx->fillCircle(ox + 35, oy + 22, 16, CLR_WHITE);
    gfx->fillCircle(ox + 18, oy + 32, 12, CLR_WHITE);
    
    // 4. Блик на макушке
    gfx->fillCircle(ox + 30, oy + 18, 6, CLR_WHITE);
}

// Рисует ДОЖДЬ с каплями (Версия 6)
void drawRainIconPremium(Arduino_GFX *gfx, int x, int y) {
    drawCloudIconPremium(gfx, x, y - 5);
    
    // Капли со светом и тенью
    int drops[4][2] = {{25,55}, {40,65}, {55,55}, {45,75}};
    for(int i=0; i<4; i++) {
        int dx = x + drops[i][0];
        int dy = y + drops[i][1];
        gfx->fillCircle(dx, dy, 3, CLR_SKY_BLUE);
        gfx->fillCircle(dx - 1, dy - 1, 1, CLR_WHITE); // Блик на капле
        gfx->drawLine(dx, dy, dx - 2, dy + 8, CLR_SKY_BLUE);
    }
}

// Рисует СНЕГ (Версия 6)
void drawSnowIconPremium(Arduino_GFX *gfx, int x, int y) {
    drawCloudIconPremium(gfx, x, y - 5);
    
    // Снежинки крестиками с центром
    int flakes[4][2] = {{25,55}, {40,65}, {58,55}, {35,75}};
    for(int i=0; i<4; i++) {
        int sx = x + flakes[i][0];
        int sy = y + flakes[i][1];
        gfx->drawLine(sx-4, sy, sx+4, sy, CLR_ICE);
        gfx->drawLine(sx, sy-4, sx, sy+4, CLR_ICE);
        gfx->drawPixel(sx, sy, CLR_WHITE);
    }
}

#endif
