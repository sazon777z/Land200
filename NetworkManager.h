#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <HTTPClient.h>
#include <NTPClient.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class WatchNetworkManager {
public:
  WatchNetworkManager();
  void begin();
  void update(); // Handle NTP and Weather updates

  String getFormattedTime();
  int getHour();
  int getMinute();
  int getSecond();
  int getYear();

  float getTemperature();
  String getWeatherCondition();
  String getWeatherIcon();

  bool isConnected();
  bool isAPMode();
  String getApSSID();
  String getApPassword();
  String getIpAddress();

  // Settings getters (for UI sync)
  int getTimeZoneOffset();
  String getWeatherCity();

  // Alarm settings
  void saveAlarm(int hour, int minute, int soundId, int carEff, int ledEff,
                 bool enabled);
  void saveAlarmVolume(int volume);
  int getAlarmHour() { return alarmHour; }
  int getAlarmMinute() { return alarmMinute; }
  int getAlarmSoundId() { return alarmSoundId; }
  int getAlarmVolume() { return alarmVolume; }
  int getAlarmCarEffect() { return alarmCarEffect; }
  int getAlarmLedEffect() { return alarmLedEffect; }
  bool isAlarmEnabled() { return alarmEnabled; }
  bool checkAlarmTrigger();
  void resetAlarmTrigger(); // Manual stop

  // Settings setters
  void saveWiFiCredentials(String ssid, String pass);
  void saveLocalizationSettings(int timezoneOffset, String city);

private:
  WiFiUDP ntpUDP;
  NTPClient *timeClient;
  Preferences preferences;

  unsigned long lastWeatherUpdate;
  float currentTemp;
  String currentCondition;
  String currentIcon;

  // Localization
  int timeOffset;
  String weatherCity;

  // Alarm state
  int alarmHour;
  int alarmMinute;
  int alarmSoundId;
  int alarmVolume;
  int alarmCarEffect;
  int alarmLedEffect;
  bool alarmEnabled;
  bool alarmTriggeredToday;
  int lastTriggerMinute; // To prevent multiple triggers in one minute

  bool apMode;

  // Мутекс для общих ресурсов (static чтобы был один на все экземпляры, если их
  // вдруг будет больше)
  static SemaphoreHandle_t systemMutex;

  void updateWeather();
  void setupAP();
};

#endif // NETWORK_MANAGER_H
