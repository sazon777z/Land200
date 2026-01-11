#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <Preferences.h>

class WatchNetworkManager {
public:
    WatchNetworkManager();
    void begin();
    void update(); // Handle NTP and Weather updates
    
    String getFormattedTime();
    int getHour();
    int getMinute();
    int getSecond();
    
    float getTemperature();
    String getWeatherCondition();
    String getWeatherIcon();
    
    bool isConnected();
    bool isApMode();
    String getApSSID();
    String getApPassword();
    String getIpAddress();
    
    // Settings getters (for UI sync)
    int getTimeZoneOffset();
    String getWeatherCity();

    // Settings setters
    void saveWiFiCredentials(String ssid, String pass);
    void saveLocalizationSettings(int timezoneOffset, String city);

private:
    WiFiUDP ntpUDP;
    NTPClient* timeClient;
    Preferences preferences;
    
    unsigned long lastWeatherUpdate;
    float currentTemp;
    String currentCondition;
    String currentIcon;
    
    // Localization
    int timeOffset;
    String weatherCity;
    
    bool apMode;
    
    void updateWeather();
    void setupAP();
};

#endif // NETWORK_MANAGER_H
