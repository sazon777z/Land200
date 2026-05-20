#line 1 "C:\\Users\\JZX\\Documents\\Arduino\\Land_Cruiser\\ClockTimeFont.h"
#ifndef CLOCK_TIME_FONT_H
#define CLOCK_TIME_FONT_H

#if defined(__has_include)
#if __has_include("../libraries/Adafruit_GFX_Library/Fonts/FreeSansBoldOblique24pt7b.h")
#include "../libraries/Adafruit_GFX_Library/Fonts/FreeSansBoldOblique24pt7b.h"
#define CLOCK_TIME_FONT_LARGE FreeSansBoldOblique24pt7b
#else
#include "FreeSansBold24pt7b.h"
#define CLOCK_TIME_FONT_LARGE FreeSansBold24pt7b
#endif
#else
#include "FreeSansBold24pt7b.h"
#define CLOCK_TIME_FONT_LARGE FreeSansBold24pt7b
#endif

#endif // CLOCK_TIME_FONT_H
