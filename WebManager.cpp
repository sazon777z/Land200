#include "WebManager.h"

WebManager::WebManager(WatchNetworkManager *networkManager,
                       AudioDriver *audioDriver, LedDriver *ledDriver,
                       DisplayDriver *displayDriver)
    : server(80), net(networkManager), audio(audioDriver), led(ledDriver),
      display(displayDriver) {}

void WebManager::begin() {
  setupRoutes();
  server.begin();
}

void WebManager::handle() { server.handleClient(); }

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
    DynamicJsonDocument doc(512);

    // Format temperature with 1 decimal place
    char tempStr[10];
    dtostrf(net->getTemperature(), 4, 1, tempStr);

    doc["time"] = net->getFormattedTime();
    doc["temp"] = String(tempStr);
    doc["condition"] = net->getWeatherCondition();
    doc["wifi_strength"] = WiFi.RSSI();
    doc["tz"] = net->getTimeZoneOffset();
    doc["city"] = net->getWeatherCity();

    // Alarm info
    char alarmTime[6];
    sprintf(alarmTime, "%02d:%02d", net->getAlarmHour(), net->getAlarmMinute());
    doc["alarm_time"] = String(alarmTime);
    doc["alarm_sound"] = net->getAlarmSoundId();
    doc["alarm_volume"] = net->getAlarmVolume();
    doc["alarm_enabled"] = net->isAlarmEnabled();
    doc["is_ap"] = net->isAPMode();

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });

  // API Sound Test
  server.on("/api/sound/test", HTTP_GET, [this]() {
    if (server.hasArg("id")) {
      int soundId = server.arg("id").toInt();
      audio->setVolume(net->getAlarmVolume());
      audio->playTrack(soundId);
    }
    server.send(200, "text/plain", "OK");
  });

  // API Sound Stop
  server.on("/api/sound/stop", HTTP_GET, [this]() {
    audio->stop();
    led->setModeIdle(); // Also stop alarm LED effect if active
    server.send(200, "text/plain", "OK");
  });

  // API Set Alarm
  server.on("/api/alarm/set", HTTP_POST, [this]() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, body);

      String time = doc["time"]; // Format "HH:MM"
      int sound = doc["sound"];
      bool enabled = doc["enabled"] | true; // Default to true if not sent

      int h = 0, m = 0;
      if (time.length() == 5) {
        h = time.substring(0, 2).toInt();
        m = time.substring(3, 5).toInt();
      }

      net->saveAlarm(h, m, sound, enabled);
      server.send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      server.send(400, "text/plain", "Bad Request");
    }
  });

  // API Settings Brightness (Display)
  server.on("/api/settings/disp_bright", HTTP_GET, [this]() {
    if (server.hasArg("val")) {
      int val = server.arg("val").toInt();
      display->setBrightness(val);
      Serial.printf("Display: Set brightness to %d\n", val);
    }
    server.send(200, "text/plain", "OK");
  });

  // API Settings LED Brightness
  server.on("/api/settings/led_bright", HTTP_GET, [this]() {
    if (server.hasArg("val")) {
      int val = server.arg("val").toInt();
      led->setBrightness(val);
    }
    server.send(200, "text/plain", "OK");
  });

  // API Settings LED Speed
  server.on("/api/settings/led_speed", HTTP_GET, [this]() {
    if (server.hasArg("val")) {
      int val = server.arg("val").toInt();
      led->setSpeed(val);
    }
    server.send(200, "text/plain", "OK");
  });

  // API Settings Alarm Volume
  server.on("/api/settings/alarm_volume", HTTP_GET, [this]() {
    if (server.hasArg("val")) {
      int val = server.arg("val").toInt();
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
      led->setVehicleHeadlights(server.arg("front").toInt());
    }
    if (server.hasArg("rear")) {
      led->setVehicleTaillights(server.arg("rear").toInt());
    }
    server.send(200, "text/plain", "OK");
  });

  // API Settings Turn Signal
  server.on("/api/settings/turn_signal", HTTP_GET, [this]() {
    if (server.hasArg("mode")) {
      int mode = server.arg("mode").toInt();
      led->setTurnSignal((LedDriver::TurnSignal)mode);
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Missing mode");
    }
  });

  // API Settings WiFi Reset
  server.on("/api/settings/wifi_reset", HTTP_GET, [this]() {
    net->saveWiFiCredentials("", ""); // Clear credentials
    server.send(200, "text/plain", "OK");
    delay(500);
    ESP.restart();
  });

  // API Settings WiFi
  server.on("/api/settings/wifi", HTTP_POST, [this]() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      DynamicJsonDocument doc(512);
      deserializeJson(doc, body);
      String ssid = doc["ssid"];
      String pass = doc["pass"];

      if (ssid.length() > 0) {
        net->saveWiFiCredentials(ssid, pass);
      }
      server.send(200, "application/json", "{\"status\":\"saved\"}");

      // Give time for the response to be sent before rebooting
      delay(500);
      ESP.restart();
    } else {
      server.send(400, "text/plain", "Bad Request");
    }
  });

  // API Settings Localization
  server.on("/api/settings/loc", HTTP_POST, [this]() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      DynamicJsonDocument doc(512);
      deserializeJson(doc, body);
      int timezone = doc["timezone"];
      String city = doc["city"];

      net->saveLocalizationSettings(timezone, city);
      server.send(200, "application/json", "{\"status\":\"saved\"}");
    } else {
      server.send(400, "text/plain", "Bad Request");
    }
  });

  // API Settings LED
  server.on("/api/settings/led", HTTP_GET, [this]() {
    if (server.hasArg("eff")) {
      int eff = server.arg("eff").toInt();
      led->setEffect((LedDriver::LedEffect)eff);
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Missing eff");
    }
  });

  // API Reboot
  server.on("/api/system/reboot", HTTP_GET, [this]() {
    server.send(200, "text/plain", "Rebooting...");
    delay(100);
    ESP.restart();
  });
}
