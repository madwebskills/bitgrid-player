#pragma once

#include "IScene.h"
#include "SceneFactory.h"
#include <Playlist.h>
#include <LedMatrix.h>

namespace BitGrid {

// Manages playlist playback state and scene transitions
class PlaybackManager {
public:
    PlaybackManager(HAL::LedMatrix& ledMatrix, Playlist* playlist);
    ~PlaybackManager();
    
    // Initialize playback (call after construction)
    void begin();
    
    // Update playback state and current scene
    // Returns true if scene changed
    bool tick(uint32_t dtMs);
    
    // Render current scene to LED matrix
    void renderFrame();
    
    // Get current scene (nullptr if no valid scene)
    Scenes::IScene* currentScene() const { return currentScene_; }
    
    // Get current scene index
    int currentSceneIndex() const { return currentSceneIndex_; }
    
private:
    HAL::LedMatrix& ledMatrix_;
    Playlist* playlist_;
    Scenes::SceneFactory sceneFactory_;
    
    // Current playback state
    int currentSceneIndex_ = 0;
    Scenes::IScene* currentScene_ = nullptr;
    uint32_t sceneStartMs_ = 0;
    uint32_t sceneElapsedMs_ = 0;
    int playCount_ = 0;  // For frames scenes: number of complete plays
    bool needSceneLoad_ = false;  // Deferred scene load flag
    int pendingSceneIndex_ = -1;  // Scene to load on next tick
    
    // Advance to next scene in playlist
    void advanceToNextScene();
    
    // Load and start a specific scene by index
    bool loadScene(int index);
    
    // Check if current scene should stop
    bool shouldStopCurrentScene() const;
    
    // Unload current scene
    void unloadCurrentScene();
};

} // namespace BitGrid
