#include "App.h"

#include <Config.h>
#include <Log.h>
#include <IScene.h>
#include <SceneError.h>

namespace BitGrid {

static const char *TAG_APP = "APP";

App::App() : testScene_(ledMatrix_) {}

App::~App() {
    if (playback_) {
        delete playback_;
        playback_ = nullptr;
    }
    if (playlist_) {
        delete playlist_;
        playlist_ = nullptr;
    }
}

void App::begin() {
    Log::begin(115200);
    Log::setLevel(LogLevel::DEBUG);

    delay(200); // allow USB CDC to settle

    logBootBanner();

    if (!ledMatrix_.begin()) {
        Log::error(TAG_APP, "Failed to initialise LED matrix");
    } else {
        Log::info(TAG_APP, "LED matrix initialised: %ux%u, %u LEDs, brightness %u",
                  Config::WIDTH, Config::HEIGHT, Config::NUM_LEDS, Config::LED_BRIGHTNESS);
    }

    bool sdOk = sdCard_.begin();
    if (sdOk) {
        sdCard_.logRootDir(1);
        
        // Try to load playlist
        playlist_ = PlaylistLoader::load("/playlist.json");
        if (playlist_) {
            Log::info(TAG_APP, "Playlist loaded successfully: %d scenes", playlist_->sceneCount());
            
            // Apply brightness from playlist config
            if (playlist_->display.brightness > 0) {
                ledMatrix_.setBrightness(playlist_->display.brightness);
                Log::info(TAG_APP, "LED brightness set to %u", playlist_->display.brightness);
            }
            
            // Initialize playback manager
            playback_ = new PlaybackManager(ledMatrix_, playlist_);
            playback_->begin();
        } else {
            Log::warn(TAG_APP, "Failed to load playlist; using test scene");
        }
    } else {
        Log::warn(TAG_APP, "Continuing without SD card; fallback scene will be used");
    }
    
    // Initialize fallback test scene (used if no playlist)
    if (!playback_) {
        testScene_.begin();
    }
    
    // Start WiFi and web server
    bool wifiOk = wifi_.begin(Config::WIFI_SSID, Config::WIFI_PASSWORD);
    if (wifiOk) {
        // Set up playlist reload callback
        webServer_.begin([this]() { this->reloadPlaylist(); });
    } else {
        Log::warn(TAG_APP, "Continuing without WiFi; web file manager unavailable");
    }

    logHeapStats(TAG_APP);

    lastTickMs_ = millis();
    lastFpsLogMs_ = lastTickMs_;
}

void App::tick() {
    uint32_t now = millis();
    uint32_t dt = now - lastTickMs_;
    lastTickMs_ = now;

    // Service web server
    webServer_.tick();

    // Update and render scene (playlist or fallback)
    if (playback_) {
        bool sceneChanged = playback_->tick(dt);
        playback_->renderFrame();
        
        if (sceneChanged) {
            Log::debug(TAG_APP, "Scene transition, free heap=%u", ESP.getFreeHeap());
        }
    } else {
        // Fallback to test scene
        testScene_.tick(dt);
        testScene_.renderFrame();
    }
    
    ledMatrix_.show();

    frameCount_++;

    // Heartbeat disabled to reduce log spam
    // Uncomment below for debugging:
    // if (now - lastFpsLogMs_ >= 10000) {
    //     float fps = (frameCount_ * 1000.0f) / (now - lastFpsLogMs_);
    //     Log::info(TAG_APP, "Heartbeat: uptime=%lu ms, approx fps=%.1f, free heap=%u", 
    //               static_cast<unsigned long>(now), fps, ESP.getFreeHeap());
    //     frameCount_ = 0;
    //     lastFpsLogMs_ = now;
    // }
}

void App::reloadPlaylist() {
    Log::info(TAG_APP, "Reloading playlist from SD card...");
    
    // Clean up current playback
    if (playback_) {
        delete playback_;
        playback_ = nullptr;
    }
    if (playlist_) {
        delete playlist_;
        playlist_ = nullptr;
    }
    
    // Try to load new playlist
    playlist_ = PlaylistLoader::load("/playlist.json");
    if (playlist_) {
        Log::info(TAG_APP, "Playlist reloaded successfully: %d scenes", playlist_->sceneCount());
        
        // Apply brightness from playlist config
        if (playlist_->display.brightness > 0) {
            ledMatrix_.setBrightness(playlist_->display.brightness);
            Log::info(TAG_APP, "LED brightness set to %u", playlist_->display.brightness);
        }
        
        playback_ = new PlaybackManager(ledMatrix_, playlist_);
        playback_->begin();
    } else {
        Log::error(TAG_APP, "Failed to reload playlist; using test scene");
    }
    
    logHeapStats(TAG_APP);
}

void App::logBootBanner() {
    Log::info(TAG_APP, "-----------------------------------------------");
    Log::info(TAG_APP, " BitGrid Player starting");
    Log::info(TAG_APP, " Build: %s %s", __DATE__, __TIME__);
    Log::info(TAG_APP, " Chip: %s, SDK: %s", ESP.getChipModel(), ESP.getSdkVersion());
    Log::info(TAG_APP, "-----------------------------------------------");
}

void App::logHeapStats(const char *tag) {
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t minFreeHeap = ESP.getMinFreeHeap();
    Log::info(tag, "Heap: free=%u, min_free=%u", freeHeap, minFreeHeap);
}

} // namespace BitGrid
