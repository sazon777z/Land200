#include "AudioDriver.h"
#include "DisplayDriver.h"
#include "LedDriver.h"
#include "NetworkManager.h"
#include "WebManager.h"
#include "config.h"
#include <Arduino.h>

// Global Driver Instances
DisplayDriver display;
AudioDriver audio;
LedDriver led;
WatchNetworkManager network;
WebManager web(&network, &audio, &led, &display);

// FreeRTOS Task Handles
TaskHandle_t TaskDisplayHandle;
TaskHandle_t TaskNetworkHandle;
TaskHandle_t TaskWebHandle;

// Button Check (Non-blocking)
void checkButton() {
  static bool btnPressed = false;
  static unsigned long lastDebounceTime = 0;

  bool currentReading = (digitalRead(PIN_BUTTON) == LOW); // Active Low

  if (currentReading && !btnPressed) {
    if (millis() - lastDebounceTime > 50) { // Debounce
      btnPressed = true;
      Serial.println("Button Pressed: Stopping Sound");
      audio.stop();
      led.setModeIdle();
      lastDebounceTime = millis();
    }
  } else if (!currentReading && btnPressed) {
    btnPressed = false;
    lastDebounceTime = millis();
  }
}

// System Tasks
void TaskDisplay(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    if (network.isAPMode()) {
      // Show AP Info and QR Code
      display.drawAPInfo(network.getApSSID(), network.getApPassword(),
                         network.getIpAddress());
      vTaskDelay(5000 / portTICK_PERIOD_MS); // Refresh less often
    } else if (!network.isConnected()) {
      // This state is briefly covered by initialization, but good for stability
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    } else {
      // Normal Operation
      display.drawClock(network.getHour(), network.getMinute(),
                        network.getSecond());
      display.drawWeather(network.getTemperature(),
                          network.getWeatherCondition(),
                          network.getWeatherIcon());
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

void TaskNetwork(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    network.update();

    // Check for alarm trigger
    if (network.checkAlarmTrigger()) {
      Serial.println("ALARM TRIGGERED!");
      audio.setVolume(network.getAlarmVolume());
      audio.playTrack(network.getAlarmSoundId());
      led.setModeAlarm();
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void TaskWeb(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    // Calling handleClient repeatedly.
    // This handles incoming HTTP requests.
    web.handle();
    // Small delay to yield to other tasks, but keep responsive
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void setup() {
  // Basic Serial for Debug
  Serial.begin(115200);

  // Initialize Button
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  // Initialize Drivers
  Serial.println("Initializing Drivers...");
  led.begin();
  display.begin();
  audio.begin();

  // Show connecting status on screen
  display.drawConnecting(WIFI_SSID);

  network.begin(); // Connects to WiFi (blocking up to 20s)

  // Initialize Web Server (after Network)
  web.begin();

  Serial.println("System Ready.");

  // Create FreeRTOS Tasks
  // Core 1 is usually app core, Core 0 is Wifi/Radio. ESP32-C3 is single core,
  // so just prioritizing.
  xTaskCreate(TaskDisplay, "DisplayTask", 4096, NULL, 1, &TaskDisplayHandle);
  xTaskCreate(TaskNetwork, "NetworkTask", 8192, NULL, 1, &TaskNetworkHandle);
  // Web task needs reasonable stack for file operations
  xTaskCreate(TaskWeb, "WebTask", 6144, NULL, 1, &TaskWebHandle);
}

void loop() {
  // Quick tasks in main loop (LED animations need high FPS)
  // Now loop is not blocked by Web or Button delays
  led.update();
  checkButton();
  delay(5); // Small delay to prevent watchdog starvation if led.update is very
            // fast
}
