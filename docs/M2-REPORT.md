# M1–M2 development report — 2026-11-01

## Scope

M1 design foundation and M2 playback controls are implemented. M3 library indexing and later milestones have not been started. The Windows package is a development build, version 0.3.0-m2.

## Implemented

- Four complete palettes, live system-theme tracking, GUI-thread palette animations, Windows/user reduced motion, scaling and reusable QML controls.
- Responsive home/sidebar/recent/queue/details/settings views and native owned floating HUD with playback auto-hide.
- Audio/subtitle selection, external subtitles, timing adjustments, speed, aspect/scaling/rotation, chapters, frame stepping, looping, A–B and screenshots.
- Real playlists, next/previous/index navigation, multi-file opening, local drag/drop and autoplay preference.
- Worker-thread SQLite history/resume with file-size/mtime identity checks; persisted preferences, keyboard assignments, volume and normal window geometry.
- Exact upstream LGPL runtime archive/DLL hashes, dependency provenance, CRT deployment, native Windows CI and UI evidence capture.

## Validation results

- Linux Qt 6.8.3 backend integration suite: passed, including actual H.264/AAC/AC-3 5.1 track switching, embedded/external SRT, chapters, A–B, scaling/rotation, queue operations, SQLite persistence and HEVC Main10 software decoding.
- Native Windows x64 compilation: passed with MSVC and Qt 6.8.3 on the hosted Windows Server 2022 runner.
- Native Windows expanded playback integration suite: passed against the pinned upstream LGPL libmpv runtime.
- MinGW cross-compilation: produced a real PE32+ Windows x64 application.
- Actual application QML: initializes in the offscreen software test harness; Light/Dark/settings layouts have been visually inspected.
- Native full-UI smoke test: passed with advancing real media, empty QML/error lists, observed `current-vo=gpu` and visible colorful video asserted beneath the HUD in both windowed and fullscreen captures. The HUD bounds must exactly match the client area in windowed/fullscreen captures. All four themes, settings and reduced-motion behavior are exercised.
- The initial deferred theme-singleton startup fault was fixed in C++; the embedded-child HUD blending failure was isolated with a HUD-hidden capture and resolved with an owned translucent tool window. Failed candidates were not packaged for delivery.

## Reproducible build evidence

- Application source revision: `065138fdd1a1c7ad28f80461f51527fdbaeef573`.
- Successful native MSVC build/test/package run: https://github.com/omkaarpatil787-dev/Jadzumi-Windows-Player/actions/runs/41262893184
- Windows backend integration suite: CTest passed, 0 failures, 4.05 seconds. Linux compatibility suite: passed, 2.88 seconds. MinGW cross-build also succeeded.
- Runtime observed by the native smoke: mpv `v0.41.0-dev-g9e2ed65eb`, FFmpeg `git-2026-10-30-a3c5339`; rawvideo fixture, software decoding on the virtual runner. No hardware-decode capability is inferred.
- Portable delivery contains `jadzumi.exe`, Qt/libmpv/CRT DLLs and plugin directories, licenses/provenance, build fingerprints and these updated documents. The application binaries are from the successful run; final documentation is updated separately without modifying the executable.

## Remaining physical verification

A physical i5 9th-gen / GTX 1650 Max-Q laptop was not connected. Hardware decode support, audible end-to-end sync, HDR output, hybrid-GPU behavior and extended resource profiling remain explicitly unverified. Synthetic tests do not establish those capabilities.

## Main files changed

`src/playback/*`, `src/library/HistoryStore.*`, `src/settings/*`, `src/app/main.cpp`, native diagnostics/icon resources, all QML screens/components, integration/UI test harnesses and fixtures, Windows CI/packaging scripts, dependency records and build/validation documents.

## Next after M2

Verify on the target laptop with real local SDR/HDR/multichannel/subtitle files, record issues, then plan M3. Do not infer M3–M6 completion from the availability of a development EXE.
