# BitGrid Data Formats (v1)

## Big picture

BitGrid playback is driven by a single `playlist.json` on the SD card. The playlist is a list of **scenes**. A scene is either:

* **frames**: play a pre-rendered animation stored as a `.bin` file (BGR1 format)
* **fx**: render a procedural effect on the ESP32 (no bin file)
* **goto**: jump within the playlist (used for looping sections without “infinite scene loops”)

For reference at any time, a sample playlist can be found at `playlist.sample.jspn` in the root of this project. 

A web UI is the “compiler”: it produces the playlist JSON and generates `anims/<scene.id>.bin` files for frames scenes.

---

# Folder layout on SD

After compiling by the web UI, a playlist folder is copied onto the SD card as-is.

Example:

```
/playlist.json
/anims/
	/intro.bin
	/bad_apple.bin
	/spinny_globe.bin
```

Any other folders such as `/media/` are for the **editor-only** and would generally not copied to SD. Ignore them.

---

# `playlist.json` format

## Top-level structure

```json
{
	"version": 1,
	"display": { ... },
	"scenes": [ ... ]
}
```

### `version`

* Integer schema version of the playlist format.
* Current is `1`.

---

## `display` block (hardware + mapping config)

This tells the ESP32 player the **logical display resolution** and how to map logical pixels to the physical WS2812 chain.

```json
"display": {
	"w": 24,
	"h": 24,
	"brightness": 64,
	"tiles": { "x": 3, "y": 3, "w": 8, "h": 8 },
	"mapping": {
		"origin": "top_left",
		"tile_rotation": 0,
		"tile_order": "row_major",
		"tile_serpentine": false,
		"pixel_order": "row_major",
		"pixel_serpentine": false
	}
}
```

### `w`, `h`

* The **logical** pixel dimensions for playback and rendering.
* For a 3×3 grid of 8×8 tiles: `w=24`, `h=24`.

### `brightness`

* 0–255 brightness cap used by the player (FastLED/NeoPixel global brightness).
* This is a cap, not necessarily the “artistic intent”.
* Player should enforce it.

### `tiles`

Describes the tile layout that makes up the display:

* `x`, `y`: number of tiles horizontally/vertically
* `w`, `h`: tile dimensions in pixels (usually 8×8)

The logical width/height should match:

* `display.w == tiles.x * tiles.w`
* `display.h == tiles.y * tiles.h`

(You can still allow mismatch later, but v1 assumes they align.)

### `mapping`

This is the critical glue between:

* the **standard logical frame data** stored in `.bin` files (always logical pixel order)
  and
* the **physical LED index order** in the WS2812 chain.

Key idea:
**Frames in BGR1 are stored as a normal 2D image in logical order.**
The ESP32 player is responsible for remapping each logical (x,y) pixel to the correct LED index on the strip based on `mapping`.

Fields:

#### `origin`

Where `(0,0)` is in the logical display:

* `top_left`
* `top_right`
* `bottom_left`
* `bottom_right`

This applies before rotation/order/serpentine rules are applied.

#### `tile_rotation`

Rotation (degrees) applied per tile:

* `0`, `90`, `180`, `270`

This is a simple way to support physical tile orientation differences.

#### `tile_order`

How tiles are enumerated into a 1D sequence:

* `row_major`: left→right tiles across row 0, then row 1, etc.
* `column_major`: top→bottom tiles down col 0, then col 1, etc.

#### `tile_serpentine`

If true, alternate tile rows (row_major) or columns (column_major) reverse direction.
This matches common “snake” wiring layouts when tiles are chained in a zig-zag.

#### `pixel_order`

How pixels within a tile are enumerated:

* `row_major`: x increments fastest (left→right), then y
* `column_major`: y increments fastest (top→bottom), then x

#### `pixel_serpentine`

If true, alternate rows/columns reverse direction (like a snake) at the **pixel level inside each tile**.

