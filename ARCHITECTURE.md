# Architecture

## Overview
The player is built around a small core loop with modular subsystems:

- Core
	- App orchestration, state machine, tick timing
- HAL (hardware abstraction)
	- LED output wrapper
	- SD wrapper
	- Time/timers
- Content
	- Manifest/playlist parsing
	- Asset loading (frames, palettes, images, etc.)
- Rendering
	- Framebuffer + matrix mapping
	- Present to LEDs
- Scenes
	- Pluggable effects / content-driven renderers

`src/main.cpp` should only:
- Initialise subsystems
- Print boot diagnostics
- Enter a `tick()` loop

## Current repo layout (as implemented)
- `src/main.cpp` — thin entrypoint (17 lines)
- `/lib/BitGridCore/`
	- `App.h/.cpp` — high-level orchestration, boot sequence, tick loop
	- `Log.h/.cpp` — tagged, levelled logging (DEBUG/INFO/WARN/ERROR)
	- `Config.h` — pins, matrix dims, WiFi credentials, compile-time config
- `/lib/BitGridHAL/`
	- `LedMatrix.h/.cpp` — wraps FastLED, includes tile mapping (24×24, 3×3 tiles of 8×8)
	- `SdCard.h/.cpp` — SPI init, mount, card info, directory listing
	- `WifiManager.h/.cpp` — WiFi station mode connection
	- `WebFileManager.h/.cpp` — HTTP server for SD card file management
- `/lib/BitGridContent/`
	- `Playlist.h` — playlist model (DisplayConfig, Scene types: Frames/FX/Goto, StopCondition)
	- `PlaylistLoader.h/.cpp` — JSON parsing with ArduinoJson, validates structure
	- `PlaybackManager.h/.cpp` — state machine for playlist playback, scene transitions, timing
- `/lib/BitGridScenes/`
	- `IScene.h` — interface (`begin`, `tick`, `renderFrame`, `shouldStop`)
	- `SceneSolid.h/.cpp` — breathing test scene (fallback when no playlist)
	- `SceneError.h/.cpp` — fallback scene (flashing red, ready but unused)
	- `SceneSolidFX.h/.cpp` — "solid" FX effect with color parameter and breathing
	- `SceneFrames.h/.cpp` — BGR1 binary animation streaming from SD card
	- `SceneFactory.h/.cpp` — dynamic scene creation from playlist FX data, color parsing

## Key interfaces

### Logging
- Levels: DEBUG/INFO/WARN/ERROR
- Compile-time switch for verbose logging
- Include component tag: `[SD]`, `[LED]`, `[APP]`, etc.
- Common helpers:
	- `Log::info(tag, fmt, ...)`
	- `Log::error(tag, fmt, ...)`

### SD Card
Responsibilities:
- SPI init + mount
- Card metadata (type, size)
- FS stats (used/free)
- File operations:
	- exists, open, read, list
- Return structured error info (not just bool)

### LED Matrix
Responsibilities:
- Initialise LED library
- Brightness limiting (configurable)
- Framebuffer abstraction (eg RGB888 or RGB565)
- Matrix mapping (x/y -> index)
- `present()` is the only “show” boundary

### Scenes
Lifecycle:
- `begin(ctx)`
- `tick(ctx, dt_ms)`
- `render(ctx)` (or render within tick, but keep consistent)
- `end(ctx)`

A Scene should not talk directly to SD; it requests assets via Content layer (or is provided prepared data).

## Data formats (v1)
Start simple:
- `sd:/playlist.json`
- Content paths relative to `sd:/`, eg. `sd:/anims/*.bin`

We can evolve our binary format later, but keep loader isolated so formats don’t infect rendering code.
### BGR1 Format (implemented)
Binary frame animation format for streaming from SD:
- **Header (16 bytes):**
  - `magic[4]`: "BGR1" (ASCII)
  - `w, h, channels, flags`: 4 × uint8 (dimensions, channels=3)
  - `frame_count`: uint16 LE
  - `reserved`: uint16 (unused)
  - `data_offset`: uint32 LE (offset to first frame, typically 16)
- **Frame data:**
  - RGB row-major, top-left origin
  - 3 bytes per pixel (R, G, B)
  - Frame size: `w * h * 3` bytes
  - Frames stored sequentially after header

**Implementation notes:**
- Streaming approach: seek to frame offset, read into reusable buffer (1728 bytes for 24×24)
- Validation: checks magic, dimensions match display, channels=3, frame_count>0
- Stop conditions: `stop.seconds` (time-based) or `stop.plays` (loop count), defaults to 1 play
- See `DATA_FORMATS.md` for full specification
## Error handling
- If SD mount fails: log + activate Error Scene (clear visible signal).
- If playlist missing: log + fallback to a default scene.
- If asset read fails mid-play: log + skip to next item.