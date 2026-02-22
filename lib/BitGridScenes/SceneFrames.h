#pragma once

#include "IScene.h"
#include <LedMatrix.h>
#include <SD.h>

namespace BitGrid {
namespace Scenes {

// BGR1 file header structure (16 bytes, little-endian)
struct BGR1Header {
    char magic[4];         // "BGR1"
    uint8_t w;             // frame width
    uint8_t h;             // frame height
    uint8_t channels;      // currently 3 (RGB)
    uint8_t flags;         // currently 0
    uint16_t frame_count;  // number of frames
    uint16_t reserved;     // currently 0
    uint32_t data_offset;  // byte offset to frame data (currently 16)
};

// Scene for playing BGR1 binary frame animations from SD card
class SceneFrames : public IScene {
public:
    SceneFrames(HAL::LedMatrix& ledMatrix, const String& filePath, uint8_t fps,
                int stopSeconds, int stopPlays);
    ~SceneFrames();
    
    const char* name() const override { return "frames"; }
    void begin() override;
    void tick(uint32_t dtMs) override;
    void renderFrame() override;
    
    // Check if scene should stop (for PlaybackManager)
    bool shouldStop() const;
    
private:
    HAL::LedMatrix& ledMatrix_;
    String filePath_;
    uint8_t fps_;
    int stopSeconds_;  // -1 = not set
    int stopPlays_;    // -1 = not set
    
    // File state
    File file_;
    BGR1Header header_;
    bool fileValid_ = false;
    
    // Playback state
    uint32_t frameSize_ = 0;
    uint16_t currentFrame_ = 0;
    uint32_t elapsedMs_ = 0;
    uint32_t frameDurationMs_ = 0;
    uint32_t frameAccumMs_ = 0;
    int playCount_ = 0;
    
    // Frame buffer (reused for each frame)
    uint8_t* frameBuffer_ = nullptr;
    
    // Helpers
    bool openAndValidateFile();
    bool readFrame(uint16_t frameIndex);
    void closeFile();
};

} // namespace Scenes
} // namespace BitGrid