> Your rigid PCB tiles tested as **NOT pixel-serpentine** internally.

---

# Scenes

`scenes` is an ordered list. Playback normally moves from `scenes[0]` → `scenes[1]` → … until the end.

Each scene has:

* `id` (string, recommended unique)
* `type` (`frames` | `fx` | `goto`)
* additional fields depending on `type`

## Shared scene fields

### `id`

Used as:

* a stable identity for goto targeting
* a stable filename for frames outputs: `anims/<id>.bin`

The toolchain “locked in” this rule:

> **For frames scenes, `file` is always `anims/<scene.id>.bin`**
> and the editor is responsible for overwriting the bin when re-rendering.

---

## Scene type: `frames`

Example:

```json
{
	"id": "bad_apple",
	"type": "frames",
	"fps": 12,
	"stop": { "seconds": 20, "plays": 3 },
	"compile": { "fit": "crop", "scale": "bilinear", "dither": "none" },
	"source": "/gifs/bad_apple.mp4",
	"file": "anims/bad_apple.bin"
}
```

### `fps`

The intended playback FPS for the scene.

Critical behaviour agreed during editor dev:

* If user edits `fps` after render **without re-rendering**, they are effectively time-stretching the playback. That’s allowed.
* Re-rendering at a new fps changes the number of frames stored, so playback duration remains closer to source.

### `stop`

Stop conditions for moving to the next scene. This is a **consistent object shape** in v1:

* If there are no stop rules, either no object shape, or an empty object `{}`, then playback will stop after one loop for `frames` type scenes or for `fx` type scenes, until the effect routine returns a stop condition.

Supported keys:

* `seconds`: stop after N seconds elapsed
* `plays`: stop after N complete animation plays (loops)

Stop semantics:

* The scene can stop based on **whichever condition hits first**.
* If both are present:

  * stop when `seconds` reached OR `plays` reached (first wins)
* If neither present:

  * default behaviour for v1 frames scene is **play once** (one full pass through stored frames)

Importantly:

* This avoids truly infinite scenes. If you want a loop forever, use a `goto` scene.

### `compile` (editor-only metadata)

These are instructions for the **compiler** (web UI) used to generate the `.bin` from source media. The ESP32 player does not need these to play the file.

* `fit`: `stretch` | `crop`
* `scale`: `nearest` | `bilinear` | `bicubic`
* `dither`: `none` | `ordered` | `floyd_steinberg`

They matter for reproducibility and future re-renders, but not runtime playback.

### `source` (editor-only)

Path to the selected source media file in the editor’s `/media` folder (not on SD).
Player should ignore.

### `file`

Path to the `.bin` relative to playlist folder.
Locked rule: `anims/<scene.id>.bin`

Player uses this to open the bin.

All other unknown or unneeded keys should be ignored by the player. 

---

## Scene type: `fx`

Example:

```json
{
	"id": "solid_teal",
	"type": "fx",
	"effect": "solid",
	"stop": { "seconds": 2 },
	"params": { "colour": "#00AEEF" }
}
```

### `effect`

A string keyword identifying the procedural effect.
The editor uses a dropdown of “supported effects” (for example: `solid`, `plasma`, `fire`, `rain`, `matrix`).

The player implements these effects.

### `stop`

For v1, FX uses:

* `stop.seconds`

In the future, there may be fx-specific stop conditions.

If `stop.seconds` missing:

* player can choose a default (e.g. 2–5 seconds) or treat it as “run until next scene trigger”
* but the editor generally sets it.

### `params`

Effect-specific parameters. This is intentionally flexible.

Example for `solid`:

* `colour`: `#RRGGBB`

In v1, params are not heavily validated. The player should:

* ignore unknown params
* apply defaults if missing
* be robust if types are wrong

---

## Scene type: `goto`

Used to loop or jump in the playlist without relying on infinite loops in a frames scene.

Example:

