#pragma once

#include <Arduino.h>

#include <LedMatrix.h>
#include <SdCard.h>
#include <WifiManager.h>
#include <WebFileManager.h>
#include <SceneSolid.h>
#include <Playlist.h>
#include <PlaylistLoader.h>
#include <PlaybackManager.h>

namespace BitGrid {

class App {
public:
    App();
    ~App();

    void begin();
    void tick();

private:
    HAL::LedMatrix ledMatrix_;
    HAL::SdCard sdCard_;
    HAL::WifiManager wifi_;
    HAL::WebFileManager webServer_;
    Scenes::SceneSolid testScene_;
    Playlist* playlist_ = nullptr;
    PlaybackManager* playback_ = nullptr;

    uint32_t lastTickMs_ = 0;
    uint32_t frameCount_ = 0;
    uint32_t lastFpsLogMs_ = 0;

    void logBootBanner();
    void logHeapStats(const char *tag);
};

} // namespace BitGrid
