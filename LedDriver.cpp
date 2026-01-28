#include "LedDriver.h"

LedDriver::LedDriver()
    : strip(NUM_LEDS, PIN_WS2812, NEO_GRB + NEO_KHZ800), lastUpdate(0),
      animationStep(0), currentEffect(OFF), solidColor(0x00D2FF),
      currentBrightness(150), effectSpeed(30), alarmState(false),
      headlightsOn(false), taillightsOn(false), turnSignalMode(TS_OFF) {}

void LedDriver::begin() {
  strip.begin();
  strip.setBrightness(currentBrightness);
  strip.show();
}

void LedDriver::update() {
  if (millis() - lastUpdate < (uint32_t)effectSpeed)
    return;
  lastUpdate = millis();

  // --- 1. Underglow (LEDs 0-3) ---
  if (!alarmState) {
    switch (currentEffect) {
    case OFF:
      for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++)
        strip.setPixelColor(i, 0);
      break;
    case RAINBOW:
      rainbowEffect();
      break;
    case SOLID:
      for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++)
        strip.setPixelColor(i, solidColor);
      break;
    case BREATH:
      breathEffect();
      break;
    case POLICE:
      policeEffect();
      break;
    case STROME:
      strobeEffect();
      break;
    case FADE:
      fadeEffect();
      break;
    case SPARKLE:
      sparkleEffect();
      break;
    case KNIGHT_RIDER:
      knightRiderEffect();
      break;
    default:
      break;
    }
  }

  // --- 2. Vehicle Lights & Alarm (LEDs 4-7 for vehicle, all for alarm) ---
  if (alarmState) {
    alarmEffect(); // Controls all 8 LEDs
  } else {
    // Base lights color
    uint32_t frontColor = headlightsOn ? strip.Color(255, 255, 255) : 0;
    uint32_t rearColor = taillightsOn ? strip.Color(255, 0, 0) : 0;

    // Apply base colors: 10-11 front, 8-9 rear
    for (int i = LED_HEAD_START; i <= LED_HEAD_END; i++)
      strip.setPixelColor(i, frontColor);
    for (int i = LED_TAIL_START; i <= LED_TAIL_END; i++)
      strip.setPixelColor(i, rearColor);

    // Turn Signal Logic (Smooth fading amber)
    if (turnSignalMode != TS_OFF) {
      uint32_t ms = millis() % 700;
      uint8_t amberBright = 0;

      if (ms < 150) { // Fade in
        amberBright = map(ms, 0, 150, 0, 255);
      } else if (ms < 350) { // Full bright stay
        amberBright = 255;
      } else if (ms < 500) { // Fade out
        amberBright = map(ms, 350, 500, 255, 0);
      } else { // Pause
        amberBright = 0;
      }

      if (amberBright > 0) {
        uint32_t amber =
            strip.Color((255 * amberBright) / 255, (45 * amberBright) / 255, 0);
        if (turnSignalMode == TS_LEFT) {
          strip.setPixelColor(10, amber); // Front Left
          strip.setPixelColor(8, amber);  // Rear Left
        } else if (turnSignalMode == TS_RIGHT) {
          strip.setPixelColor(11, amber); // Front Right
          strip.setPixelColor(9, amber);  // Rear Right
        } else if (turnSignalMode == TS_HAZARD) {
          for (int i = LED_TAIL_START; i <= LED_HEAD_END; i++)
            strip.setPixelColor(i, amber);
        }
      }
    }
  }
  strip.show();
}

void LedDriver::setBrightness(uint8_t brightness) {
  currentBrightness = brightness;
  strip.setBrightness(currentBrightness);
}

void LedDriver::setSpeed(int speed) {
  // Чем выше значение из UI (0-100), тем меньше задержка
  // Допустим UI шлет 1-100. 100 - быстро (10мс), 1 - медленно (200мс)
  effectSpeed = map(speed, 1, 100, 200, 10);
}

void LedDriver::setEffect(LedEffect effect) {
  currentEffect = effect;
  animationStep = 0;
}

void LedDriver::setColor(uint32_t color) { solidColor = color; }

void LedDriver::setVehicleHeadlights(bool on) { headlightsOn = on; }

void LedDriver::setVehicleTaillights(bool on) { taillightsOn = on; }

void LedDriver::setTurnSignal(TurnSignal mode) { turnSignalMode = mode; }

void LedDriver::setModeIdle() { alarmState = false; }

void LedDriver::setModeAlarm(AlarmCarEffect carEff, LedEffect ledEff) {
  alarmState = true;
  alarmCarEff = carEff;
  alarmLedEff = ledEff;
  animationStep = 0;
}

void LedDriver::rainbowEffect() {
  for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++) {
    int pixelHue = (animationStep + (i * 65536L / 4));
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
  }
  animationStep += 256;
}

void LedDriver::policeEffect() {
  // Сине-красная моргалка
  static int phase = 0;
  phase++;
  if (phase > 10)
    phase = 0;

  int mid = LED_UNDERGLOW_COUNT / 2;

  if (phase < 5) {
    for (int i = 0; i < mid; i++)
      strip.setPixelColor(LED_UNDERGLOW_START + i, 255, 0, 0);
    for (int i = mid; i < LED_UNDERGLOW_COUNT; i++)
      strip.setPixelColor(LED_UNDERGLOW_START + i, 0, 0, 255);
  } else {
    for (int i = 0; i < mid; i++)
      strip.setPixelColor(LED_UNDERGLOW_START + i, 0, 0, 255);
    for (int i = mid; i < LED_UNDERGLOW_COUNT; i++)
      strip.setPixelColor(LED_UNDERGLOW_START + i, 255, 0, 0);
  }
}