```json
{
	"id": "loop_back",
	"type": "goto",
	"target": { "id": "main_start", "index": 1 }
}
```

### `target`

* `id`: string scene id target
* `index`: numeric index target

Resolution rule:

* If `id` is present and matches a scene id, it wins.
* Else if `index` is present and in range, jump to that index.
* Else: do nothing or advance (player choice), but generally treat as invalid and skip.

Editor convenience:

* `target.id` is typically selected from a dropdown of current scene ids.
* When a scene id is renamed, editor updates any goto targets referencing it.

---

# BGR1 Binary Animation Format (`.bin`)

This is the on-disk format for frames scenes.

Design goals:

* Simple header
* Streamable (don’t need to load entire file into RAM)
* Supports multi-frame animations (GIF/video exports)
* Stored pixels are in **logical frame order**, not hardware order

## File overview

A `.bin` file is:

```
[Header (16 bytes)] [Frame0 RGB bytes] [Frame1 RGB bytes] ... [FrameN-1 RGB bytes]
```

No per-frame header. Frames are fixed-size.

---

## Header (16 bytes, little-endian)

Offsets and sizes are in bytes.

| Offset | Size | Type   | Name        | Meaning                                    |
| -----: | ---: | ------ | ----------- | ------------------------------------------ |
|      0 |    4 | ASCII  | magic       | `"BGR1"`                                   |
|      4 |    1 | u8     | w           | frame width in pixels                      |
|      5 |    1 | u8     | h           | frame height in pixels                     |
|      6 |    1 | u8     | channels    | currently `3` (RGB)                        |
|      7 |    1 | u8     | flags       | currently `0`                              |
|      8 |    2 | u16 LE | frame_count | number of frames                           |
|     10 |    2 | u16 LE | reserved    | currently `0`                              |
|     12 |    4 | u32 LE | data_offset | byte offset to frame data (currently `16`) |

### magic: `"BGR1"`

Not “BGR pixels”. It’s just a file signature + version (BitGRid1).
Version `1` matches the current header layout.

### w/h (u8)

Max 255×255 per file. Enough for current 24×24; also fine for most small TFTs later.

### channels

* Currently always `3`.
* Future-proofing only. (Don’t implement fancy variants now.)

### flags

* Currently `0`.
* Reserved for future (palette mode, compression flags, etc.)

### frame_count (u16)

Max 65,535 frames.
Frames can be huge in duration, but file sizes become the limiting factor.

### data_offset (u32)

Lets us extend the header in future without changing the basic parser flow.
In v1 it’s always `16`.

Player should trust it (within reason) and seek to `data_offset`.

---

## Frame data

Each frame is:

```
frame_size = w * h * channels
```

And frames are stored consecutively.

Pixel order inside a frame is **logical row-major** (top-left origin), i.e.:

For y from 0..h-1:

* for x from 0..w-1:

  * write RGB bytes for that pixel

Each pixel is stored as 3 bytes:

```
R, G, B
```

Example pixel sequence:

```
FF 00 00   00 FF 00   00 00 FF ...
```

### Important: hardware colour order vs stored order

WS2812 libraries often expect GRB order at output.
The file stores **RGB**.
So the player typically reads R,G,B and then writes to the LED object which handles GRB output (e.g. FastLED `CRGB(r,g,b)`).

---

## Streaming read strategy (ESP32 player)

Because files can be large (a 3 min music clip exported at ~10 MB video bin successfully), the ESP32 player must **stream frames**, not load the whole file.

Recommended approach:

1. Open file
2. Read 16-byte header
3. Validate header (magic, channels==3, data_offset >=16, etc.)
4. Compute:

   * `frameSize = w*h*3`
   * `frameCount`
5. For each frame `i`:

   * seek to: `data_offset + i*frameSize`
   * read `frameSize` bytes into a reusable buffer (or chunk-read if RAM tight)
   * map the logical pixels to LED indices using playlist mapping
   * show
   * delay until next frame time based on scene fps (or use a timing accumulator)

