#pragma once

#include <Arduino.h>
#include <FastLED.h>

#include <Config.h>

namespace BitGrid {
namespace HAL {

class LedMatrix {
public:
    bool begin();

    uint16_t width() const { return Config::WIDTH; }
    uint16_t height() const { return Config::HEIGHT; }

    void clear();
    void fill(const CRGB &color);
    void setPixel(uint16_t x, uint16_t y, const CRGB &color);
    void show();
    
    // Dynamic brightness control (0-255)
    void setBrightness(uint8_t brightness);

private:
    uint16_t xyToIndex(uint16_t x, uint16_t y) const;
};

} // namespace HAL
} // namespace BitGrid
