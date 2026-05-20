#line 1 "C:\\Users\\JZX\\Documents\\Arduino\\Land_Cruiser\\NetworkManager.h"
#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <HTTPClient.h>
#include <NTPClient.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <time.h>

struct WatchStateSnapshot {
  String formattedTime = "--:--:--";
  int hour = 0;
  int minute = 0;
  int second = 0;
  int weekdayIndex = 4;
  int dayOfMonth = 1;
  int monthIndex = 1;
  int year = 1970;

  float temperature = 0.0f;
  String weatherCondition = "--";
  String weatherIcon = "";

  bool connected = false;
  bool apMode = false;
  String apSSID = "";
  String apPassword = "";
  String ipAddress = "";

  int timeZoneOffset = 5;
  String weatherCity = "Almaty";

  int alarmHour = 0;
  int alarmMinute = 0;
  int alarmSoundId = 1;
  int alarmVolume = 20;
  int alarmCarEffect = 1;
  int alarmLedEffect = 1;
  bool alarmEnabled = false;
};

class WatchNetworkManager {
public:
  WatchNetworkManager();
  void begin();
  void update();
  WatchStateSnapshot getSnapshot();

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

  int getTimeZoneOffset();
  String getWeatherCity();

  void saveAlarm(int hour, int minute, int soundId, int carEff, int ledEff,
                 bool enabled);
  void saveAlarmVolume(int volume);
  bool setAlarmEnabled(bool enabled);
  bool toggleAlarmEnabled();
  int getAlarmHour();
  int getAlarmMinute();
  int getAlarmSoundId();
  int getAlarmVolume();
  int getAlarmCarEffect();
  int getAlarmLedEffect();
  bool isAlarmEnabled();
  bool checkAlarmTrigger();
  void resetAlarmTrigger();

  void saveWiFiCredentials(String ssid, String pass);
  void saveLocalizationSettings(int timezoneOffset, String city);

private:
  WiFiUDP ntpUDP;
  NTPClient *timeClient;
  Preferences preferences;

  unsigned long lastWeatherUpdate;
  WatchStateSnapshot state;
  SemaphoreHandle_t stateMutex;
  int lastTriggerMinute;

  static SemaphoreHandle_t systemMutex;

  void updateWeather();
  void setupAP();
  void refreshTimeState();
  void refreshConnectionState(bool connected, bool apMode,
                              const String &ipAddress,
                              const String &apSSID = "",
                              const String &apPassword = "");
};

#endif // NETWORK_MANAGER_H
