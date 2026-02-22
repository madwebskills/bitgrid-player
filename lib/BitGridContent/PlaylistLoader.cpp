#include "PlaylistLoader.h"
#include <Log.h>
#include <ArduinoJson.h>

namespace BitGrid {

Playlist* PlaylistLoader::load(const char* path) {
    Log::info("PLST", "Loading playlist from: %s", path);
    
    // Open file
    File file = SD.open(path, FILE_READ);
    if (!file) {
        Log::error("PLST", "Failed to open playlist file: %s", path);
        return nullptr;
    }
    
    size_t fileSize = file.size();
    Log::debug("PLST", "Playlist file size: %u bytes", fileSize);
    
    // Allocate JSON document (adjust size if needed for larger playlists)
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Log::error("PLST", "JSON parse failed: %s", error.c_str());
        return nullptr;
    }
    
    // Create playlist object
    Playlist* playlist = new Playlist();
    
    // Parse version
    playlist->version = doc["version"] | 1;
    Log::debug("PLST", "Playlist version: %d", playlist->version);
    
    // Parse display config
    if (doc.containsKey("display")) {
        if (!parseDisplay(doc["display"].as<JsonObject>(), playlist->display)) {
            Log::warn("PLST", "Display config invalid, using defaults");
        }
    } else {
        Log::warn("PLST", "No display config found, using defaults");
    }
    
    Log::info("PLST", "Display: %dx%d, brightness=%d, tiles=%dx%d (%dx%d each)",
        playlist->display.w, playlist->display.h, playlist->display.brightness,
        playlist->display.tiles.x, playlist->display.tiles.y,
        playlist->display.tiles.w, playlist->display.tiles.h);
    
    // Parse scenes
    if (doc.containsKey("scenes") && doc["scenes"].is<JsonArray>()) {
        JsonArray scenesArray = doc["scenes"].as<JsonArray>();
        
        for (JsonObject sceneObj : scenesArray) {
            Scene* scene = parseScene(sceneObj);
            if (scene) {
                playlist->scenes.push_back(scene);
                Log::debug("PLST", "  Scene[%d]: id='%s' type='%s'",
                    playlist->scenes.size() - 1, scene->id.c_str(), scene->type.c_str());
            } else {
                Log::warn("PLST", "  Skipped invalid scene at index %d", playlist->scenes.size());
            }
        }
        
        Log::info("PLST", "Loaded %d scenes successfully", playlist->sceneCount());
    } else {
        Log::warn("PLST", "No scenes array found in playlist");
    }
    
    if (playlist->sceneCount() == 0) {
        Log::error("PLST", "Playlist has no valid scenes");
        delete playlist;
        return nullptr;
    }
    
    return playlist;
}

bool PlaylistLoader::parseDisplay(JsonObject displayObj, DisplayConfig& display) {
    // Parse basic dimensions
    display.w = displayObj["w"] | 24;
    display.h = displayObj["h"] | 24;
    display.brightness = displayObj["brightness"] | 32;
    
    // Parse tiles
    if (displayObj.containsKey("tiles")) {
        JsonObject tiles = displayObj["tiles"].as<JsonObject>();
        display.tiles.x = tiles["x"] | 3;
        display.tiles.y = tiles["y"] | 3;
        display.tiles.w = tiles["w"] | 8;
        display.tiles.h = tiles["h"] | 8;
    }
    
    // Parse mapping
    if (displayObj.containsKey("mapping")) {
        JsonObject mapping = displayObj["mapping"].as<JsonObject>();
        display.mapping.origin = mapping["origin"] | "top_left";
        display.mapping.tile_rotation = mapping["tile_rotation"] | 0;
        display.mapping.tile_order = mapping["tile_order"] | "row_major";
        display.mapping.tile_serpentine = mapping["tile_serpentine"] | false;
        display.mapping.pixel_order = mapping["pixel_order"] | "row_major";
        display.mapping.pixel_serpentine = mapping["pixel_serpentine"] | false;
    }
    
    return true;
}

