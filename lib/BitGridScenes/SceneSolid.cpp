#include "SceneSolid.h"

#include <FastLED.h>

namespace BitGrid {
namespace Scenes {

SceneSolid::SceneSolid(HAL::LedMatrix &matrix) : matrix_(matrix) {}

void SceneSolid::begin() {
    elapsedMs_ = 0;
}

void SceneSolid::tick(uint32_t dtMs) {
    elapsedMs_ += dtMs;
}

void SceneSolid::renderFrame() {
    // Simple breathing between two colours as a visible test pattern
    const uint8_t phase = (elapsedMs_ / 16) & 0xFF;
    const uint8_t level = beatsin8(10, 16, 64, 0, phase);
    const CRGB color(level, 0, level); // purple-ish

    matrix_.fill(color);
}

} // namespace Scenes
} // namespace BitGrid