void LedDriver::breathEffect() {
  float intensity = (sin(animationStep * 0.05) + 1.0) / 2.0;
  uint8_t r = (uint8_t)((solidColor >> 16 & 0xFF) * intensity);
  uint8_t g = (uint8_t)((solidColor >> 8 & 0xFF) * intensity);
  uint8_t b = (uint8_t)((solidColor & 0xFF) * intensity);
  for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++)
    strip.setPixelColor(i, strip.Color(r, g, b));
  animationStep++;
}

void LedDriver::strobeEffect() {
  static bool on = false;
  on = !on;
  uint32_t color = on ? solidColor : 0;
  for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++)
    strip.setPixelColor(i, color);
}

void LedDriver::fadeEffect() {
  static int direction = 1;
  static int val = 0;

  val += direction * 5;
  if (val >= 255) {
    val = 255;
    direction = -1;
  }
  if (val <= 0) {
    val = 0;
    direction = 1;
  }

  uint8_t r = (uint8_t)((solidColor >> 16 & 0xFF) * (val / 255.0));
  uint8_t g = (uint8_t)((solidColor >> 8 & 0xFF) * (val / 255.0));
  uint8_t b = (uint8_t)((solidColor & 0xFF) * (val / 255.0));
  for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++)
    strip.setPixelColor(i, strip.Color(r, g, b));
}

void LedDriver::sparkleEffect() {
  for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++)
    strip.setPixelColor(i, 0);
  if (random(10) > 7) {
    int i = random(LED_UNDERGLOW_COUNT) + LED_UNDERGLOW_START;
    strip.setPixelColor(i, solidColor);
  }
}

void LedDriver::knightRiderEffect() {
  static int pos = 0;
  static int dir = 1;

  for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++)
    strip.setPixelColor(i, 0);
  strip.setPixelColor(pos, solidColor);

  pos += dir;
  if (pos <= LED_UNDERGLOW_START || pos >= LED_UNDERGLOW_END)
    dir = -dir;
}

void LedDriver::alarmEffect() {
  static bool alarmFlash = false;
  static int alarmSubStep = 0;
  alarmSubStep++;

  // Синхронизированная фаза мигания для всех эффектов тревоги
  if (alarmSubStep > 10) {
    alarmFlash = !alarmFlash;
    alarmSubStep = 0;
  }

  // 1. ПЕРЕДНИЕ И ЗАДНИЕ ФАРЫ (Пиксели 4-7)
  switch (alarmCarEff) {
  case ACE_BLINK:
    // Мигание только передних фар
    strip.setPixelColor(10, alarmFlash ? 0xFFFFFF : 0);
    strip.setPixelColor(11, alarmFlash ? 0xFFFFFF : 0);
    strip.setPixelColor(8, 0);
    strip.setPixelColor(9, 0);
    break;
  case ACE_HAZARD:
    // Аварийка (Оранжевый на всех 8-11)
    {
      uint32_t amber = alarmFlash ? strip.Color(255, 100, 0) : 0;
      for (int i = LED_TAIL_START; i <= LED_HEAD_END; i++)
        strip.setPixelColor(i, amber);
    }
    break;
  case ACE_POLICE:
    // Полицейское мерцание фар (перед белый, зад красный)
    strip.setPixelColor(10, alarmFlash ? 0xFFFFFF : 0);
    strip.setPixelColor(11, alarmFlash ? 0xFFFFFF : 0);
    strip.setPixelColor(8, !alarmFlash ? 0xFF0000 : 0);
    strip.setPixelColor(9, !alarmFlash ? 0xFF0000 : 0);
    break;
  default:
    // Выкл
    for (int i = LED_TAIL_START; i <= LED_HEAD_END; i++)
      strip.setPixelColor(i, 0);
    break;
  }

  // 2. ПОДСВЕТКА ДНИЩА (Пиксели 0-3)
  switch (alarmLedEff) {
  case RAINBOW:
    for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++) {
      int pixelHue = (animationStep + (i * 65536L / LED_UNDERGLOW_COUNT));
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    animationStep += 256;
    break;
  case POLICE:
    if (alarmFlash) {
      // 0-3 RED, 4-7 BLUE ? No, 0-3 first half, 4-7 second half
      // 8 LEDs total in Underglow
      for (int i = 0; i < 4; i++)
        strip.setPixelColor(i, 255, 0, 0);
      for (int i = 4; i < 8; i++)
        strip.setPixelColor(i, 0, 0, 255);
    } else {
      for (int i = 0; i < 4; i++)
        strip.setPixelColor(i, 0, 0, 255);
      for (int i = 4; i < 8; i++)
        strip.setPixelColor(i, 255, 0, 0);
    }
    break;
  case STROME:
    for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++) {
      strip.setPixelColor(i, alarmFlash ? 0xFFFFFF : 0);
    }
    break;
  case SOLID:
    for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++) {
      strip.setPixelColor(i, solidColor);
    }
    break;
  default:
    // По умолчанию - оранжевое мерцание днища
    {
      uint32_t amber = alarmFlash ? strip.Color(255, 50, 0) : 0;
      for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++)
        strip.setPixelColor(i, amber);
    }
    break;
  }
}
