#ifndef CONFIG_H
#define CONFIG_H

// --- Hardware Pin Definitions (ESP32-C3 Super Mini) ---

// Display (TZT 1.47" ST7789 172x320 IPS)
// Pinout: SDA->7, SCL->6, DC->5, RES->8, CS->4, BLK->10
#define PIN_LCD_SCK 6
#define PIN_LCD_MOSI 7
#define PIN_LCD_CS 4
#define PIN_LCD_DC 5
#define PIN_LCD_RST 8
#define PIN_LCD_BL 10

// Audio (DFPlayer Mini)
// Note: ESP32-C3 has limited UARTs. TX/RX might need remapping if
// HardwareSerial is used. We will use HardwareSerial1 or SoftwareSerial if
// needed. ESP32-C3 Defualt Serial0 is USB.
#define PIN_AUDIO_RX 21 // Connect to DFPlayer TX
#define PIN_AUDIO_TX 20 // Connect to DFPlayer RX (via 1k Resistor)

// LEDs
#define PIN_WS2812 2 // Addressable LED Strip (DIN)
#define NUM_LEDS 12  // Total LEDs: 8 Underglow + 2 Tail + 2 Head

// LED Ranges
#define LED_UNDERGLOW_START 0
#define LED_UNDERGLOW_COUNT 8
#define LED_UNDERGLOW_END (LED_UNDERGLOW_START + LED_UNDERGLOW_COUNT - 1)

#define LED_TAIL_START 8
#define LED_TAIL_COUNT 2
#define LED_TAIL_END (LED_TAIL_START + LED_TAIL_COUNT - 1)

#define LED_HEAD_START 10
#define LED_HEAD_COUNT 2
#define LED_HEAD_END (LED_HEAD_START + LED_HEAD_COUNT - 1)

// Button
#define PIN_BUTTON 3 // Input Pullup

// --- Network Configuration ---
#define WIFI_SSID "FREEDOM"
#define WIFI_PASS "Water3003"

#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 10800 // +3 UTC (Moscow) - adjustable in UI
#define DAYLIGHT_OFFSET_SEC 0

// --- Weather Configuration ---
// OpenWeatherMap API
#define WEATHER_API_KEY "19f540a199ffffdad05a380203eea9c4"
#define WEATHER_CITY "Moscow"
#define WEATHER_UPDATE_INTERVAL_MS 600000 // 10 minutes for better accuracy

// --- Display Settings ---
#define SCREEN_WIDTH 172
#define SCREEN_HEIGHT 320

#endif // CONFIG_H
