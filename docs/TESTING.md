# Validation through M2

## Automated tests

The native Windows workflow uses MSVC 2022, Qt 6.8.3 and the exact LGPL libmpv development runtime recorded in `third_party/dependencies.json`. The backend suite also runs on Linux with Qt 6.8.3 and libmpv 0.37.0 as a client-API compatibility check. Neither result establishes physical GPU compatibility.

`PlaybackTests` exercises real demuxing and decoding with null presentation outputs:

- Missing runtime, invalid/remote inputs, corrupt media and recovery.
- Load, advancing timestamps, pause, exact/relative seek, volume/mute, stop/reopen and 12 immediate load/shutdown cycles.
- H.264 with AAC mono and AC-3 5.1 tracks; actual audio selection, embedded/external SRT, chapters and synchronization controls.
- Speed, subtitle visibility/scale/position, aspect/scaling/rotation, frame stepping, A–B and looping.
- Actual queue/index/next/previous operations and autoplay-off behavior.
- HEVC Main10 software decoding, Unicode Hindi/Polish paths with spaces/#/%.
- Preference persistence/shortcut conflicts and worker-owned SQLite identity checks, resume data, reopen and clearing.

Fixture provenance and regeneration are documented in `tests/fixtures/README.md`. The standard-library generator creates a 160×90, 24 fps, 12-second raw RGB/PCM AVI; the committed MKV fixtures contain synthetic H.264/AAC/AC-3/subtitles/chapters and SDR HEVC Main10. No downloaded movie is required.

## Native UI/playback smoke

The actual executable loads the AVI in the D3D11 HWND, checks advancing timestamps, real `current-vo`, errors and QML diagnostics, switches all themes/reduced motion, opens/closes settings and enters/exits fullscreen. Test-only desktop captures include native child windows. A colorful-pixel assertion in an unobstructed fixture band verifies that video is visible in both windowed and fullscreen captures; successful decoding alone is insufficient. Exact HUD/client bounds are asserted in both modes, and manual visual inspection checks the HUD and settings composition.

Test-only screenshots and crash diagnostics are enabled only by explicit CLI/environment options; normal playback performs no screen readback or crash upload. `ui_smoke_host` is a separate, undistributed offscreen layout harness and cannot establish Windows rendering correctness.

See `M2-REPORT.md` for the exact final source revision, run and result. Prior successful backend/UI-exit results without visible-video checks are not composition acceptance.

## Physical Windows acceptance still required

Record CPU/GPU, driver, Windows build, display, Qt/mpv/FFmpeg versions and observed `hwdec-current` for each result. Test the i5 9th-gen / GTX 1650 Max-Q and Intel iGPU if present.

| Check | Procedure | Status |
| --- | --- | --- |
| Portable deployment | Launch extracted package on a clean Windows 11 x64 computer without developer PATH | Pending |
| Audible synchronization | Clap/timecode fixture through real audio output after 15 minutes and repeated seeks | Pending |
| Hardware codecs | 1080p H.264/HEVC/Main10/VP9; record actual decoder, dropped frames and load; AV1 software fallback | Pending |
| Focus and controls | Mouse/keyboard, file dialogs, drag/drop, shortcuts after sliders/text edits, media keys while app active | Pending |
| Fullscreen stress | 100 cycles, maximize/minimize/restore, paused/playing, video/HUD/input retained | Pending |
| DPI and displays | 100/125/150/200%, monitor moves and differing refresh rates | Pending |
| Hybrid GPU | Select Intel then NVIDIA through Windows Graphics preferences; check video/HUD/device-loss behavior | Pending |
| Resources | External CPU/GPU/VRAM profilers, 2-hour playback, idle/paused/minimized; no fabricated counters | Pending |
| Color/HDR input | SDR correctness and HDR-to-SDR visual checks; HDR output is not implemented | Pending |
| Privacy | Network trace with ordinary and reference-containing local media; mapped-drive/link audit | Pending |
| Failure/shutdown | Slow/missing drive, removed files, runtime errors, repeated shutdown while loading/playing | Pending |

M3–M6 and release certification are separate work. Synthetic hosted-runner checks cannot prove audible synchronization, hardware efficiency, HDR output or long-session stability.
