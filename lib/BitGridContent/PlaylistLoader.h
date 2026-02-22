#pragma once

#include "Playlist.h"
#include <SD.h>
#include <ArduinoJson.h>

namespace BitGrid {

class PlaylistLoader {
public:
    // Load playlist from SD card at given path (default: "/playlist.json")
    // Returns nullptr on failure
    static Playlist* load(const char* path = "/playlist.json");
    
private:
    // Helper parsing functions
    static bool parseDisplay(JsonObject displayObj, DisplayConfig& display);
    static Scene* parseScene(JsonObject sceneObj);
    static void parseStopCondition(JsonObject stopObj, StopCondition& stop);
    static FramesScene* parseFramesScene(JsonObject sceneObj);
    static FXScene* parseFXScene(JsonObject sceneObj);
    static GotoScene* parseGotoScene(JsonObject sceneObj);
};

} // namespace BitGrid
