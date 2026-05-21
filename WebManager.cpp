#include "WebManager.h"
#include <Arduino.h>

namespace {
constexpr size_t kMaxAlarmBodyBytes = 256;
constexpr size_t kMaxWifiBodyBytes = 256;
constexpr size_t kMaxLocalizationBodyBytes = 256;
constexpr int kMaxLedEffect = static_cast<int>(LedDriver::ALARM);
constexpr int kMaxAlarmCarEffect = static_cast<int>(LedDriver::ACE_POLICE);
constexpr int kMaxTurnSignalMode = static_cast<int>(LedDriver::TS_HAZARD);
} // namespace

WebManager::WebManager(WatchNetworkManager *networkManager,
                       AudioDriver *audioDriver, LedDriver *ledDriver,
                       DisplayDriver *displayDriver)
    : server(80), net(networkManager), audio(audioDriver), led(ledDriver),
      display(displayDriver), restartPending(false), restartAtMs(0) {}

void WebManager::begin() {
  setupRoutes();
  server.begin();
}

void WebManager::handle() {
  server.handleClient();

  if (restartPending && static_cast<long>(millis() - restartAtMs) >= 0) {
    ESP.restart();
  }
}

void WebManager::scheduleRestart(unsigned long delayMs) {
  restartPending = true;
  restartAtMs = millis() + delayMs;
}

void WebManager::stopAlarmPlayback() {
  audio->stop();
  led->setModeIdle();
  net->resetAlarmTrigger();
}

