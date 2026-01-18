#include "NetworkManager.h"
#include "config.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <Preferences.h>

// Определение статического мьютекса
SemaphoreHandle_t WatchNetworkManager::systemMutex = NULL;

WatchNetworkManager::WatchNetworkManager()
    : timeClient(new NTPClient(ntpUDP, NTP_SERVER, GMT_OFFSET_SEC,
                               DAYLIGHT_OFFSET_SEC)),
      lastWeatherUpdate(0), currentTemp(0.0), currentCondition("--"),
      currentIcon(""), timeOffset(5), weatherCity("Almaty"), alarmHour(0),
      alarmMinute(0), alarmSoundId(1), alarmVolume(20), alarmEnabled(false),
      alarmTriggeredToday(false), lastTriggerMinute(-1), apMode(false) {}

void WatchNetworkManager::begin() {
  if (systemMutex == NULL) {
    systemMutex = xSemaphoreCreateMutex();
  }

  xSemaphoreTake(systemMutex, portMAX_DELAY);

  preferences.begin("watch-config", false);
  String ssid = preferences.getString("ssid", "");
  String pass = preferences.getString("pass", "");

  // Load localization settings
  timeOffset = preferences.getInt("timeOffset", 5);
  weatherCity = preferences.getString("city", "Almaty");

  // Load alarm settings
  alarmHour = preferences.getInt("alarmHour", 0);
  alarmMinute = preferences.getInt("alarmMinute", 0);
  alarmSoundId = preferences.getInt("alarmSound", 1);
  alarmVolume = preferences.getInt("alarmVol", 20);
  alarmEnabled = preferences.getBool("alarmEnabled", false);

  preferences.end();
  xSemaphoreGive(systemMutex);

  if (ssid == "") {
    Serial.println("No saved WiFi. Starting AP.");
    preferences.end(); // Close before return
    setupAP();
    return;
  }

  WiFi.persistent(true);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED &&
         retries < 40) { // Wait up to 20 seconds
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("artemLandCruiser")) {
      Serial.println("mDNS responder started: http://artemLandCruiser.local");
      MDNS.addService("http", "tcp", 80);
    }

    timeClient->setTimeOffset(timeOffset * 3600);
    timeClient->begin();
    apMode = false;
  } else {
    Serial.println("\nConnection failed. Starting AP fallback.");
    setupAP();
  }
  preferences.end();
}

void WatchNetworkManager::setupAP() {
  Serial.println("Preparing for AP Mode...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);

  apMode = true;
  WiFi.mode(WIFI_AP);
  delay(100);

  String apSSID = "LC200-Watch-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  bool success = WiFi.softAP(apSSID.c_str(), "12345678"); // Default password

  if (!success) {
    Serial.println("AP Failed! Trying without password...");
    WiFi.softAP(apSSID.c_str());
  }

  if (MDNS.begin("artemLandCruiser")) {
    Serial.println("mDNS responder started: http://artemLandCruiser.local");
    MDNS.addService("http", "tcp", 80);
  }

  Serial.println("AP Started");
  Serial.print("SSID: ");
  Serial.println(apSSID);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
}

void WatchNetworkManager::update() {
  if (!apMode && WiFi.status() == WL_CONNECTED) {
    timeClient->update();

    if (millis() - lastWeatherUpdate > WEATHER_UPDATE_INTERVAL_MS ||
        lastWeatherUpdate == 0) {
      updateWeather();
    }
  } else if (!apMode) {
    // Попытка переподключения если связь потеряна (раз в 30 секунд не блокируя
    // поток)
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt > 30000) {
      lastReconnectAttempt = millis();
      Serial.println("Connection lost. Reconnecting...");
      WiFi.disconnect();
      WiFi.begin();
    }
  }
}

String WatchNetworkManager::getFormattedTime() {
  return timeClient->getFormattedTime();
}

int WatchNetworkManager::getHour() { return timeClient->getHours(); }

int WatchNetworkManager::getMinute() { return timeClient->getMinutes(); }

int WatchNetworkManager::getSecond() { return timeClient->getSeconds(); }

int WatchNetworkManager::getYear() {
  time_t rawtime = timeClient->getEpochTime();
  struct tm *ti;
  ti = localtime(&rawtime);
  return ti->tm_year + 1900;
}

float WatchNetworkManager::getTemperature() { return currentTemp; }

String WatchNetworkManager::getWeatherCondition() { return currentCondition; }

String WatchNetworkManager::getWeatherIcon() { return currentIcon; }

bool WatchNetworkManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

bool WatchNetworkManager::isAPMode() { return apMode; }

String WatchNetworkManager::getApSSID() {
  return "LC200-Watch-" + String((uint32_t)ESP.getEfuseMac(), HEX);
}

