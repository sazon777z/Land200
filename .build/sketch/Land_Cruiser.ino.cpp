#line 1 "C:\\Users\\JZX\\Documents\\Arduino\\Land_Cruiser\\Land_Cruiser.ino"
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

namespace {
constexpr unsigned long kButtonDebounceMs = 40;
constexpr unsigned long kDoublePressWindowMs = 350;

void stopAlarmOutput() {
  audio.stop();
  led.setModeIdle();
  network.resetAlarmTrigger();
}
} // namespace

// Button Check (Non-blocking)
#line 36 "C:\\Users\\JZX\\Documents\\Arduino\\Land_Cruiser\\Land_Cruiser.ino"
void checkButton();
#line 96 "C:\\Users\\JZX\\Documents\\Arduino\\Land_Cruiser\\Land_Cruiser.ino"
void TaskDisplay(void *pvParameters);
#line 125 "C:\\Users\\JZX\\Documents\\Arduino\\Land_Cruiser\\Land_Cruiser.ino"
void TaskNetwork(void *pvParameters);
#line 148 "C:\\Users\\JZX\\Documents\\Arduino\\Land_Cruiser\\Land_Cruiser.ino"
void TaskWeb(void *pvParameters);
#line 159 "C:\\Users\\JZX\\Documents\\Arduino\\Land_Cruiser\\Land_Cruiser.ino"
void setup();
#line 197 "C:\\Users\\JZX\\Documents\\Arduino\\Land_Cruiser\\Land_Cruiser.ino"
void loop();
#line 36 "C:\\Users\\JZX\\Documents\\Arduino\\Land_Cruiser\\Land_Cruiser.ino"
void checkButton() {
  static bool lastReading = false;
  static bool stablePressed = false;
  static bool handledAlarmStop = false;
  static uint8_t clickCount = 0;
  static unsigned long lastTransitionMs = 0;
  static unsigned long firstClickAtMs = 0;

  const unsigned long now = millis();
  const bool currentReading = (digitalRead(PIN_BUTTON) == LOW); // Active Low

  if (currentReading != lastReading) {
    lastReading = currentReading;
    lastTransitionMs = now;
  }

  if (now - lastTransitionMs < kButtonDebounceMs) {
    return;
  }

  if (stablePressed != currentReading) {
    stablePressed = currentReading;

    if (stablePressed) {
      if (led.isAlarmActive()) {
        Serial.println("Button Pressed: Stopping active alarm");
        stopAlarmOutput();
        handledAlarmStop = true;
        clickCount = 0;
        firstClickAtMs = 0;
      }
      return;
    }

    if (handledAlarmStop) {
      handledAlarmStop = false;
      return;
    }

    if (clickCount == 0 || now - firstClickAtMs > kDoublePressWindowMs) {
      clickCount = 1;
      firstClickAtMs = now;
      return;
    }

    clickCount = 0;
    firstClickAtMs = 0;
    const bool enabled = network.toggleAlarmEnabled();
    Serial.printf("Button Double Press: Alarm %s\n",
                  enabled ? "enabled" : "disabled");
  }

  if (!stablePressed && clickCount == 1 &&
      now - firstClickAtMs > kDoublePressWindowMs) {
    clickCount = 0;
    firstClickAtMs = 0;
  }
}

// System Tasks
void TaskDisplay(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    const WatchStateSnapshot snapshot = network.getSnapshot();

    if (snapshot.apMode) {
      if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        display.drawAPInfo(snapshot.apSSID, snapshot.apPassword,
                           snapshot.ipAddress);
        xSemaphoreGive(displayMutex);
      }
      vTaskDelay(pdMS_TO_TICKS(5000));
    } else if (!snapshot.connected) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    } else {
      static bool firstRun = true;
      if (xSemaphoreTake(displayMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (firstRun) {
          display.clearMainSegments();
          firstRun = false;
        }
        display.drawDashboard(snapshot);
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
      const WatchStateSnapshot snapshot = network.getSnapshot();
      Serial.println("ALARM TRIGGERED!");
      audio.setVolume(snapshot.alarmVolume);
      audio.playAlarmSound(snapshot.alarmSoundId);

      // Use user-selected effects for car lights and underglow
      LedDriver::AlarmCarEffect carEff =
          (LedDriver::AlarmCarEffect)snapshot.alarmCarEffect;
      LedDriver::LedEffect ledEff =
          (LedDriver::LedEffect)snapshot.alarmLedEffect;
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

