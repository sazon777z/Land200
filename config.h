#ifndef CONFIG_H
#define CONFIG_H

// --- Hardware Pin Definitions (ESP32-C3 Super Mini) ---

// Display (ILI9341 - 2.4" TFT SPI)
#define PIN_LCD_SCK 6
#define PIN_LCD_MOSI 7
#define PIN_LCD_CS 4
#define PIN_LCD_DC 5
#define PIN_LCD_RST 8
#define PIN_LCD_BL 10  // PWM for backlight brightness

// Audio (DFPlayer Mini)
// Note: ESP32-C3 has limited UARTs. TX/RX might need remapping if HardwareSerial is used.
// We will use HardwareSerial1 or SoftwareSerial if needed.
// ESP32-C3 Defualt Serial0 is USB.
#define PIN_AUDIO_RX 21 // Connect to DFPlayer TX
#define PIN_AUDIO_TX 20 // Connect to DFPlayer RX (via 1k Resistor)

// LEDs
#define PIN_WS2812 2      // Addressable LED Strip (DIN)
#define NUM_LEDS 8        // Number of WS2812B LEDs (4 underglow + 4 vehicle)

#define PIN_FRONT_LEDS 0  // Front Headlights (PWM/Digital)
#define PIN_REAR_LEDS 1   // Rear Taillights (PWM/Digital)

// Button
#define PIN_BUTTON 3      // Input Pullup

// --- Network Configuration ---
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 10800   // +3 UTC (Moscow) - adjustable in UI
#define DAYLIGHT_OFFSET_SEC 0

// --- Weather Configuration ---
// OpenWeatherMap API
#define WEATHER_API_KEY "19f540a199ffffdad05a380203eea9c4"
#define WEATHER_CITY "Moscow"
#define WEATHER_UPDATE_INTERVAL_MS 600000 // 10 minutes for better accuracy

// --- Display Settings ---
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

#endif // CONFIG_H
