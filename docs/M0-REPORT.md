> Historical M0 record. Current native Windows build and M1–M2 evidence are in [M2-REPORT.md](M2-REPORT.md) and [TESTING.md](TESTING.md).

# M0 implementation report — 2026-11-01

**Result: M0 source candidate implemented; Windows acceptance pending. M1 not started.**

## Environment audit

The workspace is Linux x64, not Windows. At entry it had GCC 13.3 and FFmpeg CLI but lacked CMake, Qt 6 development files, libmpv and a Windows SDK/compiler/session. No target laptop/GPU was attached. CMake 3.31.6 and Ninja 1.11.1 were installed in temporary tooling. Ubuntu packages were downloaded/extracted into an isolated sysroot after system package installation encountered restricted cache permissions; the shared playback backend could then be built and tested. No Windows executable was produced.

Observed runtime versions in the successful integration test:

| Component | Tested value |
| --- | --- |
| Compiler | GCC 13.3.0 |
| Qt / QtTest | 6.4.2, Ubuntu 24.04 |
| libmpv | mpv 0.37.0, client API major 2 |
| Runtime FFmpeg | 6.1.1-3ubuntu5 |
| Render/audio output | null/null for headless automated tests |
| Decoder | software (`hwdec-current=no`) |
| Test media | 640×360 MPEG-4 Part 2, 24 fps, AAC mono 48 kHz, 12 seconds |

Intended Windows baseline is separately pinned in `third_party/dependencies.json` (Qt 6.8.3, mpv 0.40.0, FFmpeg 7.1.1). Those Windows artifacts have not been obtained or validated. Their exact hashes, transitive versions and provenance remain unresolved.

## Files implemented

- `CMakeLists.txt`, `CMakePresets.json`: C++20, Windows x64 application target, independent portable backend tests, Qt QML module/resource registration, RC metadata.
- `src/playback/MpvApi.*`: official C-ABI loading from explicit DLL path, export checks and API-major compatibility checks.
- `src/playback/Player.*`: real mpv initialization, native `wid`/D3D11/D3D11VA settings, software recovery, asynchronous commands/property writes, observed state/diagnostics, error handling and callback-safe teardown.
- `src/rendering/VideoWindow.*`: native Qt-owned video host with explicit lifetime.
- `src/app/main.cpp`, `src/platform/windows/app.rc`: Qt Quick bootstrap, Windows DLL-search restrictions, QSettings, bounded local logs and version metadata.
- `qml/Main.qml`: functional file chooser, playback strip, scrubber, volume/mute, fullscreen action, shortcuts, decoder selection and observed diagnostics. It is an M0 utility interface, not the later premium design milestone.
- `tests/PlaybackTests.cpp`, `tests/make-fixture.sh`: real-media state/control/recovery/lifecycle integration tests and synthetic A/V fixture generator.
- `third_party/mpv/include/mpv/client.h`: unchanged official 0.40.0 client header with preserved license and SHA-256.
- `packaging/*.ps1`: Windows development staging and private GitHub repository handoff helpers; scripts not run on Windows.
- `README.md`, `docs/*`, `LICENSES/README.md`: setup, architecture decision, test matrix, known issues, milestone status, repository handoff and unresolved distribution/license requirements.

## Build and test results

| Check | Result |
| --- | --- |
| CMake configure — backend | Passed on Linux |
| C++ backend and QtTest executable build | Passed, GCC warning flags enabled |
| CTest `playback_contract` | Passed, 1/1 suite, approximately 1.07 s |
| QtTest cases | 6 passed including setup/cleanup; 0 failures; 0 skips |
| Real media open/advancing position | Passed with real libmpv demux/decode |
| Pause, exact/relative seek, volume/mute, stop/reopen | Passed |
| Corrupt media failure and subsequent recovery | Passed |
| 12 load/shutdown callback stress cycles | Passed |
| QML grammar via Qt `qmlformat` parser | Passed; not Qt 6.8 semantic/runtime validation |
| Vendored header SHA-256 | Matches dependency record |
| Windows app compile/link | Not run — Windows toolchain unavailable |
| Visible video, audible A/V sync, fullscreen | Not run — Windows application unavailable |
| GTX 1650 Max-Q / Intel hybrid hardware decoding | Not run |
| HDR output, DPI, resource usage, long-session stability | Not run |
| Packaging helpers | Source-reviewed only; not executed on Windows |

The tests use real decoding with null presentation outputs. They do not demonstrate audible audio, GPU video rendering, end-to-end synchronization or Windows performance. No fake engine, screenshot substitute or claimed successful Windows build is included.

## Rendering decision and risks

The pinned 0.40.0 render header has no public D3D11 render-API backend, so M0 requests native D3D11 output through `wid` instead of assuming new `master` APIs. Qt WindowContainer requires 6.8 and creates native-window stacking constraints. Controls remain outside the video area. Shared RHI textures, interop synchronization and immersive overlays remain unresolved follow-up work, documented in RENDERING.md. HDR output is not advertised; the initial pipeline explicitly requests SDR output. No zero-copy or hardware support claim is made without Windows evidence.

## Repository and next steps

The owner created the public repository `omkaarpatil787-dev/Jadzumi-Windows-Player`. Its initial commit and `.gitattributes` are preserved, and this M0 source is added on `main`. The Windows project is independent of the Android player. See REPOSITORY.md for cloning instructions.

Next: obtain and lock the audited Windows runtime, build using BUILD-WINDOWS.md, then execute every M0 Windows acceptance item in TESTING.md. Resolve any native-window, focus, GPU or synchronization failures before proceeding to M1. Final codec/security/license auditing, installer/signing and release packaging remain M6 requirements.
