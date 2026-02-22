#pragma once

#include <Arduino.h>
#include <vector>

namespace BitGrid {

// Display configuration from playlist
struct DisplayConfig {
    uint8_t w = 24;
    uint8_t h = 24;
    uint8_t brightness = 32;
    
    struct {
        uint8_t x = 3;  // tiles horizontally
        uint8_t y = 3;  // tiles vertically
        uint8_t w = 8;  // tile width
        uint8_t h = 8;  // tile height
    } tiles;
    
    struct {
        String origin = "top_left";
        uint16_t tile_rotation = 0;
        String tile_order = "row_major";
        bool tile_serpentine = false;
        String pixel_order = "row_major";
        bool pixel_serpentine = false;
    } mapping;
};

// Stop condition for scenes
struct StopCondition {
    int seconds = -1;  // -1 = not set
    int plays = -1;    // -1 = not set
    
    bool hasCondition() const {
        return seconds > 0 || plays > 0;
    }
};

// Base scene interface
struct Scene {
    String id;
    String type;  // "frames", "fx", "goto"
    
    virtual ~Scene() = default;
};

// Frames scene (pre-rendered animation from .bin file)
struct FramesScene : public Scene {
    uint8_t fps = 12;
    StopCondition stop;
    String file;  // path to .bin file (e.g., "anims/intro.bin")
    
    // Editor-only fields (we'll store but ignore)
    String source;
    struct {
        String fit;
        String scale;
        String dither;
    } compile;
    
    FramesScene() { type = "frames"; }
};

// FX scene (procedural effect)
struct FXScene : public Scene {
    String effect;  // "solid", "plasma", "fire", etc.
    StopCondition stop;
    std::vector<std::pair<String, String>> params;  // key-value params
    
    FXScene() { type = "fx"; }
    
    // Helper to get param value by key
    String getParam(const String& key, const String& defaultValue = "") const {
        for (const auto& p : params) {
            if (p.first == key) return p.second;
        }
        return defaultValue;
    }
};

// Goto scene (jump to another scene)
struct GotoScene : public Scene {
    String targetId;
    int targetIndex = -1;  // -1 = not set
    
    GotoScene() { type = "goto"; }
};

// Main playlist structure
class Playlist {
public:
    int version = 1;
    DisplayConfig display;
    std::vector<Scene*> scenes;
    
    ~Playlist() {
        clear();
    }
    
    void clear() {
        for (auto* scene : scenes) {
            delete scene;
        }
        scenes.clear();
    }
    
    size_t sceneCount() const { return scenes.size(); }
    
    // Find scene by ID (returns index, -1 if not found)
    int findSceneById(const String& id) const {
        for (size_t i = 0; i < scenes.size(); i++) {
            if (scenes[i]->id == id) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }
};

} // namespace BitGrid
