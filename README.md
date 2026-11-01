# Jadzumi Player — Windows Edition

Native, offline-first Windows 11 x64 video player. C++20, Qt Quick/QML and the official libmpv client API. Created by Omkar Patil with AI-assisted development.

**Milestone: M0 implementation candidate.** The C++ backend builds and passes real libmpv integration tests on Linux. The Windows executable, native D3D11 rendering, GPU decoding, audible A/V synchronization and fullscreen have **not** been validated on Windows. M0 acceptance is pending; M1 has not started. See [the implementation report](docs/M0-REPORT.md).

## Implemented in M0

- Dynamic loading of the official libmpv C API from an explicit application-local DLL.
- Qt Quick bootstrap and real native video-window host, requesting `vo=gpu`, D3D11 and D3D11VA with libmpv's software decoder fallback.
- Local file opening, command-line opening, play/pause, stop, absolute/relative seeking, volume and mute.
- Fullscreen action, fixed initial keyboard shortcuts and decoder recovery choice.
- Event-driven position, duration, stream and decoder diagnostics. No progress polling timer or application video-frame readback.
- Bounded local logs, visible errors, persisted volume and logical window size.
- Real integration tests using generated video and audio, with a headless test output that is never used by the Windows app.

No motion interpolation or frame-rate conversion. The engine uses original media timing (`video-sync=audio`, `interpolation=no`). Qt's UI rendering remains independent.

## Build and validation

Start with [Windows setup](docs/BUILD-WINDOWS.md), [renderer decision](docs/RENDERING.md), [test matrix](docs/TESTING.md) and [known limitations](docs/KNOWN-ISSUES.md).

```powershell
# x64 Native Tools Command Prompt / Developer PowerShell, Qt 6.8.3 msvc2022_64
$env:CMAKE_PREFIX_PATH = 'C:\Qt\6.8.3\msvc2022_64'
cmake --preset windows-msvc
cmake --build --preset windows-msvc
# Supply an audited x64 libmpv runtime as described in docs/BUILD-WINDOWS.md.
```

Only implemented M0 controls appear in the UI. Library, themes, artwork, immersive overlays, HDR display verification, advanced audio/subtitles, installers and file associations remain in their specified later milestones.

## Repository

Source: [omkaarpatil787-dev/Jadzumi-Windows-Player](https://github.com/omkaarpatil787-dev/Jadzumi-Windows-Player), branch `main`. This is an independent Windows project; the Android player is unchanged. See [repository setup](docs/REPOSITORY.md) for cloning instructions.

## Privacy and licensing

The application contains no HTTP client, accounts, analytics, updater or metadata requests. It accepts local media paths, disables mpv user config, scripts, ytdl and external references, and restricts the FFmpeg lavf protocol list to `file`. Logs and preferences remain on this computer. File deletion is not implemented.

The project source is publicly visible, but an open-source distribution license has not been selected. The vendored mpv client header retains its own permissive notice. Qt, libmpv, FFmpeg and all bundled dependencies require their own notices, source/relinking obligations and final license review before binary distribution. See [LICENSES/README.md](LICENSES/README.md). No Windows release binary is supplied.