String WatchNetworkManager::getApPassword() { return "12345678"; }

String WatchNetworkManager::getIpAddress() {
  if (apMode)
    return WiFi.softAPIP().toString();
  return WiFi.localIP().toString();
}

void WatchNetworkManager::saveWiFiCredentials(String ssid, String pass) {
  xSemaphoreTake(systemMutex, portMAX_DELAY);
  preferences.begin("watch-config", false);
  preferences.putString("ssid", ssid);
  preferences.putString("pass", pass);
  preferences.end();
  xSemaphoreGive(systemMutex);
  Serial.println("Credentials saved. Please reboot.");
}

void WatchNetworkManager::saveLocalizationSettings(int timezoneOffset,
                                                   String city) {
  timeOffset = timezoneOffset;
  weatherCity = city;

  xSemaphoreTake(systemMutex, portMAX_DELAY);
  preferences.begin("watch-config", false);
  preferences.putInt("timeOffset", timezoneOffset);
  preferences.putString("city", city);
  preferences.end();
  xSemaphoreGive(systemMutex);

  // Update NTP client and Weather if connected
  if (!apMode && WiFi.status() == WL_CONNECTED) {
    timeClient->setTimeOffset(timeOffset * 3600);
    timeClient->forceUpdate();
    updateWeather(); // Принудительно обновляем погоду для нового города
  }
  Serial.println("Localization settings saved.");
}

void WatchNetworkManager::saveAlarm(int hour, int minute, int soundId,
                                    bool enabled) {
  alarmHour = hour;
  alarmMinute = minute;
  alarmSoundId = soundId;
  alarmEnabled = enabled;

  xSemaphoreTake(systemMutex, portMAX_DELAY);
  preferences.begin("watch-config", false);
  preferences.putInt("alarmHour", hour);
  preferences.putInt("alarmMinute", minute);
  preferences.putInt("alarmSound", soundId);
  preferences.putBool("alarmEnabled", enabled);
  preferences.end();
  xSemaphoreGive(systemMutex);

  Serial.printf("Alarm saved: %02d:%02d, Sound: %d, Enabled: %d\n", hour,
                minute, soundId, enabled);
}

void WatchNetworkManager::saveAlarmVolume(int volume) {
  alarmVolume = volume;
  xSemaphoreTake(systemMutex, portMAX_DELAY);
  preferences.begin("watch-config", false);
  preferences.putInt("alarmVol", volume);
  preferences.end();
  xSemaphoreGive(systemMutex);
  Serial.printf("Alarm volume saved: %d\n", volume);
}

bool WatchNetworkManager::checkAlarmTrigger() {
  if (!alarmEnabled || apMode || WiFi.status() != WL_CONNECTED)
    return false;

  int currentH = getHour();
  int currentM = getMinute();

  // Trigger if time matches and we haven't triggered this minute yet
  if (currentH == alarmHour && currentM == alarmMinute) {
    if (lastTriggerMinute != currentM) {
      lastTriggerMinute = currentM;
      return true;
    }
  } else {
    // Reset lastTriggerMinute if we moved past the alarm minute
    if (lastTriggerMinute != -1)
      lastTriggerMinute = -1;
  }

  return false;
}

void WatchNetworkManager::resetAlarmTrigger() {
  // This could also disable the alarm if it's not a repeating one
  // For now, we just let the logic in checkAlarmTrigger handle the per-minute
  // block
}

int WatchNetworkManager::getTimeZoneOffset() { return timeOffset; }

String WatchNetworkManager::getWeatherCity() { return weatherCity; }

void WatchNetworkManager::updateWeather() {
  if (WiFi.status() != WL_CONNECTED)
    return;

  HTTPClient http;
  // Окружаем город кавычками или проверяем пробелы (лучше кодировать, если
  // будут сложные названия)
  String url =
      "http://api.openweathermap.org/data/2.5/weather?q=" + weatherCity +
      "&units=metric&appid=" + String(WEATHER_API_KEY) + "&lang=ru";

  lastWeatherUpdate =
      millis(); // Сбрасываем таймер здесь, чтобы избежать циклов при ошибках
  Serial.print("Updating weather for: ");
  Serial.println(weatherCity);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(2048); // Увеличим буфер для надежности
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      if (doc.containsKey("main")) {
        currentTemp = doc["main"]["temp"].as<float>();
        const char *desc = doc["weather"][0]["main"];
        currentCondition = String(desc);
        const char *icon = doc["weather"][0]["icon"];
        currentIcon = String(icon);
        Serial.printf("Weather updated: %.1f C, %s\n", currentTemp, desc);
      } else {
        Serial.println("Error: No 'main' in JSON");
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
