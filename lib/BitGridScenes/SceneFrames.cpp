#include "SceneFrames.h"
#include <Log.h>
#include <Config.h>
#include <FastLED.h>

namespace BitGrid {
namespace Scenes {

SceneFrames::SceneFrames(HAL::LedMatrix& ledMatrix, const String& filePath, 
                         uint8_t fps, int stopSeconds, int stopPlays)
    : ledMatrix_(ledMatrix), stopSeconds_(stopSeconds), stopPlays_(stopPlays) {
    
    // Ensure path starts with '/' for SD library
    if (filePath.startsWith("/")) {
        filePath_ = filePath;
    } else {
        filePath_ = "/" + filePath;
    }
    
    fps_ = (fps == 0) ? 12 : fps;  // Default fps
    frameDurationMs_ = 1000 / fps_;
}

SceneFrames::~SceneFrames() {
    closeFile();
    if (frameBuffer_) {
        delete[] frameBuffer_;
        frameBuffer_ = nullptr;
    }
}

void SceneFrames::begin() {
    elapsedMs_ = 0;
    currentFrame_ = 0;
    frameAccumMs_ = 0;
    playCount_ = 0;
    
    if (!openAndValidateFile()) {
        Log::error("FRAM", "Failed to open/validate: %s", filePath_.c_str());
        fileValid_ = false;
        return;
    }
    
    // Allocate frame buffer
    frameBuffer_ = new uint8_t[frameSize_];
    if (!frameBuffer_) {
        Log::error("FRAM", "Failed to allocate frame buffer (%u bytes)", frameSize_);
        fileValid_ = false;
        closeFile();
        return;
    }
    
    fileValid_ = true;
    Log::info("FRAM", "Ready: %dx%d, %d frames @ %d fps", 
        header_.w, header_.h, header_.frame_count, fps_);
}

void SceneFrames::tick(uint32_t dtMs) {
    if (!fileValid_) return;
    
    elapsedMs_ += dtMs;
    frameAccumMs_ += dtMs;
    
    // Advance frame based on fps timing
    while (frameAccumMs_ >= frameDurationMs_ && fileValid_) {
        frameAccumMs_ -= frameDurationMs_;
        
        currentFrame_++;
        if (currentFrame_ >= header_.frame_count) {
            currentFrame_ = 0;
            playCount_++;
        }
    }
}

void SceneFrames::renderFrame() {
    if (!fileValid_) {
        // Show error pattern (red flash)
        ledMatrix_.fill(CRGB::Red);
        return;
    }
    
    // Read current frame from SD
    if (!readFrame(currentFrame_)) {
        Log::warn("FRAM", "Failed to read frame %d", currentFrame_);
        ledMatrix_.fill(CRGB::Red);
        return;
    }
    
    // Map logical pixels to LED matrix
    // Frame buffer contains RGB in row-major order (top-left origin)
    for (uint8_t y = 0; y < header_.h; y++) {
        for (uint8_t x = 0; x < header_.w; x++) {
            // Calculate offset in frame buffer (logical row-major)
            uint32_t pixelOffset = (y * header_.w + x) * 3;
            
            // Read RGB from buffer
            uint8_t r = frameBuffer_[pixelOffset + 0];
            uint8_t g = frameBuffer_[pixelOffset + 1];
            uint8_t b = frameBuffer_[pixelOffset + 2];
            
            // Set pixel (FastLED handles RGB->GRB conversion)
            ledMatrix_.setPixel(x, y, CRGB(r, g, b));
        }
    }
}

bool SceneFrames::shouldStop() const {
    if (!fileValid_) return true;  // Stop if file invalid
    
    // Check stop.seconds
    if (stopSeconds_ > 0) {
        uint32_t stopTimeMs = stopSeconds_ * 1000;
        if (elapsedMs_ >= stopTimeMs) {
            return true;
        }
    }
    
    // Check stop.plays
    if (stopPlays_ > 0) {
        if (playCount_ >= stopPlays_) {
            return true;
        }
    }
    
    // Default: stop after one play if no stop condition set
    if (stopSeconds_ < 0 && stopPlays_ < 0) {
        if (playCount_ >= 1) {
            return true;
        }
    }
    
    return false;
}

bool SceneFrames::openAndValidateFile() {
    // Open file
    file_ = SD.open(filePath_.c_str(), FILE_READ);
    if (!file_) {
        Log::error("FRAM", "File not found: %s", filePath_.c_str());
        return false;
    }
    
    // Check file size (minimum: 16 byte header)
    size_t fileSize = file_.size();
    if (fileSize < 16) {
        Log::error("FRAM", "File too small: %u bytes", fileSize);
        closeFile();
        return false;
    }
    
    // Read header (16 bytes)
    if (file_.read((uint8_t*)&header_, 16) != 16) {
        Log::error("FRAM", "Failed to read header");
        closeFile();
        return false;
    }
    
    // Validate magic
    if (header_.magic[0] != 'B' || header_.magic[1] != 'G' ||
        header_.magic[2] != 'R' || header_.magic[3] != '1') {
        Log::error("FRAM", "Invalid magic: %.4s", header_.magic);
        closeFile();
        return false;
    }
    
    // Validate dimensions
    if (header_.w != Config::WIDTH || header_.h != Config::HEIGHT) {
        Log::error("FRAM", "Size mismatch: file=%dx%d, display=%dx%d",
            header_.w, header_.h, Config::WIDTH, Config::HEIGHT);
        closeFile();
        return false;
    }
    
    // Validate channels
    if (header_.channels != 3) {
        Log::error("FRAM", "Unsupported channels: %d (expected 3)", header_.channels);
        closeFile();
        return false;
    }
    
    // Validate data_offset
    if (header_.data_offset < 16) {
        Log::error("FRAM", "Invalid data_offset: %u", header_.data_offset);
        closeFile();
        return false;
    }
    
    // Validate frame_count
    if (header_.frame_count == 0) {
        Log::error("FRAM", "No frames in file");
        closeFile();
        return false;
    }
    
    // Calculate frame size
    frameSize_ = header_.w * header_.h * header_.channels;
    
    // Validate file size
    uint32_t expectedSize = header_.data_offset + (frameSize_ * header_.frame_count);
    if (fileSize < expectedSize) {
        Log::error("FRAM", "File truncated: %u bytes, expected %u", fileSize, expectedSize);
        closeFile();
        return false;
    }
    
    return true;
}

bool SceneFrames::readFrame(uint16_t frameIndex) {
    if (!file_ || frameIndex >= header_.frame_count) {
        return false;
    }
    
    // Seek to frame position
    uint32_t frameOffset = header_.data_offset + (frameIndex * frameSize_);
    if (!file_.seek(frameOffset)) {
        Log::error("FRAM", "Seek failed: offset=%u", frameOffset);
        return false;
    }
    
    // Read frame data
    size_t bytesRead = file_.read(frameBuffer_, frameSize_);
    if (bytesRead != frameSize_) {
        Log::error("FRAM", "Read failed: got %u bytes, expected %u", bytesRead, frameSize_);
        return false;
    }
    
    return true;
}

void SceneFrames::closeFile() {
    if (file_) {
        file_.close();
    }
}

} // namespace Scenes
} // namespace BitGrid
