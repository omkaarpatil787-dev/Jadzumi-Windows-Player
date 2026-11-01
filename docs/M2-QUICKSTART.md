# Jadzumi Player M2 — Windows x64

1. Extract the entire ZIP to a folder you can write to.
2. Open `Jadzumi-Player` and run `jadzumi.exe`.
3. Select **Open video**, drop local videos onto the window, or press **Ctrl+O**. Multiple selections become a real playback queue.

Keep the EXE, DLLs and plugin directories together. The development application is unsigned; no separate Qt/libmpv installation is needed. Supported decoding depends on the bundled runtime and your machine.

## Controls

Space toggles playback; Left/Right seek 10 seconds; Up/Down change volume; M mutes; F/F11 toggles fullscreen; Esc exits fullscreen; S toggles subtitles. The settings panel lists the actual customizable bindings. Double-click the video for fullscreen. Controls reappear when the pointer moves and remain visible while paused or using a panel.

Use the bottom HUD buttons for audio/subtitles, video settings and additional controls. The queue button opens playlist navigation. Open Settings for themes, reduced motion, interface scale, resume/autoplay and history controls. The diagnostics panel shows observed decoder and stream values; it does not invent GPU telemetry.

For a first check, play an ordinary local H.264 MP4, seek, change volume, enter/exit fullscreen and reopen the file to check resume. Then test your HEVC, subtitle and multichannel files. If hardware decoding fails on a driver/format, disable Hardware decoding in the video panel and retry.

## Data and feedback

Settings/history/logs remain local. Clearing playback history never deletes media. Screenshots are saved under Pictures/Jadzumi Player. A full scanned media library is planned for M3.

This M2 build outputs SDR. Hardware decoding, HDR output, audible sync and long-session performance on the GTX 1650 Max-Q are not certified by hosted CI. When reporting a problem, include Windows/GPU/driver versions, diagnostics, the action that triggered it and whether software decoding changes the result. Logs can contain local filenames; review them before sharing.

See M2-REPORT.md, TESTING.md and KNOWN-ISSUES.md for evidence and limits.
