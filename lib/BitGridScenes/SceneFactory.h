#pragma once

#include "IScene.h"
#include <Playlist.h>
#include <LedMatrix.h>

namespace BitGrid {
namespace Scenes {

// Factory for creating scene instances from playlist data
class SceneFactory {
public:
    SceneFactory(HAL::LedMatrix& ledMatrix);
    
    // Create a scene from FX scene data
    // Returns nullptr if effect not supported
    IScene* createFXScene(const FXScene* fxScene);
    
private:
    HAL::LedMatrix& ledMatrix_;
    
    // Helper to parse color from hex string (#RRGGBB or RRGGBB)
    static uint32_t parseColor(const String& colorStr);
};

} // namespace Scenes
} // namespace BitGrid
