#include "LedMatrix.h"

namespace BitGrid {
namespace HAL {

// Single global LED buffer owned by this translation unit
static CRGB s_leds[Config::NUM_LEDS];

bool LedMatrix::begin() {
    FastLED.addLeds<WS2812B, Config::LED_DATA_PIN, GRB>(s_leds, Config::NUM_LEDS);
    FastLED.setBrightness(Config::LED_BRIGHTNESS);
    clear();
    FastLED.show();
    return true;
}

void LedMatrix::clear() {
    fill_solid(s_leds, Config::NUM_LEDS, CRGB::Black);
}

void LedMatrix::fill(const CRGB &color) {
    fill_solid(s_leds, Config::NUM_LEDS, color);
}

void LedMatrix::setPixel(uint16_t x, uint16_t y, const CRGB &color) {
    if (x >= Config::WIDTH || y >= Config::HEIGHT) return;
    uint16_t idx = xyToIndex(x, y);
    if (idx < Config::NUM_LEDS) {
        s_leds[idx] = color;
    }
}

void LedMatrix::show() {
    FastLED.show();
}

uint16_t LedMatrix::xyToIndex(uint16_t x, uint16_t y) const {
    // Mapping: tiles row-major, pixels row-major, origin top-left, not serpentine
    const uint16_t tileX = x / Config::TILE_W;
    const uint16_t tileY = y / Config::TILE_H;
    const uint16_t localX = x % Config::TILE_W;
    const uint16_t localY = y % Config::TILE_H;

    const uint16_t tileIndex = tileY * Config::TILES_X + tileX;
    const uint16_t localIndex = localY * Config::TILE_W + localX;

    return tileIndex * (Config::TILE_W * Config::TILE_H) + localIndex;
}

} // namespace HAL
} // namespace BitGrid
