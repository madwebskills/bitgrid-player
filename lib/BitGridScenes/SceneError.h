#pragma once

#include <IScene.h>
#include <LedMatrix.h>

namespace BitGrid {
namespace Scenes {

class SceneError : public IScene {
public:
    explicit SceneError(HAL::LedMatrix &matrix) : matrix_(matrix) {}

    const char *name() const override { return "error"; }
    void begin() override { elapsedMs_ = 0; }
    void tick(uint32_t dtMs) override { elapsedMs_ += dtMs; }
    void renderFrame() override;

private:
    HAL::LedMatrix &matrix_;
    uint32_t elapsedMs_ = 0;
};

} // namespace Scenes
} // namespace BitGrid
