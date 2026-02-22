#include "PlaybackManager.h"
#include "SceneFrames.h"
#include <Log.h>

namespace BitGrid {

PlaybackManager::PlaybackManager(HAL::LedMatrix& ledMatrix, Playlist* playlist)
    : ledMatrix_(ledMatrix), playlist_(playlist), sceneFactory_(ledMatrix) {}

PlaybackManager::~PlaybackManager() {
    unloadCurrentScene();
}

void PlaybackManager::begin() {
    Log::info("PLBK", "Starting playback with %d scenes", playlist_->sceneCount());
    
    if (playlist_->sceneCount() == 0) {
        Log::error("PLBK", "Playlist has no scenes!");
        return;
    }
    
    // Load first scene
    loadScene(0);
}

bool PlaybackManager::tick(uint32_t dtMs) {
    // Handle deferred scene load (avoids stack overflow from rapid skipping)
    if (needSceneLoad_) {
        needSceneLoad_ = false;
        loadScene(pendingSceneIndex_);
        return true;
    }
    
    if (!currentScene_) {
        return false;  // No scene loaded
    }
    
    // Update scene elapsed time
    sceneElapsedMs_ += dtMs;
    
    // Update current scene
    currentScene_->tick(dtMs);
    
    // Check if we should advance to next scene
    if (shouldStopCurrentScene()) {
        advanceToNextScene();
        return true;  // Scene changed
    }
    
    return false;  // No change
}

void PlaybackManager::renderFrame() {
    if (currentScene_) {
        currentScene_->renderFrame();
    }
}

void PlaybackManager::advanceToNextScene() {
    int nextIndex = currentSceneIndex_ + 1;
    
    // Handle end of playlist - loop back to start
    if (nextIndex >= static_cast<int>(playlist_->sceneCount())) {
        nextIndex = 0;
        Log::info("PLBK", "Reached end of playlist, looping to start");
    }
    
    // Defer load to next tick to avoid stack overflow
    needSceneLoad_ = true;
    pendingSceneIndex_ = nextIndex;
}

bool PlaybackManager::loadScene(int index) {
    if (index < 0 || index >= static_cast<int>(playlist_->sceneCount())) {
        Log::error("PLBK", "Invalid scene index: %d", index);
        return false;
    }
    
    Scene* sceneData = playlist_->scenes[index];
    
    // Unload previous scene
    unloadCurrentScene();
    
    // Handle different scene types
    if (sceneData->type == "fx") {
        FXScene* fxScene = static_cast<FXScene*>(sceneData);
        currentScene_ = sceneFactory_.createFXScene(fxScene);
        
        if (currentScene_) {
            Log::info("PLBK", "Scene[%d]: FX '%s' effect='%s'", 
                index, sceneData->id.c_str(), fxScene->effect.c_str());
        } else {
            Log::error("PLBK", "Failed to create FX scene: %s", fxScene->effect.c_str());
            return false;
        }
        
    } else if (sceneData->type == "goto") {
        GotoScene* gotoScene = static_cast<GotoScene*>(sceneData);
        
        // Find target scene
        int targetIndex = -1;
        if (!gotoScene->targetId.isEmpty()) {
            targetIndex = playlist_->findSceneById(gotoScene->targetId);
            Log::debug("PLBK", "Goto by id '%s' -> index %d", gotoScene->targetId.c_str(), targetIndex);
        } else if (gotoScene->targetIndex >= 0) {
            targetIndex = gotoScene->targetIndex;
            Log::debug("PLBK", "Goto by index %d", targetIndex);
        }
        
        if (targetIndex >= 0 && targetIndex < static_cast<int>(playlist_->sceneCount())) {
            Log::info("PLBK", "Scene[%d]: Goto -> Scene[%d]", index, targetIndex);
            // Update current index and defer load to next tick
            currentSceneIndex_ = index;
            needSceneLoad_ = true;
            pendingSceneIndex_ = targetIndex;
            return true;
        } else {
            Log::error("PLBK", "Invalid goto target, advancing to next");
            currentSceneIndex_ = index;
            advanceToNextScene();
            return false;
        }
        
    } else if (sceneData->type == "frames") {
        FramesScene* framesScene = static_cast<FramesScene*>(sceneData);
        currentScene_ = sceneFactory_.createFramesScene(framesScene);
        
        if (currentScene_) {
            Log::info("PLBK", "Scene[%d]: Frames '%s' fps=%d file='%s'",
                index, sceneData->id.c_str(), framesScene->fps, framesScene->file.c_str());
        } else {
            Log::error("PLBK", "Failed to create frames scene: %s", framesScene->file.c_str());
            currentSceneIndex_ = index;
            advanceToNextScene();
            return false;
        }
        
    } else {
        Log::error("PLBK", "Unknown scene type: %s", sceneData->type.c_str());
        return false;
    }
    
    // Initialize scene
    currentSceneIndex_ = index;
    sceneStartMs_ = millis();
    sceneElapsedMs_ = 0;
    playCount_ = 0;
    
    if (currentScene_) {
        currentScene_->begin();
    }
    
    return true;
}

bool PlaybackManager::shouldStopCurrentScene() const {
    if (!currentScene_) {
        return false;
    }
    
    Scene* sceneData = playlist_->scenes[currentSceneIndex_];
    
    if (sceneData->type == "fx") {
        FXScene* fxScene = static_cast<FXScene*>(sceneData);
        
        // Check stop condition for FX scenes
        if (fxScene->stop.seconds > 0) {
            uint32_t stopTimeMs = fxScene->stop.seconds * 1000;
            if (sceneElapsedMs_ >= stopTimeMs) {
                Log::debug("PLBK", "FX scene stopping: elapsed=%u ms, stop=%d s",
                    sceneElapsedMs_, fxScene->stop.seconds);
                return true;
            }
        } else {
            // No stop condition - default to 5 seconds
            if (sceneElapsedMs_ >= 5000) {
                Log::debug("PLBK", "FX scene stopping: no stop condition, default 5s elapsed");
                return true;
            }
        }
        
    } else if (sceneData->type == "frames") {
        // Check if frames scene says it should stop
        if (currentScene_->shouldStop()) {
            Log::debug("PLBK", "Frames scene stopping");
            return true;
        }
    }
    
    return false;
}

void PlaybackManager::unloadCurrentScene() {
    if (currentScene_) {
        delete currentScene_;
        currentScene_ = nullptr;
    }
}

} // namespace BitGrid
