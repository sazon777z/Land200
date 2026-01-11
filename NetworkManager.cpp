#include "NetworkManager.h"
#include <Preferences.h>
#include <ArduinoJson.h>
#include "config.h"

WatchNetworkManager::WatchNetworkManager() :
    timeClient(new NTPClient(ntpUDP, NTP_SERVER, GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC)),
    lastWeatherUpdate(0),
    currentTemp(0.0),
    currentCondition("--"),
    currentIcon(""),
    timeOffset(5),
    weatherCity("Almaty"),
    apMode(false)
{
}

void WatchNetworkManager::begin() {
    preferences.begin("watch-config", false);
    String ssid = preferences.getString("ssid", "");
    String pass = preferences.getString("pass", "");
    
    // Load localization settings
    timeOffset = preferences.getInt("timeOffset", 5);
    weatherCity = preferences.getString("city", "Almaty");
    
    if (ssid == "") {
        Serial.println("No saved WiFi. Starting AP.");
        setupAP();
        return;
    }
    
    WiFi.mode(WIFI_STA);
    
    // Настройка статического IP: 192.168.0.191
    IPAddress local_IP(192, 168, 0, 191);
    IPAddress gateway(192, 168, 0, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress primaryDNS(8, 8, 8, 8); // Google DNS для работы погоды
    
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
        Serial.println("STA Failed to configure Static IP");
    }
    
    WiFi.begin(ssid.c_str(), pass.c_str());
    
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) { // Wait up to 10 seconds
        delay(500);
        Serial.print(".");
        retries++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nCombined!");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        timeClient->setTimeOffset(timeOffset * 3600); // Convert hours to seconds
        timeClient->begin();
        apMode = false;
    } else {
        Serial.println("\nConnection failed. Starting AP.");
        setupAP();
    }
}

void WatchNetworkManager::setupAP() {
    apMode = true;
    WiFi.mode(WIFI_AP);
    
    // Configure Static IP
    IPAddress local_IP(192, 168, 0, 191);
    IPAddress gateway(192, 168, 0, 191);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    
    String apSSID = "LC200-Watch-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    WiFi.softAP(apSSID.c_str(), "12345678"); // Default password
    
    Serial.println("AP Started");
    Serial.print("SSID: "); Serial.println(apSSID);
    Serial.print("IP: "); Serial.println(WiFi.softAPIP());
}

void WatchNetworkManager::update() {
    if (!apMode && WiFi.status() == WL_CONNECTED) {
        timeClient->update();
        
        if (millis() - lastWeatherUpdate > WEATHER_UPDATE_INTERVAL_MS || lastWeatherUpdate == 0) {
            updateWeather();
            lastWeatherUpdate = millis();
        }
    }
}

String WatchNetworkManager::getFormattedTime() {
    return timeClient->getFormattedTime();
}

int WatchNetworkManager::getHour() {
    return timeClient->getHours();
}

int WatchNetworkManager::getMinute() {
    return timeClient->getMinutes();
}

int WatchNetworkManager::getSecond() {
    return timeClient->getSeconds();
}

float WatchNetworkManager::getTemperature() {
    return currentTemp;
}

String WatchNetworkManager::getWeatherCondition() {
    return currentCondition;
}

String WatchNetworkManager::getWeatherIcon() {
    return currentIcon;
}

bool WatchNetworkManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool WatchNetworkManager::isApMode() {
    return apMode;
}

String WatchNetworkManager::getApSSID() {
    return "LC200-Watch-" + String((uint32_t)ESP.getEfuseMac(), HEX);
}

String WatchNetworkManager::getApPassword() {
    return "12345678";
}

String WatchNetworkManager::getIpAddress() {
    if (apMode) return WiFi.softAPIP().toString();
    return WiFi.localIP().toString();
}

void WatchNetworkManager::saveWiFiCredentials(String ssid, String pass) {
    preferences.putString("ssid", ssid);
    preferences.putString("pass", pass);
    Serial.println("Credentials saved. Please reboot.");
}

void WatchNetworkManager::saveLocalizationSettings(int timezoneOffset, String city) {
    timeOffset = timezoneOffset;
    weatherCity = city;
    preferences.putInt("timeOffset", timezoneOffset);
    preferences.putString("city", city);
    
    // Update NTP client if connected
    if (!apMode && WiFi.status() == WL_CONNECTED) {
        timeClient->setTimeOffset(timeOffset * 3600);
        timeClient->forceUpdate();
    }
    Serial.println("Localization settings saved.");
}

int WatchNetworkManager::getTimeZoneOffset() {
    return timeOffset;
}

String WatchNetworkManager::getWeatherCity() {
    return weatherCity;
}

void WatchNetworkManager::updateWeather() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    HTTPClient http;
    // Окружаем город кавычками или проверяем пробелы (лучше кодировать, если будут сложные названия)
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + weatherCity + "&units=metric&appid=" + String(WEATHER_API_KEY) + "&lang=ru";
    
    Serial.print("Updating weather for: "); Serial.println(weatherCity);
    
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(2048); // Увеличим буфер для надежности
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            if (doc.containsKey("main")) {
                currentTemp = doc["main"]["temp"].as<float>();
                const char* desc = doc["weather"][0]["main"];
                currentCondition = String(desc);
                const char* icon = doc["weather"][0]["icon"];
                currentIcon = String(icon);
                Serial.printf("Weather updated: %.1f C, %s\n", currentTemp, desc);
            } else {
                Serial.println("Error: No 'main' in JSON");
            }
        } else {
            Serial.print("JSON Error: "); Serial.println(error.c_str());
        }
    } else {
        Serial.printf("HTTP Error: %d\n", httpCode);
    }
    
    http.end();
}
