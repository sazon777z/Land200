#include "NetworkManager.h"
#include "config.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <Preferences.h>

SemaphoreHandle_t WatchNetworkManager::systemMutex = NULL;

WatchNetworkManager::WatchNetworkManager()
    : lastWeatherUpdate(0), state(), stateMutex(NULL), lastTriggerMinute(-1) {}

void WatchNetworkManager::begin() {
  if (systemMutex == NULL) {
    systemMutex = xSemaphoreCreateMutex();
  }
  if (stateMutex == NULL) {
    stateMutex = xSemaphoreCreateMutex();
  }

  String ssid;
  String pass;
  int timeOffset = 5;
  String weatherCity = "Almaty";
  int alarmHour = 0;
  int alarmMinute = 0;
  int alarmSoundId = 1;
  int alarmVolume = 20;
  int alarmCarEffect = 1;
  int alarmLedEffect = 1;
  bool alarmEnabled = false;

  if (systemMutex != NULL && xSemaphoreTake(systemMutex, portMAX_DELAY) == pdTRUE) {
    preferences.begin("watch-config", false);
    ssid = preferences.getString("ssid", "");
    pass = preferences.getString("pass", "");
    timeOffset = preferences.getInt("timeOffset", 5);
    weatherCity = preferences.getString("city", "Almaty");
    alarmHour = preferences.getInt("alarmHour", 0);
    alarmMinute = preferences.getInt("alarmMinute", 0);
    alarmSoundId = preferences.getInt("alarmSound", 1);
    alarmVolume = preferences.getInt("alarmVol", 20);
    alarmCarEffect = preferences.getInt("alarmCarEff", 1);
    alarmLedEffect = preferences.getInt("alarmLedEff", 1);
    alarmEnabled = preferences.getBool("alarmEnabled", false);
    preferences.end();
    xSemaphoreGive(systemMutex);
  }

  if (stateMutex != NULL && xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    state.timeZoneOffset = timeOffset;
    snprintf(state.weatherCity, sizeof(state.weatherCity), "%s", weatherCity.c_str());
    state.alarmHour = alarmHour;
    state.alarmMinute = alarmMinute;
    state.alarmSoundId = alarmSoundId;
    state.alarmVolume = alarmVolume;
    state.alarmCarEffect = alarmCarEffect;
    state.alarmLedEffect = alarmLedEffect;
    state.alarmEnabled = alarmEnabled;
    state.connected = false;
    state.apMode = false;
    state.ipAddress[0] = '\0';
    state.apSSID[0] = '\0';
    state.apPassword[0] = '\0';
    xSemaphoreGive(stateMutex);
  }

  if (ssid == "") {
    if (String(WIFI_SSID) != "" && String(WIFI_SSID) != "YOUR_WIFI_SSID") {
      Serial.println("No saved WiFi. Using defaults from config.h.");
      ssid = WIFI_SSID;
      pass = WIFI_PASS;
    } else {
      Serial.println("No saved WiFi and no defaults. Starting AP.");
      setupAP();
      return;
    }
  }

  WiFi.persistent(true);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 40) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("lc200")) {
      Serial.println("mDNS responder started: http://lc200.local");
      MDNS.addService("http", "tcp", 80);
    }

    configTime(timeOffset * 3600, 0, NTP_SERVER);
    refreshTimeState();

    refreshConnectionState(true, false, WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nConnection failed. Starting AP fallback.");
    setupAP();
  }
}

void WatchNetworkManager::setupAP() {
  Serial.println("Preparing for AP Mode...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);

  WiFi.mode(WIFI_AP);
  delay(100);

  const String apSSID = "LC200-Watch-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  String apPassword = "12345678";

  bool success = WiFi.softAP(apSSID.c_str(), apPassword.c_str());
  if (!success) {
    Serial.println("AP Failed! Trying without password...");
    apPassword = "";
    WiFi.softAP(apSSID.c_str());
  }

  if (MDNS.begin("lc200")) {
    Serial.println("mDNS responder started: http://lc200.local");
    MDNS.addService("http", "tcp", 80);
  }

  Serial.println("AP Started");
  Serial.print("SSID: ");
  Serial.println(apSSID);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  refreshConnectionState(false, true, WiFi.softAPIP().toString().c_str(), apSSID.c_str(),
                         apPassword.c_str());
}

void WatchNetworkManager::refreshConnectionState(bool connected, bool apMode,
                                                 const char *ipAddress,
                                                 const char *apSSID,
                                                 const char *apPassword) {
  if (stateMutex == NULL) {
    return;
  }

  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    state.connected = connected;
    state.apMode = apMode;
    if (ipAddress) {
      snprintf(state.ipAddress, sizeof(state.ipAddress), "%s", ipAddress);
    }
    if (apSSID) {
      snprintf(state.apSSID, sizeof(state.apSSID), "%s", apSSID);
    }
    if (apPassword) {
      snprintf(state.apPassword, sizeof(state.apPassword), "%s", apPassword);
    }
    xSemaphoreGive(stateMutex);
  }
}

