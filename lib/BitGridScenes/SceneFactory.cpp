#include "SceneFactory.h"
#include "SceneSolidFX.h"
#include <Log.h>

namespace BitGrid {
namespace Scenes {

SceneFactory::SceneFactory(HAL::LedMatrix& ledMatrix)
    : ledMatrix_(ledMatrix) {}

IScene* SceneFactory::createFXScene(const FXScene* fxScene) {
    if (!fxScene) return nullptr;
    
    if (fxScene->effect == "solid") {
        // Get color parameter (default to white if missing)
        String colorStr = fxScene->getParam("colour", "#FFFFFF");
        if (colorStr.isEmpty()) {
            colorStr = fxScene->getParam("color", "#FFFFFF");  // Try US spelling
        }
        
        uint32_t color = parseColor(colorStr);
        Log::debug("SCNF", "Creating solid FX: color=0x%06X", color);
        
        return new SceneSolidFX(ledMatrix_, color);
    }
    
    Log::warn("SCNF", "Unsupported FX effect: %s", fxScene->effect.c_str());
    return nullptr;
}

uint32_t SceneFactory::parseColor(const String& colorStr) {
    if (colorStr.isEmpty()) {
        return 0xFFFFFF;  // Default to white
    }
    
    String hex = colorStr;
    
    // Remove leading # if present
    if (hex.startsWith("#")) {
        hex = hex.substring(1);
    }
    
    // Parse hex string
    char* endPtr;
    uint32_t color = strtoul(hex.c_str(), &endPtr, 16);
    
    // Validate that we parsed something
    if (endPtr == hex.c_str()) {
        Log::warn("SCNF", "Invalid color format: %s, using white", colorStr.c_str());
        return 0xFFFFFF;
    }
    
    return color & 0xFFFFFF;  // Mask to 24 bits
}

} // namespace Scenes
} // namespace BitGrid
