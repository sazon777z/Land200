#include "LedDriver.h"

LedDriver::LedDriver()
    : strip(NUM_LEDS, PIN_WS2812, NEO_GRB + NEO_KHZ800), lastUpdate(0),
      animationStep(0), currentEffect(RAINBOW), solidColor(0x00D2FF),
      currentBrightness(150), effectSpeed(30), alarmState(false),
      headlightsOn(false), taillightsOn(false), turnSignalMode(TS_OFF) {}

void LedDriver::begin() {
  strip.begin();
  strip.setBrightness(currentBrightness);
  strip.show();

  pinMode(PIN_FRONT_LEDS, OUTPUT);
  pinMode(PIN_REAR_LEDS, OUTPUT);

  digitalWrite(PIN_FRONT_LEDS, LOW);
  digitalWrite(PIN_REAR_LEDS, LOW);
}

void LedDriver::update() {
  if (millis() - lastUpdate < (uint32_t)effectSpeed)
    return;
  lastUpdate = millis();

  // --- 1. Underglow (LEDs 0-3) ---
  if (!alarmState) {
    switch (currentEffect) {
    case OFF:
      for (int i = 0; i < 4; i++)
        strip.setPixelColor(i, 0);
      break;
    case RAINBOW:
      rainbowEffect();
      break;
    case SOLID:
      for (int i = 0; i < 4; i++)
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

    // Apply base colors to 4-7
    strip.setPixelColor(4, frontColor);
    strip.setPixelColor(5, frontColor);
    strip.setPixelColor(6, rearColor);
    strip.setPixelColor(7, rearColor);

    // Turn Signal Logic (Blinking amber/yellow)
    static bool turnFlash = false;
    static uint32_t lastTurnUpdate = 0;
    if (millis() - lastTurnUpdate > 500) {
      turnFlash = !turnFlash;
      lastTurnUpdate = millis();
    }

    if (turnFlash && turnSignalMode != TS_OFF) {
      uint32_t amber = strip.Color(255, 80, 0);
      if (turnSignalMode == TS_LEFT) {
        strip.setPixelColor(4, amber);
        strip.setPixelColor(6, amber);
      } else if (turnSignalMode == TS_RIGHT) {
        strip.setPixelColor(5, amber);
        strip.setPixelColor(7, amber);
      } else if (turnSignalMode == TS_HAZARD) {
        strip.setPixelColor(4, amber);
        strip.setPixelColor(5, amber);
        strip.setPixelColor(6, amber);
        strip.setPixelColor(7, amber);
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

void LedDriver::setModeIdle() {
  alarmState = false;
  setHeadlights(true);
  setTaillights(true);
}

void LedDriver::setModeAlarm(AlarmCarEffect carEff, LedEffect ledEff) {
  alarmState = true;
  alarmCarEff = carEff;
  alarmLedEff = ledEff;
  animationStep = 0;
}

void LedDriver::setHeadlights(bool on) {
  digitalWrite(PIN_FRONT_LEDS, on ? HIGH : LOW);
}

void LedDriver::setTaillights(bool on) {
  digitalWrite(PIN_REAR_LEDS, on ? HIGH : LOW);
}

void LedDriver::rainbowEffect() {
  for (int i = 0; i < 4; i++) {
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

  if (phase < 5) {
    strip.setPixelColor(0, 255, 0, 0);
    strip.setPixelColor(1, 255, 0, 0);
    strip.setPixelColor(2, 0, 0, 255);
    strip.setPixelColor(3, 0, 0, 255);
  } else {
    strip.setPixelColor(0, 0, 0, 255);
    strip.setPixelColor(1, 0, 0, 255);
    strip.setPixelColor(2, 255, 0, 0);
    strip.setPixelColor(3, 255, 0, 0);
  }
}

void LedDriver::breathEffect() {
  float intensity = (sin(animationStep * 0.05) + 1.0) / 2.0;
  uint8_t r = (uint8_t)((solidColor >> 16 & 0xFF) * intensity);
  uint8_t g = (uint8_t)((solidColor >> 8 & 0xFF) * intensity);
  uint8_t b = (uint8_t)((solidColor & 0xFF) * intensity);
  for (int i = 0; i < 4; i++)
    strip.setPixelColor(i, strip.Color(r, g, b));
  animationStep++;
}

void LedDriver::strobeEffect() {
  static bool on = false;
  on = !on;
  uint32_t color = on ? solidColor : 0;
  for (int i = 0; i < 4; i++)
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
  for (int i = 0; i < 4; i++)
    strip.setPixelColor(i, strip.Color(r, g, b));
}

void LedDriver::sparkleEffect() {
  for (int i = 0; i < 4; i++)
    strip.setPixelColor(i, 0);
  if (random(10) > 7) {
    int i = random(4);
    strip.setPixelColor(i, solidColor);
  }
}

void LedDriver::knightRiderEffect() {
  static int pos = 0;
  static int dir = 1;

  for (int i = 0; i < 4; i++)
    strip.setPixelColor(i, 0);
  strip.setPixelColor(pos, solidColor);

  pos += dir;
  if (pos <= 0 || pos >= 3)
    dir = -dir;
}

void LedDriver::alarmEffect() {
  static bool flash = false;
  static int flashCount = 0;
  flashCount++;

  // speed management for alarm
  if (flashCount > 10) {
    flash = !flash;
    flashCount = 0;
  }

  // 1. Car Lights Effect
  switch (alarmCarEff) {
  case ACE_BLINK:
    setHeadlights(flash);
    setTaillights(false);
    break;
  case ACE_HAZARD:
    setHeadlights(flash);
    setTaillights(flash);
    // Hazard usually means amber turn signals too, but here we use digital pins
    // for main lamps
    break;
  case ACE_POLICE:
    setHeadlights(flash);
    setTaillights(!flash);
    break;
  default:
    setHeadlights(false);
    setTaillights(false);
    break;
  }

  // 2. Underglow (Strip) Effect
  // We reuse existing effects but applied to the whole strip during alarm
  switch (alarmLedEff) {
  case RAINBOW:
    for (int i = 0; i < strip.numPixels(); i++) {
      int pixelHue = (animationStep + (i * 65536L / strip.numPixels()));
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    animationStep += 256;
    break;
  case POLICE:
    if (flash) {
      strip.fill(strip.Color(255, 0, 0), 0, strip.numPixels() / 2);
      strip.fill(strip.Color(0, 0, 255), strip.numPixels() / 2);
    } else {
      strip.fill(strip.Color(0, 0, 255), 0, strip.numPixels() / 2);
      strip.fill(strip.Color(255, 0, 0), strip.numPixels() / 2);
    }
    break;
  case STROME:
    strip.fill(flash ? 0xFFFFFF : 0);
    break;
  case SOLID:
    strip.fill(solidColor);
    break;
  default:
    // Default hazard blink (amber)
    strip.fill(flash ? strip.Color(255, 100, 0) : 0);
    break;
  }
}