void WebManager::setupRoutes() {
  // Serve Static Files directly from PROGMEM (Flash)
  server.on("/", HTTP_GET,
            [this]() { server.send_P(200, "text/html", index_html); });

  server.on("/index.html", HTTP_GET,
            [this]() { server.send_P(200, "text/html", index_html); });

  server.on("/style.css", HTTP_GET,
            [this]() { server.send_P(200, "text/css", style_css); });

  server.on("/script.js", HTTP_GET, [this]() {
    server.send_P(200, "application/javascript", script_js);
  });

  // API Status
  server.on("/api/status", HTTP_GET, [this]() {
    const WatchStateSnapshot snapshot = net->getSnapshot();
    StaticJsonDocument<512> doc;

    // Format temperature with 1 decimal place
    char tempStr[10];
    dtostrf(snapshot.temperature, 4, 1, tempStr);

    doc["time"] = snapshot.formattedTime;
    doc["temp"] = String(tempStr);
    doc["condition"] = snapshot.weatherCondition;
    doc["wifi_strength"] = snapshot.connected ? WiFi.RSSI() : 0;
    doc["tz"] = snapshot.timeZoneOffset;
    doc["city"] = snapshot.weatherCity;
    doc["led_effect"] = led->getCurrentEffect();
    doc["led_bright"] = led->getBrightness();
    doc["led_speed"] = led->getSpeed();
    doc["disp_bright"] = display->getBrightness();

    // Alarm info
    char alarmTime[6];
    sprintf(alarmTime, "%02d:%02d", snapshot.alarmHour, snapshot.alarmMinute);
    doc["alarm_time"] = String(alarmTime);
    doc["alarm_sound"] = snapshot.alarmSoundId;
    doc["alarm_volume"] = snapshot.alarmVolume;
    doc["alarm_car_eff"] = snapshot.alarmCarEffect;
    doc["alarm_led_eff"] = snapshot.alarmLedEffect;
    doc["alarm_enabled"] = snapshot.alarmEnabled;
    doc["is_ap"] = snapshot.apMode;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });

  // API Sound Test
  server.on("/api/sound/test", HTTP_GET, [this]() {
    if (server.hasArg("id")) {
      const WatchStateSnapshot snapshot = net->getSnapshot();
      int soundId = constrain(server.arg("id").toInt(), 1, 10);
      audio->setVolume(snapshot.alarmVolume);
      audio->playTrack(soundId);
    }
    server.send(200, "text/plain", "OK");
  });

  // API Sound Stop
  server.on("/api/sound/stop", HTTP_GET, [this]() {
    stopAlarmPlayback();
    server.send(200, "text/plain", "OK");
  });

  // API Set Alarm
  server.on("/api/alarm/set", HTTP_POST, [this]() {
    if (server.hasArg("plain")) {
      const WatchStateSnapshot snapshot = net->getSnapshot();
      String body = server.arg("plain");
      if (body.length() > kMaxAlarmBodyBytes) {
        server.send(413, "text/plain", "Payload too large");
        return;
      }
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, body);
      if (error) {
        server.send(400, "text/plain", "Invalid JSON");
        return;
      }

      String time = doc["time"]; // Format "HH:MM"
      int sound = constrain(doc["sound"] | snapshot.alarmSoundId, 1, 10);
      int carEff = constrain(doc["carEff"] | snapshot.alarmCarEffect, 0,
                             kMaxAlarmCarEffect);
      int ledEff =
          constrain(doc["ledEff"] | snapshot.alarmLedEffect, 0, kMaxLedEffect);
      int volume = constrain(doc["volume"] | snapshot.alarmVolume, 0, 30);
      bool enabled = doc["enabled"] | snapshot.alarmEnabled;

      int h = 0, m = 0;
      if (time.length() == 5) {
        h = time.substring(0, 2).toInt();
        m = time.substring(3, 5).toInt();
      }
      if (time.length() != 5 || time.charAt(2) != ':' || h < 0 || h > 23 ||
          m < 0 || m > 59) {
        server.send(400, "text/plain", "Invalid time");
        return;
      }

      net->saveAlarm(h, m, sound, carEff, ledEff, enabled);
      net->saveAlarmVolume(volume);
      audio->setVolume(volume);
      if (!enabled) {
        stopAlarmPlayback();
      }
      server.send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      server.send(400, "text/plain", "Bad Request");
    }
  });

  server.on("/api/alarm/enabled", HTTP_POST, [this]() {
    if (!server.hasArg("plain")) {
      server.send(400, "text/plain", "Bad Request");
      return;
    }

    String body = server.arg("plain");
    if (body.length() > 64) {
      server.send(413, "text/plain", "Payload too large");
      return;
    }

    DynamicJsonDocument doc(128);
    DeserializationError error = deserializeJson(doc, body);
    if (error || !doc.containsKey("enabled")) {
      server.send(400, "text/plain", "Invalid JSON");
      return;
    }

    const bool enabled = doc["enabled"].as<bool>();
    net->setAlarmEnabled(enabled);
    if (!enabled) {
      stopAlarmPlayback();
    }

    DynamicJsonDocument responseDoc(64);
    responseDoc["status"] = "success";
    responseDoc["alarm_enabled"] = enabled;
    String response;
    serializeJson(responseDoc, response);
    server.send(200, "application/json", response);
  });

  // API Settings Brightness (Display)
  server.on("/api/settings/disp_bright", HTTP_GET, [this]() {
    if (server.hasArg("val")) {
      int val = constrain(server.arg("val").toInt(), 0, 255);
      display->setBrightness(val);
      Serial.printf("Display: Set brightness to %d\n", val);
    }
    server.send(200, "text/plain", "OK");
  });

  // API Settings LED Brightness
  server.on("/api/settings/led_bright", HTTP_GET, [this]() {
    if (server.hasArg("val")) {
      int val = constrain(server.arg("val").toInt(), 0, 255);
      led->setBrightness(val);
    }
    server.send(200, "text/plain", "OK");
  });

  // API Settings LED Speed
  server.on("/api/settings/led_speed", HTTP_GET, [this]() {
    if (server.hasArg("val")) {
      int val = constrain(server.arg("val").toInt(), 1, 100);
      led->setSpeed(val);
    }
    server.send(200, "text/plain", "OK");
  });

  // API Settings Alarm Volume
  server.on("/api/settings/alarm_volume", HTTP_GET, [this]() {
    if (server.hasArg("val")) {
      int val = constrain(server.arg("val").toInt(), 0, 30);
      net->saveAlarmVolume(val);
      audio->setVolume(val);
    }
    server.send(200, "text/plain", "OK");
  });

  // API Settings LED Color (HEX)
  server.on("/api/settings/led_color", HTTP_GET, [this]() {
    if (server.hasArg("hex")) {
      String hex = server.arg("hex");
      if (hex.startsWith("#"))
        hex = hex.substring(1);
      if (hex.length() != 6) {
        server.send(400, "text/plain", "Invalid hex");
        return;
      }
      uint32_t color = strtoul(hex.c_str(), NULL, 16);
      led->setColor(color);
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Missing hex");
    }
  });

  // API Settings Car Lights
  server.on("/api/settings/car_light", HTTP_GET, [this]() {
    if (server.hasArg("front")) {
      led->setVehicleHeadlights(server.arg("front").toInt() != 0);
    }
    if (server.hasArg("rear")) {
      led->setVehicleTaillights(server.arg("rear").toInt() != 0);
    }
    server.send(200, "text/plain", "OK");
  });

  // API Settings Turn Signal
  server.on("/api/settings/turn_signal", HTTP_GET, [this]() {
    if (server.hasArg("mode")) {
      int mode = constrain(server.arg("mode").toInt(), 0, kMaxTurnSignalMode);
      led->setTurnSignal((LedDriver::TurnSignal)mode);
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Missing mode");
    }
  });

  // API High Beam Flash (3 быстрых моргания)
  server.on("/api/settings/car_light_flash", HTTP_GET, [this]() {
    led->flashHighBeam();
    server.send(200, "text/plain", "OK");
  });

  // API Settings WiFi Reset
  server.on("/api/settings/wifi_reset", HTTP_GET, [this]() {
    net->saveWiFiCredentials("", ""); // Clear credentials
    server.send(202, "text/plain", "Restart scheduled");
    scheduleRestart(500);
  });

  // API Settings WiFi
  server.on("/api/settings/wifi", HTTP_POST, [this]() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      if (body.length() > kMaxWifiBodyBytes) {
        server.send(413, "text/plain", "Payload too large");
        return;
      }
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, body);
      if (error) {
        server.send(400, "text/plain", "Invalid JSON");
        return;
      }
      String ssid = doc["ssid"];
      String pass = doc["pass"];

      if (ssid.length() > 0) {
        net->saveWiFiCredentials(ssid, pass);
      }
      server.send(200, "application/json", "{\"status\":\"saved\"}");
      scheduleRestart(500);
    } else {
      server.send(400, "text/plain", "Bad Request");
    }
  });

  // API Settings Localization
  server.on("/api/settings/loc", HTTP_POST, [this]() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      if (body.length() > kMaxLocalizationBodyBytes) {
        server.send(413, "text/plain", "Payload too large");
        return;
      }
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, body);
      if (error) {
        server.send(400, "text/plain", "Invalid JSON");
        return;
      }
      int timezone =
          constrain(doc["timezone"] | net->getTimeZoneOffset(), -12, 14);
      String city = doc["city"];
      if (city.length() == 0) {
        city = net->getWeatherCity();
      }

      net->saveLocalizationSettings(timezone, city);
      server.send(200, "application/json", "{\"status\":\"saved\"}");
    } else {
      server.send(400, "text/plain", "Bad Request");
    }
  });

  // API Settings LED
  server.on("/api/settings/led", HTTP_GET, [this]() {
    if (server.hasArg("eff")) {
      int eff = constrain(server.arg("eff").toInt(), 0, kMaxLedEffect);
      Serial.printf("API: LED set effect %d\n", eff);
      led->setEffect((LedDriver::LedEffect)eff);
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Missing eff");
    }
  });

  // API Reboot
  server.on("/api/system/reboot", HTTP_GET, [this]() {
    server.send(202, "text/plain", "Restart scheduled");
    scheduleRestart(100);
  });
}
