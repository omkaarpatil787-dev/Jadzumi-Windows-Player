# Jadzumi Player — Windows Edition

A native, offline-first Windows x64 video player built with C++20, Qt 6 Quick/QML and the official libmpv client API. Created by Omkar Patil with AI-assisted development.

**Current scope: M1–M2 development build (0.3.0-m2).** This is a working playback application, not a mockup. Native Windows compilation and the expanded real-media backend tests have passed. Native visible-video/HUD and fullscreen smoke checks have also passed. Build and package evidence is tracked in [M2-REPORT.md](docs/M2-REPORT.md). Physical laptop GPU, audible synchronization and HDR-output checks remain separate acceptance work.

## Features through M2

- Light, Dark, AMOLED and System themes; animated color transitions, Windows reduced-motion support and interface scaling.
- Adaptive sidebar, home/recent playback, file details, queue and persistent settings.
- Native D3D11 video hosting with a dedicated transparent owned Qt Quick tool window for floating player controls.
- Play/pause, stop, scrub/relative seek, volume/mute, 0.5×–2× speed and frame stepping.
- Real audio/subtitle track selection, external subtitle loading, subtitle visibility, size/position and audio/subtitle synchronization.
- Aspect ratio, Fit/Fill/Zoom, rotation, chapters, A–B repeat, looping and screenshots.
- Multi-file opening, drag-and-drop, playlist navigation, autoplay preference and SQLite resume/history on a worker thread.
- Customizable keyboard shortcuts with conflict checks, fullscreen, and persisted normal window size/position.
- Observed codec/decoder diagnostics, visible errors and bounded local logs.

Original media frame rates are preserved (`video-sync=audio`, `interpolation=no`). There is no motion interpolation, optical flow or generated-frame conversion. UI motion is independent of video timing.

## Run

Use the portable Windows package from the build artifacts: extract the entire folder and run **`jadzumi.exe`**. Keep its Qt and libmpv DLLs and plugin directories together. Copying just the application EXE to an empty folder will not provide its runtime dependencies.

Quick start: [M2-QUICKSTART.md](docs/M2-QUICKSTART.md).

Source: [omkaarpatil787-dev/Jadzumi-Windows-Player](https://github.com/omkaarpatil787-dev/Jadzumi-Windows-Player), branch `main`. The Android project is unchanged.

## Build and tests

See [Windows build instructions](docs/BUILD-WINDOWS.md), [rendering decisions](docs/RENDERING.md), [tests](docs/TESTING.md), [known limitations](docs/KNOWN-ISSUES.md) and [milestones](docs/MILESTONES.md).

The Windows workflow builds with Qt **6.8.3**, verifies an exact upstream LGPL libmpv archive hash, exercises synthetic real media and stages dependencies. The runtime is an explicitly pinned **development** snapshot, not advertised as a stable release. Its exact commit and binary hashes are in [third_party/dependencies.json](third_party/dependencies.json).

## Privacy and distribution

No accounts, advertising, analytics, metadata requests or updater. Core playback uses local files. mpv user config/scripts, ytdl and external references are disabled. The lavf protocol list is restricted to `file`; explicit URLs/network-share paths are rejected. Displayed metadata uses plain text. History and preferences remain local; clearing history never deletes videos.

M3 library scanning and M4 advanced HDR/audio processing are not implemented. Output is SDR in this development build. M5 profiling and M6 release security/licensing/signing remain pending. The publicly visible application source has no general open-source license selected yet; third-party libraries retain their own terms. Dependency notices/provenance are included. See [LICENSES](LICENSES/README.md).
