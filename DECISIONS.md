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