Memory notes:

* For 24×24: frameSize = 24*24*3 = 1728 bytes. That’s easy.
* Even 240×240 would be 172,800 bytes/frame, which is too big for a single buffer on many ESP32 builds unless you use PSRAM. But BitGrid v1 is 24×24.

---

# Hardware mapping responsibilities

This is the key separation of labour:

* **BGR1 frame files** contain a normal “image” in logical pixel coordinates.
* The **ESP32 player** is solely responsible for mapping `(x,y)` to the correct WS2812 LED index based on the `display.tiles` + `display.mapping` config.

So:

* the editor preview can ignore wiring (unless we add “wiring preview” UI later)
* the player must implement mapping correctly

In hardware testing we confirmed a working mapping for 24×24:

* tiles row-major (123/456/789)
* pixel order row-major
* pixel serpentine false
* origin top_left
* no rotation

That’s a “nice” baseline.

---

# How FX scenes fit in

FX scenes bypass `.bin` entirely.

Playback loop sees a scene:

* if `type == fx`:

  * run effect renderer for `stop.seconds` (or default)
  * effect draws into the same “logical framebuffer” abstraction (or writes directly to LEDs through mapping)
  * at end, advance to next scene

The trick that makes everything consistent:

* Both frames and fx ultimately produce a logical `(x,y)->RGB` output each frame.
* Then the same mapping code converts that to the physical LED array.

So in the player architecture, it’s clean to have:

* a “render target” representing logical pixels (or just compute and write per pixel)
* one mapping function: `xy_to_led_index(x,y)`
* one output stage: write to LEDs + show

---

# Scene timing: what the player must honour

## For frames scenes

* `fps` determines display timing for frame advancement.
* Rendered bins contain discrete frames. The player’s job is to show them at the requested fps.
* Stop rules (`stop.seconds`, `stop.plays`) determine how long to keep looping.

A simple, deterministic model that matches our editor intent:

* Define `frameDurationMs = 1000 / fps`
* One “play” is reaching the end of `frame_count` and wrapping back to 0.
* `stop.plays` counts complete plays.
* `stop.seconds` counts wall time since scene start.

Stop when either condition triggers first.

Default when no stop rules:

* play exactly one full pass through frames (one play)

## For fx scenes

* Run for `stop.seconds`
* Update at a chosen internal tick rate (e.g. 30 fps equivalent) or effect-defined update step.

## goto

* Perform jump immediately (no rendering)
* No delay unless you deliberately choose to show something (v1: don’t)

---

# Robustness expectations for player implementation

* With an SD card inserted, the player mounts SD, loads playlist, and plays content.
* Without SD (or on failure), it shows a clear error scene and prints actionable logs to Serial.

AI agents should implement the player defensively:

* If `stop` is missing or wrong type: treat as `{}`.
* If `fps` missing: default to 12.
* If `file` missing for frames: derive from id (`anims/<id>.bin`) or treat as error and skip.
* If bin header mismatch (wrong magic, channels !=3): skip with error.
* If w/h in bin doesn’t match playlist display w/h:

  * v1 simplest: allow it but scale? (not planned)
  * recommended v1: require exact match and skip if mismatch

On failure:
* Log error to Serial with specific reason
* Abort playback of this file for case of SD error reading
* For scene errors, show quick error scene, then move to next playlist item

---

# Summary “rules of thumb”

* **playlist.json drives everything**
* frames scenes reference `anims/<scene.id>.bin`
* BGR1 bin header is 16 bytes, little-endian fields
* frames are raw RGB, row-major, fixed-size
* ESP32 must stream frames, not preload whole file
* mapping is applied at runtime based on playlist `display.mapping`
* FX scenes ignore bins and render procedurally, but still output logical pixels through the same mapping

