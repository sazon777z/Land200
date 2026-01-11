#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include <Adafruit_NeoPixel.h>
#include "config.h"

class LedDriver {
public:
    LedDriver();
    void begin();
    void update(); // Called periodically for animations
    
    // Vehicle Light States
    enum TurnSignal {
        TS_OFF,
        TS_LEFT,
        TS_RIGHT,
        TS_HAZARD
    };

    // Effect modes
    enum LedEffect {
        OFF,
        RAINBOW,
        SOLID,
        BREATH,
        POLICE,
        STROME,   // Стробоскоп
        FADE,     // Плавное затухание
        SPARKLE,
        KNIGHT_RIDER,
        ALARM
    };

    void setEffect(LedEffect effect);
    void setColor(uint32_t color);
    void setBrightness(uint8_t brightness);
    void setSpeed(int speed);
    
    // Vehicle controls
    void setVehicleHeadlights(bool on);
    void setVehicleTaillights(bool on);
    void setTurnSignal(TurnSignal mode);

    void setModeIdle();
    void setModeAlarm();
    
    // Physical pin controls (legacy)
    void setHeadlights(bool on);
    void setTaillights(bool on);

private:
    Adafruit_NeoPixel strip;
    uint32_t lastUpdate;
    int animationStep;
    LedEffect currentEffect;
    uint32_t solidColor;
    uint8_t currentBrightness;
    int effectSpeed;
    bool alarmState;
    
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
};

#endif // LED_DRIVER_H
