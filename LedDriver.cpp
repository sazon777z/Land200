#include "LedDriver.h"

LedDriver::LedDriver()
    : strip(NUM_LEDS, PIN_WS2812, NEO_GRB + NEO_KHZ800), stateMutex(NULL),
      lastUpdate(0), animationStep(0), currentEffect(OFF), solidColor(0x00D2FF),
      currentBrightness(150), effectSpeed(30), alarmState(false),
      alarmCarEff(ACE_BLINK), alarmLedEff(RAINBOW), headlightsOn(false),
      taillightsOn(false), turnSignalMode(TS_OFF) {}

void LedDriver::begin() {
  if (stateMutex == NULL) {
    stateMutex = xSemaphoreCreateMutex();
  }

  strip.begin();
  strip.setBrightness(currentBrightness);
  strip.show();
}

void LedDriver::update() {
  if (stateMutex == NULL) {
    return;
  }

  if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(5)) != pdTRUE) {
    return;
  }

  if (millis() - lastUpdate < (uint32_t)effectSpeed) {
    xSemaphoreGive(stateMutex);
    return;
  }
  lastUpdate = millis();

  if (!alarmState) {
    switch (currentEffect) {
    case OFF:
      for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++) {
        strip.setPixelColor(i, 0);
      }
      break;
    case RAINBOW:
      rainbowEffect();
      break;
    case SOLID:
      for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++) {
        strip.setPixelColor(i, solidColor);
      }
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

  if (alarmState) {
    alarmEffect();
  } else {
    uint32_t frontColor = headlightsOn ? strip.Color(255, 255, 255) : 0;
    uint32_t rearColor = taillightsOn ? strip.Color(255, 0, 0) : 0;

    for (int i = LED_HEAD_START; i <= LED_HEAD_END; i++) {
      strip.setPixelColor(i, frontColor);
    }
    for (int i = LED_TAIL_START; i <= LED_TAIL_END; i++) {
      strip.setPixelColor(i, rearColor);
    }

    if (turnSignalMode != TS_OFF) {
      uint32_t ms = millis() % 700;
      uint8_t amberBright = 0;

      if (ms < 150) {
        amberBright = map(ms, 0, 150, 0, 255);
      } else if (ms < 350) {
        amberBright = 255;
      } else if (ms < 500) {
        amberBright = map(ms, 350, 500, 255, 0);
      } else {
        amberBright = 0;
      }

      if (turnSignalMode == TS_LEFT) {
        strip.setPixelColor(
            10, makeSignalColor(headlightsOn ? 255 : 0, headlightsOn ? 255 : 0,
                                headlightsOn ? 255 : 0, amberBright));
        strip.setPixelColor(
            9, makeSignalColor(taillightsOn ? 255 : 0, 0, 0, amberBright));
      } else if (turnSignalMode == TS_RIGHT) {
        strip.setPixelColor(
            11, makeSignalColor(headlightsOn ? 255 : 0, headlightsOn ? 255 : 0,
                                headlightsOn ? 255 : 0, amberBright));
        strip.setPixelColor(
            8, makeSignalColor(taillightsOn ? 255 : 0, 0, 0, amberBright));
      } else if (turnSignalMode == TS_HAZARD) {
        strip.setPixelColor(
            10, makeSignalColor(headlightsOn ? 255 : 0, headlightsOn ? 255 : 0,
                                headlightsOn ? 255 : 0, amberBright));
        strip.setPixelColor(
            11, makeSignalColor(headlightsOn ? 255 : 0, headlightsOn ? 255 : 0,
                                headlightsOn ? 255 : 0, amberBright));
        strip.setPixelColor(
            8, makeSignalColor(taillightsOn ? 255 : 0, 0, 0, amberBright));
        strip.setPixelColor(
            9, makeSignalColor(taillightsOn ? 255 : 0, 0, 0, amberBright));
      }
    }
  }

  strip.show();
  xSemaphoreGive(stateMutex);
}

