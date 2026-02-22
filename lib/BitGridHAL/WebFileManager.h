#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <SD.h>

namespace BitGrid {
namespace HAL {

class WebFileManager {
public:
    WebFileManager(uint16_t port = 80);
    
    bool begin();
    void tick();

private:
    WebServer server_;

    void handleRoot();
    void handleList();
    void handleUpload();
    void handleUploadComplete();
    void handleDelete();
    void handleCreateFolder();
    void handleNotFound();

    String listDirectory(const String& path);
};

} // namespace HAL
} // namespace BitGrid