void WatchNetworkManager::refreshTimeState() {
  if (stateMutex == NULL) {
    return;
  }

  time_t rawTime = time(nullptr);
  struct tm timeInfo = {};
  localtime_r(&rawTime, &timeInfo);

  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    strftime(state.formattedTime, sizeof(state.formattedTime), "%H:%M:%S", &timeInfo);
    state.hour = timeInfo.tm_hour;
    state.minute = timeInfo.tm_min;
    state.second = timeInfo.tm_sec;
    state.weekdayIndex = timeInfo.tm_wday;
    state.dayOfMonth = timeInfo.tm_mday > 0 ? timeInfo.tm_mday : 1;
    state.monthIndex = timeInfo.tm_mon >= 0 ? timeInfo.tm_mon + 1 : 1;
    state.year = timeInfo.tm_year > 0 ? timeInfo.tm_year + 1900 : 1970;
    xSemaphoreGive(stateMutex);
  }
}

void WatchNetworkManager::update() {
  const bool apMode = isAPMode();

  if (!apMode && WiFi.status() == WL_CONNECTED) {
    refreshConnectionState(true, false, WiFi.localIP().toString().c_str());
    refreshTimeState();

    if (millis() - lastWeatherUpdate > WEATHER_UPDATE_INTERVAL_MS ||
        lastWeatherUpdate == 0) {
      updateWeather();
    }
  } else if (!apMode) {
    refreshConnectionState(false, false, "");

    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt > 30000) {
      lastReconnectAttempt = millis();
      Serial.println("Connection lost. Reconnecting...");
      WiFi.disconnect();
      WiFi.begin();
    }
  }
}

WatchStateSnapshot WatchNetworkManager::getSnapshot() {
  WatchStateSnapshot snapshot;

  if (stateMutex != NULL && xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    snapshot = state;
    xSemaphoreGive(stateMutex);
  }

  return snapshot;
}

String WatchNetworkManager::getFormattedTime() {
  if (stateMutex == NULL) {
    return "--:--:--";
  }

  String formattedTime = "--:--:--";
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    formattedTime = String(state.formattedTime);
    xSemaphoreGive(stateMutex);
  }

  return formattedTime;
}

int WatchNetworkManager::getHour() {
  if (stateMutex == NULL) {
    return 0;
  }

  int value = 0;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = state.hour;
    xSemaphoreGive(stateMutex);
  }

  return value;
}

int WatchNetworkManager::getMinute() {
  if (stateMutex == NULL) {
    return 0;
  }

  int value = 0;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = state.minute;
    xSemaphoreGive(stateMutex);
  }

  return value;
}

int WatchNetworkManager::getSecond() {
  if (stateMutex == NULL) {
    return 0;
  }

  int value = 0;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = state.second;
    xSemaphoreGive(stateMutex);
  }

  return value;
}

int WatchNetworkManager::getYear() {
  if (stateMutex == NULL) {
    return 1970;
  }

  int value = 1970;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = state.year;
    xSemaphoreGive(stateMutex);
  }

  return value;
}

float WatchNetworkManager::getTemperature() {
  if (stateMutex == NULL) {
    return 0.0f;
  }

  float value = 0.0f;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = state.temperature;
    xSemaphoreGive(stateMutex);
  }

  return value;
}

String WatchNetworkManager::getWeatherCondition() {
  if (stateMutex == NULL) {
    return "--";
  }

  String value = "--";
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = String(state.weatherCondition);
    xSemaphoreGive(stateMutex);
  }

  return value;
}

String WatchNetworkManager::getWeatherIcon() {
  if (stateMutex == NULL) {
    return "";
  }

  String value;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = String(state.weatherIcon);
    xSemaphoreGive(stateMutex);
  }

  return value;
}

bool WatchNetworkManager::isConnected() {
  if (stateMutex == NULL) {
    return false;
  }

  bool value = false;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = state.connected;
    xSemaphoreGive(stateMutex);
  }

  return value;
}

bool WatchNetworkManager::isAPMode() {
  if (stateMutex == NULL) {
    return false;
  }

  bool value = false;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = state.apMode;
    xSemaphoreGive(stateMutex);
  }

  return value;
}

String WatchNetworkManager::getApSSID() {
  if (stateMutex == NULL) {
    return "";
  }

  String value;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = String(state.apSSID);
    xSemaphoreGive(stateMutex);
  }

  return value;
}

String WatchNetworkManager::getApPassword() {
  if (stateMutex == NULL) {
    return "";
  }

  String value;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = String(state.apPassword);
    xSemaphoreGive(stateMutex);
  }

  return value;
}

