#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <SD.h>
#include <functional>

namespace BitGrid {
namespace HAL {

class WebFileManager {
public:
    WebFileManager(uint16_t port = 80);
    
    // Optional callback for playlist reload
    using ReloadCallback = std::function<void()>;
    
    bool begin(ReloadCallback reloadCb = nullptr);
    void tick();

private:
    WebServer server_;
    ReloadCallback reloadCallback_;

    void handleRoot();
    void handleList();
    void handleUpload();
    void handleUploadComplete();
    void handleDelete();
    void handleCreateFolder();
    void handleReloadPlaylist();
    void handleNotFound();

    String listDirectory(const String& path);
};

} // namespace HAL
} // namespace BitGrid