uint32_t LedDriver::makeSignalColor(uint8_t baseR, uint8_t baseG, uint8_t baseB,
                                    uint8_t signalBrightness) {
  auto blendChannel = [signalBrightness](uint8_t base,
                                         uint8_t signal) -> uint8_t {
    return (static_cast<uint16_t>(base) * (255 - signalBrightness) +
            static_cast<uint16_t>(signal) * signalBrightness) /
           255;
  };

  return strip.Color(blendChannel(baseR, 255), blendChannel(baseG, 45),
                     blendChannel(baseB, 0));
}

void LedDriver::setBrightness(uint8_t brightness) {
  if (stateMutex != NULL &&
      xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    currentBrightness = brightness;
    strip.setBrightness(currentBrightness);
    xSemaphoreGive(stateMutex);
    return;
  }

  currentBrightness = brightness;
}

void LedDriver::setSpeed(int speed) {
  const int mappedSpeed = map(speed, 1, 100, 200, 10);
  if (stateMutex != NULL &&
      xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    effectSpeed = mappedSpeed;
    xSemaphoreGive(stateMutex);
    return;
  }

  effectSpeed = mappedSpeed;
}

int LedDriver::getCurrentEffect() {
  if (stateMutex != NULL &&
      xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    const int effect = static_cast<int>(currentEffect);
    xSemaphoreGive(stateMutex);
    return effect;
  }

  return static_cast<int>(currentEffect);
}

void LedDriver::setEffect(LedEffect effect) {
  Serial.printf("LedDriver: Setting effect to %d\n", static_cast<int>(effect));
  if (stateMutex != NULL &&
      xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    currentEffect = effect;
    animationStep = 0;
    xSemaphoreGive(stateMutex);
    return;
  }

  currentEffect = effect;
  animationStep = 0;
}

void LedDriver::setColor(uint32_t color) {
  if (stateMutex != NULL &&
      xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    solidColor = color;
    xSemaphoreGive(stateMutex);
    return;
  }

  solidColor = color;
}

void LedDriver::setVehicleHeadlights(bool on) {
  if (stateMutex != NULL &&
      xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    headlightsOn = on;
    xSemaphoreGive(stateMutex);
    return;
  }

  headlightsOn = on;
}

void LedDriver::setVehicleTaillights(bool on) {
  if (stateMutex != NULL &&
      xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    taillightsOn = on;
    xSemaphoreGive(stateMutex);
    return;
  }

  taillightsOn = on;
}

void LedDriver::setTurnSignal(TurnSignal mode) {
  if (stateMutex != NULL &&
      xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    turnSignalMode = mode;
    xSemaphoreGive(stateMutex);
    return;
  }

  turnSignalMode = mode;
}

void LedDriver::setModeIdle() {
  if (stateMutex != NULL &&
      xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    alarmState = false;
    xSemaphoreGive(stateMutex);
    return;
  }

  alarmState = false;
}

void LedDriver::setModeAlarm(AlarmCarEffect carEff, LedEffect ledEff) {
  if (stateMutex != NULL &&
      xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    alarmState = true;
    alarmCarEff = carEff;
    alarmLedEff = ledEff;
    animationStep = 0;
    xSemaphoreGive(stateMutex);
    return;
  }

  alarmState = true;
  alarmCarEff = carEff;
  alarmLedEff = ledEff;
  animationStep = 0;
}

bool LedDriver::isAlarmActive() {
  if (stateMutex != NULL &&
      xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    const bool active = alarmState;
    xSemaphoreGive(stateMutex);
    return active;
  }

  return alarmState;
}

void LedDriver::rainbowEffect() {
  for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++) {
    int pixelHue = (animationStep + (i * 65536L / 4));
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
  }
  animationStep += 256;
}