String WatchNetworkManager::getIpAddress() {
  if (stateMutex == NULL) {
    return "";
  }

  String value;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = String(state.ipAddress);
    xSemaphoreGive(stateMutex);
  }

  return value;
}

void WatchNetworkManager::saveWiFiCredentials(String ssid, String pass) {
  if (systemMutex != NULL && xSemaphoreTake(systemMutex, portMAX_DELAY) == pdTRUE) {
    preferences.begin("watch-config", false);
    preferences.putString("ssid", ssid);
    preferences.putString("pass", pass);
    preferences.end();
    xSemaphoreGive(systemMutex);
  }

  Serial.println("Credentials saved. Please reboot.");
}

void WatchNetworkManager::saveLocalizationSettings(int timezoneOffset,
                                                   String city) {
  if (stateMutex != NULL && xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    state.timeZoneOffset = timezoneOffset;
    snprintf(state.weatherCity, sizeof(state.weatherCity), "%s", city.c_str());
    xSemaphoreGive(stateMutex);
  }

  if (systemMutex != NULL && xSemaphoreTake(systemMutex, portMAX_DELAY) == pdTRUE) {
    preferences.begin("watch-config", false);
    preferences.putInt("timeOffset", timezoneOffset);
    preferences.putString("city", city);
    preferences.end();
    xSemaphoreGive(systemMutex);
  }

  if (!isAPMode() && WiFi.status() == WL_CONNECTED) {
    configTime(timezoneOffset * 3600, 0, NTP_SERVER);
    refreshTimeState();
    updateWeather();
  }

  Serial.println("Localization settings saved.");
}

void WatchNetworkManager::saveAlarm(int hour, int minute, int soundId,
                                    int carEff, int ledEff, bool enabled) {
  if (stateMutex != NULL && xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    state.alarmHour = hour;
    state.alarmMinute = minute;
    state.alarmSoundId = soundId;
    state.alarmCarEffect = carEff;
    state.alarmLedEffect = ledEff;
    state.alarmEnabled = enabled;
    xSemaphoreGive(stateMutex);
  }

  if (systemMutex != NULL && xSemaphoreTake(systemMutex, portMAX_DELAY) == pdTRUE) {
    preferences.begin("watch-config", false);
    preferences.putInt("alarmHour", hour);
    preferences.putInt("alarmMinute", minute);
    preferences.putInt("alarmSound", soundId);
    preferences.putInt("alarmCarEff", carEff);
    preferences.putInt("alarmLedEff", ledEff);
    preferences.putBool("alarmEnabled", enabled);
    preferences.end();
    xSemaphoreGive(systemMutex);
  }

  Serial.printf("Alarm saved: %02d:%02d, Sound: %d, CarEff: %d, LedEff: %d, "
                "Enabled: %d\n",
                hour, minute, soundId, carEff, ledEff, enabled);
}

void WatchNetworkManager::saveAlarmVolume(int volume) {
  if (stateMutex != NULL && xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    state.alarmVolume = volume;
    xSemaphoreGive(stateMutex);
  }

  if (systemMutex != NULL && xSemaphoreTake(systemMutex, portMAX_DELAY) == pdTRUE) {
    preferences.begin("watch-config", false);
    preferences.putInt("alarmVol", volume);
    preferences.end();
    xSemaphoreGive(systemMutex);
  }

  Serial.printf("Alarm volume saved: %d\n", volume);
}

bool WatchNetworkManager::setAlarmEnabled(bool enabled) {
  if (stateMutex != NULL && xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    state.alarmEnabled = enabled;
    xSemaphoreGive(stateMutex);
  } else {
    state.alarmEnabled = enabled;
  }

  if (systemMutex != NULL && xSemaphoreTake(systemMutex, portMAX_DELAY) == pdTRUE) {
    preferences.begin("watch-config", false);
    preferences.putBool("alarmEnabled", enabled);
    preferences.end();
    xSemaphoreGive(systemMutex);
  }

  Serial.printf("Alarm enabled set to: %d\n", enabled);
  return enabled;
}

bool WatchNetworkManager::toggleAlarmEnabled() {
  bool enabled = false;

  if (stateMutex != NULL && xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    state.alarmEnabled = !state.alarmEnabled;
    enabled = state.alarmEnabled;
    xSemaphoreGive(stateMutex);
  } else {
    state.alarmEnabled = !state.alarmEnabled;
    enabled = state.alarmEnabled;
  }

  if (systemMutex != NULL && xSemaphoreTake(systemMutex, portMAX_DELAY) == pdTRUE) {
    preferences.begin("watch-config", false);
    preferences.putBool("alarmEnabled", enabled);
    preferences.end();
    xSemaphoreGive(systemMutex);
  }

  Serial.printf("Alarm toggled. Enabled: %d\n", enabled);
  return enabled;
}

