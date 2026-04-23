#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include "config.h"
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class LedDriver {
public:
  LedDriver();
  void begin();
  void update(); // Called periodically for animations

  // Vehicle Light States
  enum TurnSignal { TS_OFF, TS_LEFT, TS_RIGHT, TS_HAZARD };

  // Effect modes
  enum LedEffect {
    OFF,
    RAINBOW,
    SOLID,
    BREATH,
    POLICE,
    STROME, // Стробоскоп
    FADE,   // Плавное затухание
    SPARKLE,
    KNIGHT_RIDER,
    ALARM
  };

  enum AlarmCarEffect { ACE_OFF, ACE_BLINK, ACE_HAZARD, ACE_POLICE };

  void setEffect(LedEffect effect);
  void setColor(uint32_t color);
  void setBrightness(uint8_t brightness);
  void setSpeed(int speed);
  int getCurrentEffect();
  uint8_t getBrightness() const { return currentBrightness; }
  int getSpeed() const { return effectSpeed; }

  // Vehicle controls
  void setVehicleHeadlights(bool on);
  void setVehicleTaillights(bool on);
  void setTurnSignal(TurnSignal mode);

  void setModeIdle();
  void setModeAlarm(AlarmCarEffect carEff = ACE_BLINK,
                    LedEffect ledEff = RAINBOW);
  bool isAlarmActive();

private:
  Adafruit_NeoPixel strip;
  SemaphoreHandle_t stateMutex;
  uint32_t lastUpdate;
  int animationStep;
  LedEffect currentEffect;
  uint32_t solidColor;
  uint8_t currentBrightness;
  int effectSpeed;
  bool alarmState;
  AlarmCarEffect alarmCarEff;
  LedEffect alarmLedEff;

  // Vehicle state
  bool headlightsOn;
  bool taillightsOn;
  TurnSignal turnSignalMode;

  void rainbowEffect();
  void policeEffect();
  void breathEffect();
  void strobeEffect();
  void fadeEffect();
  void sparkleEffect();
  void knightRiderEffect();
  void alarmEffect();
  uint32_t makeSignalColor(uint8_t baseR, uint8_t baseG, uint8_t baseB,
                           uint8_t signalBrightness);
};

#endif // LED_DRIVER_H