void LedDriver::policeEffect() {
  static int phase = 0;
  phase++;
  if (phase > 10) {
    phase = 0;
  }

  int mid = LED_UNDERGLOW_COUNT / 2;

  if (phase < 5) {
    for (int i = 0; i < mid; i++) {
      strip.setPixelColor(LED_UNDERGLOW_START + i, 255, 0, 0);
    }
    for (int i = mid; i < LED_UNDERGLOW_COUNT; i++) {
      strip.setPixelColor(LED_UNDERGLOW_START + i, 0, 0, 255);
    }
  } else {
    for (int i = 0; i < mid; i++) {
      strip.setPixelColor(LED_UNDERGLOW_START + i, 0, 0, 255);
    }
    for (int i = mid; i < LED_UNDERGLOW_COUNT; i++) {
      strip.setPixelColor(LED_UNDERGLOW_START + i, 255, 0, 0);
    }
  }
}

void LedDriver::breathEffect() {
  float intensity = (sin(animationStep * 0.05) + 1.0) / 2.0;
  uint8_t r = (uint8_t)((solidColor >> 16 & 0xFF) * intensity);
  uint8_t g = (uint8_t)((solidColor >> 8 & 0xFF) * intensity);
  uint8_t b = (uint8_t)((solidColor & 0xFF) * intensity);

  for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  animationStep++;
}

void LedDriver::strobeEffect() {
  static bool on = false;
  on = !on;

  uint32_t color = on ? solidColor : 0;
  for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++) {
    strip.setPixelColor(i, color);
  }
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

  for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
}

void LedDriver::sparkleEffect() {
  for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++) {
    strip.setPixelColor(i, 0);
  }

  if (random(10) > 7) {
    int i = random(LED_UNDERGLOW_COUNT) + LED_UNDERGLOW_START;
    strip.setPixelColor(i, solidColor);
  }
}

void LedDriver::knightRiderEffect() {
  static int pos = 0;
  static int dir = 1;

  for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++) {
    strip.setPixelColor(i, 0);
  }
  strip.setPixelColor(pos, solidColor);

  pos += dir;
  if (pos <= LED_UNDERGLOW_START || pos >= LED_UNDERGLOW_END) {
    dir = -dir;
  }
}

void LedDriver::alarmEffect() {
  static bool alarmFlash = false;
  static int alarmSubStep = 0;
  alarmSubStep++;

  if (alarmSubStep > 10) {
    alarmFlash = !alarmFlash;
    alarmSubStep = 0;
  }

  switch (alarmCarEff) {
  case ACE_BLINK:
    strip.setPixelColor(10, alarmFlash ? 0xFFFFFF : 0);
    strip.setPixelColor(11, alarmFlash ? 0xFFFFFF : 0);
    strip.setPixelColor(8, 0);
    strip.setPixelColor(9, 0);
    break;
  case ACE_HAZARD: {
    uint32_t amber = alarmFlash ? strip.Color(255, 100, 0) : 0;
    for (int i = LED_TAIL_START; i <= LED_HEAD_END; i++) {
      strip.setPixelColor(i, amber);
    }
    break;
  }
  case ACE_POLICE:
    strip.setPixelColor(10, alarmFlash ? 0xFFFFFF : 0);
    strip.setPixelColor(11, alarmFlash ? 0xFFFFFF : 0);
    strip.setPixelColor(8, !alarmFlash ? 0xFF0000 : 0);
    strip.setPixelColor(9, !alarmFlash ? 0xFF0000 : 0);
    break;
  default:
    for (int i = LED_TAIL_START; i <= LED_HEAD_END; i++) {
      strip.setPixelColor(i, 0);
    }
    break;
  }

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
      for (int i = 0; i < 4; i++) {
        strip.setPixelColor(i, 255, 0, 0);
      }
      for (int i = 4; i < 8; i++) {
        strip.setPixelColor(i, 0, 0, 255);
      }
    } else {
      for (int i = 0; i < 4; i++) {
        strip.setPixelColor(i, 0, 0, 255);
      }
      for (int i = 4; i < 8; i++) {
        strip.setPixelColor(i, 255, 0, 0);
      }
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
  default: {
    uint32_t amber = alarmFlash ? strip.Color(255, 50, 0) : 0;
    for (int i = LED_UNDERGLOW_START; i <= LED_UNDERGLOW_END; i++) {
      strip.setPixelColor(i, amber);
    }
    break;
  }
  }
}
