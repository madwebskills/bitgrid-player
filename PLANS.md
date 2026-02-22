# Execution Plan

## Phase 0 — Baseline skeleton
- Create core module structure under `/lib` as per ARCHITECTURE.
- Add logging module with levels and tags.
- Ensure `main.cpp` is thin and delegates to `App`.

Acceptance:
- Serial shows boot banner, build info, heap.
- LED shows a simple scene (solid colour) without SD involvement.

## Phase 1 — SD subsystem
- Implement `SdCard` wrapper:
	- init SPI
	- mount SD
	- print card type/size
	- list root `/` directory (if present)
- Add health diagnostics (mount status, last error).

Acceptance:
- On boot, SD mount result is logged.
- If SD absent/fail, app continues with Error Scene.

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