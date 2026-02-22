# Key Decisions & Trade-offs

## Use SD as primary content source
Decision:
- Content is loaded from SD (SPI) rather than relying on Wi-Fi.

Why:
- Standalone appliance behaviour, predictable performance, no network dependency.

Trade-off:
- More wiring and physical constraints, but already proven working.

## Thin main.cpp, modules in /lib
Decision:
- `src/main.cpp` only orchestrates `App`.

Why:
- Readability, testability, and easier agent collaboration.

Trade-off:
- Slightly more boilerplate upfront.

## Scene API separated from Content API
Decision:
- Scenes do not directly read SD files.

Why:
- Keeps rendering logic independent from storage formats.
- Allows future sources (Wi-Fi, flash cache) without rewriting scenes.

Trade-off:
- Requires a Content layer and some indirection.

## Aggressive Serial diagnostics
Decision:
- Serial logs are first-class: boot summary, SD stats, transitions, errors, optional periodic health.

Why:
- Enclosure is tight; wiring faults happen; logs must guide troubleshooting quickly.

Trade-off:
- Must rate-limit logs and allow compile-time verbosity control.

## Start with simple playlist format
Decision:
- Begin with a minimal playlist format and loader module, then extend.

Why:
- Faster to get end-to-end playback running.

Trade-off:
- Possible format changes later, mitigated by isolating loader code.

## Add WiFi file manager (mid-project decision)
Decision:
- Added WiFi station mode + web-based SD file manager during Phase 1.

Why:
- Enclosure is very tight and wiring is fragile.
- Removing SD card repeatedly risks damaging connections.
- Web UI allows uploading playlist.json and .bin files without physical access.

Trade-off:
- Adds ~40KB RAM usage and ~520KB flash for WiFi + WebServer libraries.
- WiFi credentials are hardcoded in Config.h (acceptable for standalone appliance).
- Reduced loop FPS from ~1000 to ~52 due to web server tick overhead (still more than adequate).

## Disable periodic heartbeat logs
Decision:
- Heartbeat logs (FPS, heap, uptime) are disabled by default, can be uncommented for debugging.

Why:
- Web file manager usage generates enough useful logs.
- Continuous FPS logging clutters Serial monitor during normal operation.

Trade-off:
- Less visibility into runtime health, but can be re-enabled easily when troubleshooting.