# M1–M2 implementation checkpoint

Scope: native design foundation and complete playback controls through M2, followed by a runnable Windows x64 development package. M3 library indexing and M4 HDR/audio processing are not represented as implemented.

1. Establish a native Windows build/test job and pin the available upstream LGPL libmpv binary by exact source commit and archive hash. Preserve the native D3D11 window route and explicitly test composition/focus.
2. Add four themes, semantic tokens, reusable controls, adaptive navigation, Windows/user reduced motion and persistent preferences.
3. Extend observed playback state and commands for audio/subtitle tracks and synchronization, speed, aspect/fit/fill/zoom, chapters, frame stepping, rotation, screenshots, A–B repeat and looping.
4. Add real playlists, next/previous, local subtitle loading, persisted SQLite resume/history, and customizable shortcuts with conflict checks.
5. Replace the M0 utility interface with a coherent home/recent/queue/player/settings flow. Native overlay controls must remain usable and auto-hide only during playback.
6. Run backend integration tests and native Windows build/runtime checks, stage actual dependencies and notices, and provide the executable with the portable ZIP. Record target-hardware/HDR verification limits separately from completed checks.

No motion interpolation or frame-rate conversion. No decorative settings or fabricated telemetry.
