#line 1 "C:\\Users\\JZX\\Documents\\Arduino\\Land_Cruiser\\ClockFont.h"
#ifndef CLOCK_FONT_H
#define CLOCK_FONT_H

#include <Arduino_GFX_Library.h>

class ClockDigits {
public:
  // Параметры цифр
  static const int DEFAULT_HEIGHT = 80;

  // Отрисовка одной цифры
  // x, y - верхний левый угол
  // h - высота цифры (ширина вычисляется автоматически ~0.6 * h)
  static void draw(Arduino_GFX *gfx, int x, int y, int h, uint16_t color,
                   int digit) {
    int w = h * 0.6;  // Ширина
    int t = h * 0.12; // Толщина линий

    switch (digit) {
    case 0:
      draw0(gfx, x, y, w, h, t, color);
      break;
    case 1:
      draw1(gfx, x, y, w, h, t, color);
      break;
    case 2:
      draw2(gfx, x, y, w, h, t, color);
      break;
    case 3:
      draw3(gfx, x, y, w, h, t, color);
      break;
    case 4:
      draw4(gfx, x, y, w, h, t, color);
      break;
    case 5:
      draw5(gfx, x, y, w, h, t, color);
      break;
    case 6:
      draw6(gfx, x, y, w, h, t, color);
      break;
    case 7:
      draw7(gfx, x, y, w, h, t, color);
      break;
    case 8:
      draw8(gfx, x, y, w, h, t, color);
      break;
    case 9:
      draw9(gfx, x, y, w, h, t, color);
      break;
    }
  }

  static void drawColon(Arduino_GFX *gfx, int x, int y, int h, uint16_t color) {
    int s = h * 0.15;          // Размер точек
    int centerX = x + (s / 2); // Условно, хотя x это левый край
    // Точки центрируем по x
    // Верхняя
    gfx->fillCircle(x, y + h / 3, s / 2, color);
    // Нижняя
    gfx->fillCircle(x, y + (h * 2) / 3, s / 2, color);
  }

  static int getWidth(int h) { return h * 0.6; }

  static int getColonWidth(int h) {
    return h * 0.15; // Ширина двоеточия равна диаметру точки
  }

private:
  // Вспомогательные методы для рисования сегментов
  // Стиль: Скругленные линии (Rounded Caps)

  static void draw0(Arduino_GFX *gfx, int x, int y, int w, int h, int t,
                    uint16_t color) {
    // Овал
    gfx->drawRoundRect(x, y, w, h, t, color);
    gfx->drawRoundRect(x + 1, y + 1, w - 2, h - 2, t, color); // Толщина +1

    // Для толщины t рисуем несколько вложенных прямоугольников или
    // fillRoundRect с вычитанием? GFX не умеет вычитать. Рисуем fillRoundRect
    // внешний, потом fillRoundRect внутренний цветом фона? Нет, фон может быть
    // не однородным (хотя в нашем случае BLACK). Лучше рисовать толстыми
    // линиями. Но drawRoundRect толщину 1 имеет.

    // Простой вариат: fillRoundRect всего, затем fillRoundRect внутри черным
    gfx->fillRoundRect(x, y, w, h, w / 2, color);
    gfx->fillRoundRect(x + t, y + t, w - 2 * t, h - 2 * t, (w - 2 * t) / 2,
                       0x0000 /*BLACK*/);
  }

  static void draw1(Arduino_GFX *gfx, int x, int y, int w, int h, int t,
                    uint16_t color) {
    // Палка по центру или спарва? Обычно цифра 1 уже. Но для моноширинности
    // оставим центр.
    int cx = x + w / 2;
    gfx->fillRoundRect(cx - t / 2, y, t, h, t / 2, color);
  }

  static void draw2(Arduino_GFX *gfx, int x, int y, int w, int h, int t,
                    uint16_t color) {
    // Верхняя дуга
    gfx->fillRoundRect(x, y, w, t, t / 2, color); // Top Rect (flat?) Or Round?
    // Right vertical half
    gfx->fillRoundRect(x + w - t, y, t, h / 2, t / 2, color);
    // Middle horizontal
    gfx->fillRoundRect(x, y + h / 2 - t / 2, w, t, t / 2, color);
    // Left vertical bottom
    gfx->fillRoundRect(x, y + h / 2, t, h / 2, t / 2, color);
    // Bottom horizontal
    gfx->fillRoundRect(x, y + h - t, w, t, t / 2, color);

    // Это стиль "8" цифровой.
    // 2 это: Top, Right-Top, Middle, Left-Bottom, Bottom.

    // Более скругленный 2:
    // Рисуем полностью
    int h2 = h / 2;
    // Верх
    gfx->fillRoundRect(x, y, w, t, t / 2, color);
    // Право верх
    gfx->fillRoundRect(x + w - t, y, t, h2, t / 2, color);
    // Середина
    gfx->fillRoundRect(x, y + h2 - t / 2, w, t, t / 2, color);
    // Лево низ
    gfx->fillRoundRect(x, y + h2, t, h2, t / 2, color);
    // Низ
    gfx->fillRoundRect(x, y + h - t, w, t, t / 2, color);
  }