Scene* PlaylistLoader::parseScene(JsonObject sceneObj) {
    if (!sceneObj.containsKey("type") || !sceneObj.containsKey("id")) {
        Log::warn("PLST", "Scene missing 'type' or 'id'");
        return nullptr;
    }
    
    String type = sceneObj["type"].as<String>();
    
    if (type == "frames") {
        return parseFramesScene(sceneObj);
    } else if (type == "fx") {
        return parseFXScene(sceneObj);
    } else if (type == "goto") {
        return parseGotoScene(sceneObj);
    } else {
        Log::warn("PLST", "Unknown scene type: %s", type.c_str());
        return nullptr;
    }
}

void PlaylistLoader::parseStopCondition(JsonObject stopObj, StopCondition& stop) {
    if (stopObj.isNull()) {
        return;  // No stop condition
    }
    
    stop.seconds = stopObj["seconds"] | -1;
    stop.plays = stopObj["plays"] | -1;
}

FramesScene* PlaylistLoader::parseFramesScene(JsonObject sceneObj) {
    FramesScene* scene = new FramesScene();
    scene->id = sceneObj["id"].as<String>();
    scene->fps = sceneObj["fps"] | 12;
    
    // Parse stop condition
    if (sceneObj.containsKey("stop")) {
        parseStopCondition(sceneObj["stop"].as<JsonObject>(), scene->stop);
    }
    
    // Parse file path - if not present, derive from ID
    if (sceneObj.containsKey("file")) {
        scene->file = sceneObj["file"].as<String>();
    } else {
        scene->file = "anims/" + scene->id + ".bin";
    }
    
    // Parse editor-only fields (store but don't use)
    if (sceneObj.containsKey("source")) {
        scene->source = sceneObj["source"].as<String>();
    }
    if (sceneObj.containsKey("compile")) {
        JsonObject compile = sceneObj["compile"].as<JsonObject>();
        scene->compile.fit = compile["fit"] | "";
        scene->compile.scale = compile["scale"] | "";
        scene->compile.dither = compile["dither"] | "";
    }
    
    Log::debug("PLST", "    frames: fps=%d file='%s'", scene->fps, scene->file.c_str());
    
    return scene;
}

FXScene* PlaylistLoader::parseFXScene(JsonObject sceneObj) {
    FXScene* scene = new FXScene();
    scene->id = sceneObj["id"].as<String>();
    scene->effect = sceneObj["effect"].as<String>();
    
    if (scene->effect.isEmpty()) {
        Log::warn("PLST", "FX scene missing 'effect' field");
        delete scene;
        return nullptr;
    }
    
    // Parse stop condition
    if (sceneObj.containsKey("stop")) {
        parseStopCondition(sceneObj["stop"].as<JsonObject>(), scene->stop);
    }
    
    // Parse params as key-value pairs
    if (sceneObj.containsKey("params")) {
        JsonObject params = sceneObj["params"].as<JsonObject>();
        for (JsonPair kv : params) {
            String key = kv.key().c_str();
            String value = kv.value().as<String>();
            scene->params.push_back({key, value});
        }
    }
    
    Log::debug("PLST", "    fx: effect='%s' params=%d", scene->effect.c_str(), scene->params.size());
    
    return scene;
}

GotoScene* PlaylistLoader::parseGotoScene(JsonObject sceneObj) {
    GotoScene* scene = new GotoScene();
    scene->id = sceneObj["id"].as<String>();
    
    // Parse target
    if (sceneObj.containsKey("target")) {
        JsonObject target = sceneObj["target"].as<JsonObject>();
        
        if (target.containsKey("id")) {
            scene->targetId = target["id"].as<String>();
        }
        if (target.containsKey("index")) {
            scene->targetIndex = target["index"] | -1;
        }
    }
    
    if (scene->targetId.isEmpty() && scene->targetIndex < 0) {
        Log::warn("PLST", "Goto scene missing valid target");
        delete scene;
        return nullptr;
    }
    
    Log::debug("PLST", "    goto: targetId='%s' targetIndex=%d",
        scene->targetId.c_str(), scene->targetIndex);
    
    return scene;
}

} // namespace BitGrid
