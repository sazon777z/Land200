#ifndef WEATHER_ICONS_BITMAP_H
#define WEATHER_ICONS_BITMAP_H

#include "WeatherIcons_Data.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>

// Константы размеров
#define ICON_WIDTH 100
#define ICON_HEIGHT 100
#define FULL_IMAGE_WIDTH 600

// Индексы иконок в массиве
#define IC_CLEAR_DAY 0
#define IC_CLEAR_NIGHT 1
#define IC_CLOUDY 2
#define IC_FOG 3
#define IC_RAIN 4
#define IC_THUNDER 5

// Общие цвета для интерфейса
#define CLR_DGRAY 0x8410     // Серый (тень)
#define CLR_DEEP_BLUE 0x0019 // Глубокий синий

// Функция отрисовки фрагмента из общего массива 600x100
void drawWeatherIconChunk(Arduino_GFX *gfx, int16_t x, int16_t y,
                          int iconIndex) {
  if (iconIndex < 0 || iconIndex > 5)
    return;

  for (int16_t row = 0; row < ICON_HEIGHT; row++) {
    // Вычисляем смещение в плоском массиве: (ряд * полная_ширина) +
    // (индекс_иконки * ширина_иконки)
    uint32_t offset =
        (uint32_t)row * FULL_IMAGE_WIDTH + (iconIndex * ICON_WIDTH);

    // Рисуем одну строку (100 пикселей)
    gfx->draw16bitRGBBitmap(
        x, y + row, (uint16_t *)weather_icons_array + offset, ICON_WIDTH, 1);
  }
}

// Note: drawSunVolumetric and other compatibility stubs were removed.
// DisplayDriver now uses drawWeatherIconChunk directly or via updated logic.

#endif
