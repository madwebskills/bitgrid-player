# BitGrid Player (ESP32-C3)

BitGrid Player runs on an ESP32-C3 Super Mini and drives a 24×24 WS2812 LED matrix from content stored on an SD card. This repository contains the firmware for the standalone "player" device.

At a high level the player:
- Boots, prints a detailed boot banner and hardware status over Serial.
- Mounts an SPI SD card and reports card/FS status.
- Renders scenes onto a logical 2D framebuffer and maps that to the physical LED chain.
- In later phases, loads `playlist.json` from SD and plays pre-rendered animations or FX scenes.

The code is structured for clarity and extension, with a very thin `src/main.cpp` and most logic in `/lib` modules.

## Project layout

- `src/main.cpp` — very small entrypoint (17 lines) that instantiates and drives the core `App`.
- `lib/BitGridCore/`  
  - `Config.h` — pins, matrix dimensions, WiFi credentials, and build-time constants.  
  - `Log.h/.cpp` — tagged, levelled Serial logging (DEBUG/INFO/WARN/ERROR).  
  - `App.h/.cpp` — high-level application orchestration (boot sequence, tick loop).
- `lib/BitGridHAL/`  
  - `LedMatrix.h/.cpp` — wrapper around FastLED for the 24×24 WS2812 matrix, includes verified tile mapping.  
  - `SdCard.h/.cpp` — SPI + SD mount, card info, and directory logging.  
  - `WifiManager.h/.cpp` — WiFi station mode connection to local network.  
  - `WebFileManager.h/.cpp` — HTTP server providing web-based SD card file management.
- `lib/BitGridScenes/`  
  - `IScene.h` — minimal scene interface (`begin`, `tick`, `renderFrame`).  
  - `SceneSolid.*` — simple built-in test scene (breathing magenta pattern).  
  - `SceneError.*` — flashing red error/fallback scene (ready but unused).
- `lib/BitGridContent/` — reserved for playlist and asset loading (Phase 2+).

For deeper architectural intent and the full execution plan, see:
- `PROJECT_BRIEF.md` — high-level goals and constraints.
- `ARCHITECTURE.md` — core/HAL/content/renderer/scenes design.
- `PLANS.md` — phased implementation plan for the player.
- `DATA_FORMATS.md` — **authoritative description of `playlist.json` and BGR1 `.bin` formats**.
- `DECISIONS.md` — recorded design decisions and trade-offs.

These documents are intentionally written to be AI-friendly; other assistants and agents should treat them as the source of truth when extending the player.

## Hardware

The current target is an ESP32-C3 Super Mini with:
- WS2812 LED matrix: 3×3 tiles of 8×8 pixels (24×24 total, 576 LEDs).  
  - Data pin: GPIO 10.
- SD card over SPI:  
  - SCK:  GPIO 4  
  - MISO: GPIO 5  
  - MOSI: GPIO 6  
  - CS:   GPIO 7

These values are centralised in `lib/BitGridCore/Config.h` so they can be tweaked in one place if the wiring changes.

## Building and running

This is a PlatformIO project using the Arduino framework on ESP32:

1. Open the project folder in VS Code with the PlatformIO extension installed.
2. Select the `esp32-c3-devkitm-1` environment (already defined in `platformio.ini`).
3. Build and upload the firmware.
4. Open the Serial Monitor at 115200 baud.

On boot you should see:
- A BitGrid boot banner with build info and chip/SDK details.
- LED matrix initialisation summary (dimensions, LED count, brightness).
- SD mount result and, if successful, basic card information and a root directory listing.
- WiFi connection attempt and, if successful, the device IP address.
- Web server start confirmation.

The LEDs will display a breathing magenta test scene at ~52 FPS, providing visual confirmation that the matrix is wired and mapped correctly.

## SD card and content

### Current state (Phase 1 complete)
- SD is mounted at boot and basic card info + root directory contents are logged.
- A missing or failed SD mount does not crash the device; the player continues with the test scene.
- **WiFi File Manager:** Access the device via web browser at the IP address shown in Serial logs.
  - Browse SD card folders
  - Upload files (playlist.json, .bin animations)
  - Create folders (e.g., `/anims/`)
  - Delete files
  - No need to physically remove SD card from enclosure!

### For full content-driven playback (Phase 2+)
- The SD card should contain a `playlist.json` at the root and an `anims/` folder with BGR1 `.bin` files.  
- The exact format of `playlist.json` and `.bin` files is defined in `DATA_FORMATS.md` and is the contract between the web playlist editor and this firmware.
- Current test setup: `playlist.json` and populated `/anims/` folder ready for playlist loader implementation.

## Serial logging

Logging is handled via `BitGridCore::Log` and is designed to be:
- Tagged by subsystem (e.g. `[APP]`, `[SD]`).
- Levelled (DEBUG/INFO/WARN/ERROR) with a compile-time minimum level.
- Easy to extend from new modules.

When debugging hardware or content issues, start by:
- Checking the boot banner for build info and chip/SDK version.
- Verifying SD mount status and card type/size lines.
- Watching for WARN/ERROR lines from the SD or scene subsystems.

## WiFi and web file manager

**Current WiFi credentials are set in** `lib/BitGridCore/Config.h`:
```cpp
constexpr const char* WIFI_SSID = "YourNetwork";
constexpr const char* WIFI_PASSWORD = "YourPassword";
```

Update these before building to match your local network. On successful connection, the Serial monitor will display:
```
[INFO][WiFi] Connected! IP address: 192.168.x.x
[INFO][WiFi] Access file manager at: http://192.168.x.x/
```

Open that URL in a browser to manage SD card contents without opening the enclosure.

## Extending the player

When adding new functionality, prefer:
- Keeping `src/main.cpp` thin — initialise the `App` and call `tick()` only.
- Adding new hardware wrappers under `lib/BitGridHAL/`.
- Adding new scenes under `lib/BitGridScenes/` implementing `IScene`.
- Implementing playlist and content loading under `lib/BitGridContent/` using `DATA_FORMATS.md` as the schema reference.

This structure is intended to make the code approachable for both humans and AI assistants when iterating on features like new FX scenes, content types, or robustness improvements.

## Current status

**Phases complete:** 0, 1, 1.5 (WiFi file manager)  
**Next milestone:** Phase 2 — Playlist loader and JSON parsing  
**Hardware:** Fully functional, all subsystems tested  
**Memory usage:** RAM 12.8% (41KB/327KB), Flash 64.6% (847KB/1.3MB)  
**Performance:** ~52 FPS rendering + web server handling
