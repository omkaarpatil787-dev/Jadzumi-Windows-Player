# Known limitations — M2 development build

1. Native Windows compilation and real-media control tests pass. UI smoke-test results and visual composition evidence are recorded in M2-REPORT.md; target-laptop GPU, audible synchronization, multi-monitor and long-session validation are not implied by CI results.
2. The available upstream LGPL libmpv Windows runtime is a pinned development snapshot. Its exact source commit, archive and DLL hashes are recorded. A stable-runtime/security/transitive-license audit is still required before M6 release.
3. The video and Qt scene graph use separate native D3D11 windows. A transparent owned tool window carries the HUD; ordinary QML siblings cannot overlap the native video. Direct Qt RHI/libmpv texture integration is not implemented or claimed as zero-copy.
4. Windows focus, media-key behavior outside the active application, hybrid GPU switching, HDR monitors, device-loss recovery and playback while moving between displays need physical-machine testing.
5. Output is SDR. The app does not claim verified HDR passthrough or universal Dolby Vision. Ten-bit HEVC decoding is tested separately from HDR color correctness.
6. Diagnostics show observed stream/decoder values. CPU/GPU utilization, VRAM and actual GPU adapter identity are not measured. Missing telemetry is not fabricated.
7. Core paths are local videos. Explicit network URLs/UNC paths and playlist files are rejected. Network-mapped drives and link traversal still require a Windows privacy audit. No automatic media deletion exists.
8. SQLite resume/history is not a scanned media library. Poster caching, folder indexing, search/sorting across a collection and optional online metadata belong to M3 or later. Failed/missing files produce visible errors.
9. The build is unsigned. Installer/release signing and full release qualification are M6 tasks. Development packaging is supplied to make the M2 app runnable, not to certify a release candidate.
10. Shutdown synchronously joins mpv and flushes SQLite worker writes. Extended playback, slow-disk failure, paused-idle usage and memory/GPU resource behavior need M5 profiling.
11. ASS/SSA script positioning can override simple subtitle position/scale controls. Advanced font/color controls, audio-device selection, passthrough and processing are not exposed as fake settings.
12. Language selection, updater controls, file associations, taskbar integration and background Windows media-session support are not implemented in M2.
13. App code has no general redistribution license selected. Library source/notice completeness, codec patent review and dependency security auditing remain unresolved release gates; see LICENSES/THIRD-PARTY-SOURCES.md.
