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

// Mutex for SPI Display access
SemaphoreHandle_t displayMutex;

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
      if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        display.drawAPInfo(network.getApSSID(), network.getApPassword(),
                           network.getIpAddress());
        xSemaphoreGive(displayMutex);
      }
      vTaskDelay(pdMS_TO_TICKS(5000));
    } else if (!network.isConnected()) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    } else {
      if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        display.drawClock(network.getHour(), network.getMinute(),
                          network.getSecond());
        display.drawWeather(network.getTemperature(),
                            network.getWeatherCondition(),
                            network.getWeatherIcon());
        xSemaphoreGive(displayMutex);
      }
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}

void TaskNetwork(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    network.update();
    audio.processQueue(); // Process any pending sound commands

    if (network.checkAlarmTrigger()) {
      Serial.println("ALARM TRIGGERED!");
      audio.setVolume(network.getAlarmVolume());
      audio.playAlarmSound(network.getAlarmSoundId());

      // Use user-selected effects for car lights and underglow
      LedDriver::AlarmCarEffect carEff =
          (LedDriver::AlarmCarEffect)network.getAlarmCarEffect();
      LedDriver::LedEffect ledEff =
          (LedDriver::LedEffect)network.getAlarmLedEffect();
      led.setModeAlarm(carEff, ledEff);
    }
    vTaskDelay(pdMS_TO_TICKS(100)); // More frequent check for queue/alarm
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

  // Initialize Mutexes
  displayMutex = xSemaphoreCreateMutex();

  // Initialize Drivers
  Serial.println("Initializing Drivers...");
  led.begin();

  xSemaphoreTake(displayMutex, portMAX_DELAY);
  display.begin();
  xSemaphoreGive(displayMutex);
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
