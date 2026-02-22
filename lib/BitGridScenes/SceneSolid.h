#pragma once

#include <IScene.h>
#include <LedMatrix.h>

namespace BitGrid {
namespace Scenes {

class SceneSolid : public IScene {
public:
    explicit SceneSolid(HAL::LedMatrix &matrix);

    const char *name() const override { return "solid_test"; }
    void begin() override;
    void tick(uint32_t dtMs) override;
    void renderFrame() override;

private:
    HAL::LedMatrix &matrix_;
    uint32_t elapsedMs_ = 0;
};

} // namespace Scenes
} // namespace BitGrid
