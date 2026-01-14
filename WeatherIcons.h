#ifndef WEATHER_ICONS_H
#define WEATHER_ICONS_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

// Современная палитра для версии «Kawaii»
#define CLR_SUN_MAIN   0xFD00 // Насыщенный желтый
#define CLR_SUN_ORANGE 0xFB00 // Оранжевый контур/тень
#define CLR_MOON_MAIN  0xFFE0 // Светло-желтый
#define CLR_CLOUD_MAIN 0xADFF // Голубой (тело облака)
#define CLR_CLOUD_DARK 0x5ADF // Глубокий голубой (низ облака)
#define CLR_WHITE      0xFFFF // Белый блик
#define CLR_OUTLINE    0x0000 // Черный контур
#define CLR_SNOW_FLAKE 0xDEFB // Светло-голубой для снега
#define CLR_DGRAY      0x8410 // Серый (тень)
#define CLR_DEEP_BLUE  0x0019 // Глубокий синий

// Вспомогательная функция для рисования глаз и улыбки
void drawFace(Arduino_GFX *gfx, int cx, int cy) {
    // Глаза
    gfx->fillCircle(cx - 6, cy - 2, 2, CLR_OUTLINE);
    gfx->fillCircle(cx + 6, cy - 2, 2, CLR_OUTLINE);
    // Улыбка
    gfx->drawCircle(cx, cy + 2, 4, CLR_OUTLINE);
    gfx->fillRect(cx - 5, cy, 10, 5, CLR_SUN_MAIN); // Скрываем верхнюю часть круга для улыбки дугой
}

// Рисует СОЛНЫШКО с улыбкой
void drawSunIconPremium(Arduino_GFX *gfx, int x, int y) {
    int cx = x + 40;
    int cy = y + 45;
    
    // 1. Лучи (более толстые)
    for(int i=0; i<360; i+=45) {
        float rad = i * 3.14159 / 180;
        int x1 = cx + cos(rad) * 20;
        int y1 = cy + sin(rad) * 20;
        int x2 = cx + cos(rad) * 35;
        int y2 = cy + sin(rad) * 35;
        gfx->drawLine(x1, y1, x2, y2, CLR_SUN_ORANGE);
        gfx->drawLine(x1+1, y1+1, x2+1, y2+1, CLR_OUTLINE);
    }
    
    // 2. Тело солнца с контуром
    gfx->fillCircle(cx, cy, 21, CLR_OUTLINE);
    gfx->fillCircle(cx, cy, 20, CLR_SUN_MAIN);
    
    // 3. Блик
    gfx->fillCircle(cx - 8, cy - 8, 5, CLR_WHITE);
    
    // 4. Личико
    drawFace(gfx, cx, cy);
}

// Рисует МЕСЯЦ со звездами
void drawMoonIconPremium(Arduino_GFX *gfx, int x, int y) {
    int cx = x + 40;
    int cy = y + 45;
    
    // 1. Звездочки вокруг (в стиле картинки)
    gfx->fillCircle(cx + 15, cy - 20, 3, CLR_MOON_MAIN);
    gfx->fillCircle(cx + 25, cy - 10, 2, CLR_MOON_MAIN);
    
    // 2. Тело луны (полумесяц)
    gfx->fillCircle(cx, cy, 23, CLR_OUTLINE);
    gfx->fillCircle(cx, cy, 22, CLR_MOON_MAIN);
    gfx->fillCircle(cx - 8, cy - 4, 22, BLACK); // Вырезаем форму
    
    // 3. Личико на полумесяце
    gfx->fillCircle(cx + 8, cy - 2, 2, CLR_OUTLINE); // Один глаз (профиль)
    gfx->drawCircle(cx + 12, cy + 4, 3, CLR_OUTLINE); // Улыбка
    gfx->fillRect(cx + 8, cy + 2, 8, 4, CLR_MOON_MAIN);
}

// Рисует Объемное ОБЛАКО (стилизованное)
void drawCloudIconPremium(Arduino_GFX *gfx, int x, int y) {
    int ox = x + 15;
    int oy = y + 20;
    
    // 1. Контур облака (составной из кругов)
    gfx->fillCircle(ox + 10, oy + 30, 17, CLR_OUTLINE);
    gfx->fillCircle(ox + 30, oy + 20, 24, CLR_OUTLINE);
    gfx->fillCircle(ox + 55, oy + 30, 20, CLR_OUTLINE);
    
    // 2. Тело облака
    gfx->fillCircle(ox + 10, oy + 30, 15, CLR_CLOUD_MAIN);
    gfx->fillCircle(ox + 30, oy + 20, 22, CLR_WHITE);
    gfx->fillCircle(ox + 55, oy + 30, 18, CLR_CLOUD_DARK);
    
    // Горизонтальный наполнитель
    gfx->fillRect(ox + 10, oy + 25, 45, 20, CLR_CLOUD_MAIN);
    
    // 3. Блик вверху
    gfx->fillCircle(ox + 30, oy + 15, 12, CLR_WHITE);
}

// Рисует ДОЖДЬ
void drawRainIconPremium(Arduino_GFX *gfx, int x, int y) {
    drawCloudIconPremium(gfx, x, y - 5);
    
    // Капли (крупные и голубые)
    int drops[4][2] = {{30,65}, {45,75}, {60,65}, {45,85}};
    for(int i=0; i<4; i++) {
        int dx = x + drops[i][0];
        int dy = y + drops[i][1];
        gfx->fillCircle(dx, dy, 4, CLR_OUTLINE);
        gfx->fillCircle(dx, dy, 3, CLR_CLOUD_DARK);
        gfx->fillCircle(dx - 1, dy - 1, 1, CLR_WHITE); // Мини-блик
    }
}

// Рисует СНЕГ
void drawSnowIconPremium(Arduino_GFX *gfx, int x, int y) {
    drawCloudIconPremium(gfx, x, y - 5);
    
    int flakes[4][2] = {{30,65}, {45,75}, {60,65}, {45,85}};
    for(int i=0; i<4; i++) {
        int sx = x + flakes[i][0];
        int sy = y + flakes[i][1];
        gfx->fillCircle(sx, sy, 4, CLR_OUTLINE);
        gfx->fillCircle(sx, sy, 3, CLR_WHITE);
        // Рисуем простую снежинку (крестик)
        gfx->drawLine(sx-2, sy, sx+2, sy, CLR_SNOW_FLAKE);
        gfx->drawLine(sx, sy-2, sx, sy+2, CLR_SNOW_FLAKE);
    }
}

#endif
