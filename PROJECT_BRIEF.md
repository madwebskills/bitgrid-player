# BitGrid Player — Project Brief

## Summary
BitGrid Player runs on an ESP32-C3 Super Mini, reads “content” from an SD card, and renders it onto a WS2812 LED matrix. It is designed to be reliable, debuggable via Serial logs, and easy to extend with new scenes/effects and new content types.

## Goals
- Boot reliably and report hardware status (pins, heap, SD mount, card info).
- Read content from SD and play it on the LED matrix.
- Clean separation of concerns:
	- Thin `src/main.cpp`
	- Core logic + modules in `/lib`
- Provide a clean API for:
	- Scenes/effects (render loop + lifecycle)
	- SD content access (files, manifests, playlists)
- Strong Serial debug output for troubleshooting:
	- Boot banner and build info
	- SD mount result + card type/size + filesystem stats
	- Scene transitions and errors
	- Optional periodic health metrics (fps, heap)

## Non-goals (for v1)
- Web UI for authoring playlists (handled externally).
- Real-time streaming content over Wi-Fi.
- OTA updates (though WiFi is available for future use).
- Complex 3D/physics visualisations.

## Features added beyond original scope
- **WiFi File Manager:** Web-based SD card management to avoid opening the tight enclosure.
  - Browse folders, upload files, create directories, delete files.
  - Accessible via browser at device IP address.
  - Uses Bootstrap 5 + Vue 3 (CDN) for clean, responsive UI.

## Constraints
- Target: ESP32-C3 Super Mini (Arduino framework via PlatformIO).
- WS2812 matrix rendering must be performant and non-blocking where possible.
- SD card is SPI-based and must tolerate common failure modes:
	- No card inserted
	- Mount fails
	- File missing/corrupt
- Enclosure is “squishy”: wiring issues are possible, so debug logs must be useful.

## Success criteria
- With an SD card inserted, the player mounts SD, loads a manifest/playlist, and plays content.
- Without SD (or on failure), it shows a clear fallback scene and prints actionable logs.
- Adding a new FX scene is straightforward (new class/module, register it, done).