# Execution Plan

## Phase 0 — Baseline skeleton ✅ COMPLETE
- Create core module structure under `/lib` as per ARCHITECTURE.
- Add logging module with levels and tags.
- Ensure `main.cpp` is thin and delegates to `App`.

Acceptance:
- Serial shows boot banner, build info, heap.
- LED shows a simple scene (solid colour) without SD involvement.

**Status:** Complete. All modules created, logging working, main.cpp is 17 lines, breathing magenta test scene renders at ~52 FPS.

## Phase 1 — SD subsystem ✅ COMPLETE
- Implement `SdCard` wrapper:
	- init SPI
	- mount SD
	- print card type/size
	- list root `/` directory (if present)
- Add health diagnostics (mount status, last error).

Acceptance:
- On boot, SD mount result is logged.
- If SD absent/fail, app continues with Error Scene.

**Status:** Complete. SD mounts successfully, logs card type/size, lists root directory. Graceful fallback if SD missing.

## Phase 1.5 — WiFi File Manager (not originally planned) ✅ COMPLETE
- Added WiFi station mode connection to local network.
- Implemented web-based file manager with Bootstrap + Vue 3 (CDN).
- Features:
	- Browse folders on SD card
	- Upload files to current directory
	- Create folders
	- Delete files
- Accessible via browser at device IP address.

Acceptance:
- WiFi connects on boot and logs IP address.
- Web UI allows full SD card management without opening enclosure.

**Status:** Complete. File uploads working, folder creation working, no need to remove SD card from tight enclosure.

## Phase 2 — Playlist loader (minimal)
- Decide initial playlist format:
	- JSON (preferred) OR plain text list
- Implement `PlaylistLoader` and models.
- Load `sd://playlist.json` and log parse results.

Acceptance:
- Logs: number of items, item types, any invalid entries.
- If playlist missing/invalid: fallback scene.

## Phase 3 — Content-driven playback loop
- Implement a simple player state machine:
	- load item
	- play for duration
	- transition
- Start with one content type:
	- Example: “built-in scene name + duration”
- Add scene registry/factory.

Acceptance:
- Playlist can switch between two scenes on a timer.
- Serial logs scene transitions and timing.

## Phase 4 — Add first real content type
Pick one:
- Frame sequences (binary/raw) OR
- Palette animations OR
- Simple image format conversion precomputed offline

Implement content reader in Content layer (not scenes).

Acceptance:
- At least one SD-backed animation plays.
- Read errors are handled gracefully.

## Phase 5 — Polish & robustness
- Add periodic metrics (every N seconds):
	- fps, free heap, SD ok
- Add watchdog-friendly loop behaviour (avoid long blocking reads).
- Optional: caching/buffering strategy.

Acceptance:
- Player runs for hours without crashing.
- Logs remain useful and not spammy (rate-limited).