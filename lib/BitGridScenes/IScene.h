#pragma once

#include <Arduino.h>

namespace BitGrid {
namespace HAL { class LedMatrix; }
namespace Scenes {

class IScene {
public:
    virtual ~IScene() = default;

    virtual const char *name() const = 0;
    virtual void begin() = 0;
    virtual void tick(uint32_t dtMs) = 0;
    virtual void renderFrame() = 0;
    virtual bool shouldStop() const { return false; }
};

} // namespace Scenes
} // namespace BitGrid
