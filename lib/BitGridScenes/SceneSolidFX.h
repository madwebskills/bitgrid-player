#pragma once

#include "IScene.h"
#include <LedMatrix.h>

namespace BitGrid {
namespace Scenes {

// FX scene for solid color fill
class SceneSolidFX : public IScene {
public:
    SceneSolidFX(HAL::LedMatrix& ledMatrix, uint32_t color);
    
    const char* name() const override { return "solid_fx"; }
    void begin() override;
    void tick(uint32_t dtMs) override;
    void renderFrame() override;
    
private:
    HAL::LedMatrix& ledMatrix_;
    uint32_t color_;
    uint32_t elapsedMs_ = 0;
};

} // namespace Scenes
} // namespace BitGrid
