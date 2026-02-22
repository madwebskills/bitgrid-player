#pragma once

#include <stdint.h>

namespace BitGrid {
namespace Config {

// SD card SPI pins (ESP32-C3 Super Mini, verified wiring)
constexpr uint8_t SD_SCK  = 4;
constexpr uint8_t SD_MISO = 5;
constexpr uint8_t SD_MOSI = 6;
constexpr uint8_t SD_CS   = 7;

// WS2812 LED matrix data pin
constexpr uint8_t LED_DATA_PIN = 10;

// Tile layout: 3x3 tiles of 8x8 => 24x24 logical display
constexpr uint8_t TILE_W  = 8;
constexpr uint8_t TILE_H  = 8;
constexpr uint8_t TILES_X = 3;
constexpr uint8_t TILES_Y = 3;

constexpr uint16_t WIDTH  = TILE_W * TILES_X;   // 24
constexpr uint16_t HEIGHT = TILE_H * TILES_Y;   // 24
constexpr uint16_t NUM_LEDS = WIDTH * HEIGHT;   // 576

// LED brightness cap (0-255)
constexpr uint8_t LED_BRIGHTNESS = 32;

// WiFi credentials (station mode - connect to existing network)
constexpr const char* WIFI_SSID = "MadAir";
constexpr const char* WIFI_PASSWORD = "b00b5b00b5";

// Web file manager port
constexpr uint16_t WEB_SERVER_PORT = 80;

} // namespace Config
} // namespace BitGrid
