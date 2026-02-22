#include "SceneError.h"

#include <FastLED.h>

namespace BitGrid {
namespace Scenes {

void SceneError::renderFrame() {
    // Flashing red error indication
    uint8_t level = (elapsedMs_ / 250) % 2 ? 64 : 4;
    CRGB color(level, 0, 0);
    matrix_.fill(color);
}

} // namespace Scenes
} // namespace BitGrid