  static void draw3(Arduino_GFX *gfx, int x, int y, int w, int h, int t,
                    uint16_t color) {
    int h2 = h / 2;
    // Top
    gfx->fillRoundRect(x, y, w, t, t / 2, color);
    // Right
    gfx->fillRoundRect(x + w - t, y, t, h, t / 2, color);
    // Middle
    gfx->fillRoundRect(x + w / 4, y + h2 - t / 2, w * 3 / 4, t, t / 2, color);
    // Bottom
    gfx->fillRoundRect(x, y + h - t, w, t, t / 2, color);
  }

  static void draw4(Arduino_GFX *gfx, int x, int y, int w, int h, int t,
                    uint16_t color) {
    int h2 = h / 2;
    // Right full
    gfx->fillRoundRect(x + w - t, y, t, h, t / 2, color);
    // Left top
    gfx->fillRoundRect(x, y, t, h2, t / 2, color);
    // Middle
    gfx->fillRoundRect(x, y + h2 - t / 2, w, t, t / 2, color);
  }

  static void draw5(Arduino_GFX *gfx, int x, int y, int w, int h, int t,
                    uint16_t color) {
    int h2 = h / 2;
    // Top
    gfx->fillRoundRect(x, y, w, t, t / 2, color);
    // Left Top
    gfx->fillRoundRect(x, y, t, h2, t / 2, color);
    // Middle
    gfx->fillRoundRect(x, y + h2 - t / 2, w, t, t / 2, color);
    // Right Bottom
    gfx->fillRoundRect(x + w - t, y + h2, t, h2, t / 2, color);
    // Bottom
    gfx->fillRoundRect(x, y + h - t, w, t, t / 2, color);
  }

  static void draw6(Arduino_GFX *gfx, int x, int y, int w, int h, int t,
                    uint16_t color) {
    int h2 = h / 2;
    // Left Full
    gfx->fillRoundRect(x, y, t, h, t / 2, color);
    // Top
    gfx->fillRoundRect(x, y, w, t, t / 2, color);
    // Bottom
    gfx->fillRoundRect(x, y + h - t, w, t, t / 2, color);
    // Middle
    gfx->fillRoundRect(x, y + h2 - t / 2, w, t, t / 2, color);
    // Right Bottom
    gfx->fillRoundRect(x + w - t, y + h2, t, h2, t / 2, color);
  }

  static void draw7(Arduino_GFX *gfx, int x, int y, int w, int h, int t,
                    uint16_t color) {
    // Top
    gfx->fillRoundRect(x, y, w, t, t / 2, color);
    // Diagonal? Or just Right? Usually Right for digital
    gfx->fillRoundRect(x + w - t, y, t, h, t / 2, color);
  }

  static void draw8(Arduino_GFX *gfx, int x, int y, int w, int h, int t,
                    uint16_t color) {
    // Full rect outer
    gfx->fillRoundRect(x, y, w, h, w / 2, color);
    // Holes logic is hard with primitives overlap.
    // Revert to segments
    int h2 = h / 2;
    // Top Box
    gfx->fillRoundRect(x, y, w, h2, t, color);
    gfx->fillRoundRect(x + t, y + t, w - 2 * t, h2 - 2 * t, t / 2,
                       0x0000); // Hole

    // Bottom Box
    gfx->fillRoundRect(x, y + h2 - (t / 2), w, h2 + (t / 2), t,
                       color); // Overlap slightly
    gfx->fillRoundRect(x + t, y + h2 + t - (t / 2), w - 2 * t, h2 - 2 * t,
                       t / 2, 0x0000); // Hole
  }

  static void draw9(Arduino_GFX *gfx, int x, int y, int w, int h, int t,
                    uint16_t color) {
    int h2 = h / 2;
    // Top Box (Full Loop)
    gfx->fillRoundRect(x, y, w, h2, t, color);
    gfx->fillRoundRect(x + t, y + t, w - 2 * t, h2 - 2 * t, t / 2,
                       0x0000); // Hole

    // Right Full
    gfx->fillRoundRect(x + w - t, y, t, h, t / 2, color);
    // OR Right Bottom and Bottom
    // gfx->fillRoundRect(x+w-t, y, t, h, t/2, color); will cover right side of
    // top box loop which is fine.

    // Bottom? usually 9 has bottom.
    // gfx->fillRoundRect(x, y+h-t, w, t, t/2, color);
  }
};

#endif // CLOCK_FONT_H
