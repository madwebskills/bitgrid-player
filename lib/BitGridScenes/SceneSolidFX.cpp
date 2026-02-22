#include "SceneSolidFX.h"
#include <FastLED.h>

namespace BitGrid {
namespace Scenes {

SceneSolidFX::SceneSolidFX(HAL::LedMatrix& ledMatrix, uint32_t color)
    : ledMatrix_(ledMatrix), color_(color) {}

void SceneSolidFX::begin() {
    elapsedMs_ = 0;
}

void SceneSolidFX::tick(uint32_t dtMs) {
    elapsedMs_ += dtMs;
}

void SceneSolidFX::renderFrame() {
    // Extract RGB from color (stored as 0xRRGGBB)
    uint8_t r = (color_ >> 16) & 0xFF;
    uint8_t g = (color_ >> 8) & 0xFF;
    uint8_t b = color_ & 0xFF;
    
    // Add subtle breathing effect using beatsin8
    // Oscillates between 80% and 100% brightness
    uint8_t breath = beatsin8(10, 204, 255);
    
    r = (r * breath) >> 8;
    g = (g * breath) >> 8;
    b = (b * breath) >> 8;
    
    ledMatrix_.fill(CRGB(r, g, b));
}

} // namespace Scenes
} // namespace BitGrid