int WatchNetworkManager::getAlarmHour() {
  if (stateMutex == NULL) {
    return 0;
  }

  int value = 0;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = state.alarmHour;
    xSemaphoreGive(stateMutex);
  }

  return value;
}

int WatchNetworkManager::getAlarmMinute() {
  if (stateMutex == NULL) {
    return 0;
  }

  int value = 0;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = state.alarmMinute;
    xSemaphoreGive(stateMutex);
  }

  return value;
}

int WatchNetworkManager::getAlarmSoundId() {
  if (stateMutex == NULL) {
    return 1;
  }

  int value = 1;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = state.alarmSoundId;
    xSemaphoreGive(stateMutex);
  }

  return value;
}

int WatchNetworkManager::getAlarmVolume() {
  if (stateMutex == NULL) {
    return 20;
  }

  int value = 20;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = state.alarmVolume;
    xSemaphoreGive(stateMutex);
  }

  return value;
}

int WatchNetworkManager::getAlarmCarEffect() {
  if (stateMutex == NULL) {
    return 1;
  }

  int value = 1;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = state.alarmCarEffect;
    xSemaphoreGive(stateMutex);
  }

  return value;
}

int WatchNetworkManager::getAlarmLedEffect() {
  if (stateMutex == NULL) {
    return 1;
  }

  int value = 1;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = state.alarmLedEffect;
    xSemaphoreGive(stateMutex);
  }

  return value;
}

bool WatchNetworkManager::isAlarmEnabled() {
  if (stateMutex == NULL) {
    return false;
  }

  bool value = false;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = state.alarmEnabled;
    xSemaphoreGive(stateMutex);
  }

  return value;
}

bool WatchNetworkManager::checkAlarmTrigger() {
  if (stateMutex == NULL) {
    return false;
  }

  bool shouldTrigger = false;

  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    if (!state.alarmEnabled || state.apMode || !state.connected) {
      xSemaphoreGive(stateMutex);
      return false;
    }

    if (state.hour == state.alarmHour && state.minute == state.alarmMinute) {
      if (lastTriggerMinute != state.minute) {
         lastTriggerMinute = state.minute;
         shouldTrigger = true;
      }
    } else if (lastTriggerMinute != -1) {
      lastTriggerMinute = -1;
    }

    xSemaphoreGive(stateMutex);
  }

  return shouldTrigger;
}

void WatchNetworkManager::resetAlarmTrigger() {
  if (stateMutex != NULL && xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    lastTriggerMinute = state.minute;
    xSemaphoreGive(stateMutex);
  }
}

int WatchNetworkManager::getTimeZoneOffset() {
  if (stateMutex == NULL) {
    return 5;
  }

  int value = 5;
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = state.timeZoneOffset;
    xSemaphoreGive(stateMutex);
  }

  return value;
}

String WatchNetworkManager::getWeatherCity() {
  if (stateMutex == NULL) {
    return "Almaty";
  }

  String value = "Almaty";
  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    value = String(state.weatherCity);
    xSemaphoreGive(stateMutex);
  }

  return value;
}

void WatchNetworkManager::updateWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  char city[48] = "Almaty";
  if (stateMutex != NULL && xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
    snprintf(city, sizeof(city), "%s", state.weatherCity);
    xSemaphoreGive(stateMutex);
  }

  String encodedCity = String(city);
  encodedCity.replace(" ", "%20");

  HTTPClient http;
  String url =
      "http://api.openweathermap.org/data/2.5/weather?q=" + encodedCity +
      "&units=metric&appid=" + String(WEATHER_API_KEY) + "&lang=ru";

  lastWeatherUpdate = millis();
  Serial.print("Updating weather for: ");
  Serial.println(city);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    // Использование компактного документа для сохранения RAM на стеке
    StaticJsonDocument<768> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      if (doc.containsKey("main") && doc["weather"][0].is<JsonObject>()) {
        const float temperature = doc["main"]["temp"].as<float>();
        const String condition = String(doc["weather"][0]["description"] | "--");
        const String icon = String(doc["weather"][0]["icon"] | "");

        if (stateMutex != NULL &&
            xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
          state.temperature = temperature;
          snprintf(state.weatherCondition, sizeof(state.weatherCondition), "%s", condition.c_str());
          snprintf(state.weatherIcon, sizeof(state.weatherIcon), "%s", icon.c_str());
          xSemaphoreGive(stateMutex);
        }

        Serial.printf("Weather updated: %.1f C, %s\n", temperature,
                      condition.c_str());
      } else {
        Serial.println("Error: incomplete weather JSON");
      }
    } else {
      Serial.print("JSON Error: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.printf("HTTP Error: %d\n", httpCode);
  }

  http.end();
}
