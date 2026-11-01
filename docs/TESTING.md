# M0 test record and acceptance matrix

Automated validation ran on a Linux x64 container with GCC 13.3, CMake 3.31.6, Ninja 1.11.1, extracted Ubuntu Qt 6.4.2 and libmpv 0.37.0 runtime packages. The compiled client header is official mpv 0.40.0 (compatible client ABI major 2). The runtime FFmpeg version is recorded in `M0-REPORT.md` from observed properties. This is a compatibility test, not validation of the requested Windows 0.40.0 runtime.

Fixture: FFmpeg-generated 640×360, 24 fps, 12-second MPEG-4 Part 2 video with 48 kHz AAC sine-wave audio. The test engine uses null audio/video outputs; decoding is real. No output screenshot can establish audible A/V sync.

The QtTest suite covers missing DLL, rejected HTTP/UNC inputs, successful file load and advancing position, duration and codec observations, pause stability, exact and relative seek, clamped volume, mute, stop/reopen, corrupt-file error/recovery, and 12 immediate load/shutdown cycles. Tests fail when required runtime/fixture environment variables are absent; they do not silently skip integration coverage.

## Windows acceptance — all pending

Record actual GPU, driver, Windows build, display, Qt, mpv and FFmpeg versions with each result. Test the i5 9th-gen / GTX 1650 Max-Q laptop, including Intel iGPU if present.

| Check | Procedure and evidence | Result |
| --- | --- | --- |
| Build/bootstrap | Configure/build x64, deploy DLLs, launch from clean folder without developer PATH | Not run |
| D3D11 video + audible sync | Play a clap/timecode test with real audio output; verify visible frames and audible alignment after 15 minutes and repeated seeks | Not run |
| H.264 | 1080p 24/30/60 fps; verify original timing, observed codec and hwdec-current | Not run |
| HEVC Main10 | Representative 1080p/4K; verify actual decoder and dropped-frame counters | Not run |
| VP9 / AV1 | Test hardware-supported VP9 and software AV1; measure performance rather than assuming real-time 4K AV1 | Not run |
| Unsupported hardware format | Verify automatic software fallback and explicit Software recovery without losing controls | Not run |
| Fullscreen/resize | 100 enter/exit cycles, maximize/minimize/restore, paused and playing, retain audio/surface | Not run |
| Focus/keyboard | Test F/Space/arrows/M/Esc after clicking video, sliders, diagnostics and native file dialog; check native-child focus behavior | Not run |
| Double-click | Probe native video-window event forwarding; button/F11 is the M0 guaranteed action in source | Not run |
| High DPI | 100/125/150/200%; inspect video bounds, controls, file dialog and monitor moves | Not run |
| Hybrid GPU | Run on Intel and NVIDIA via Windows Graphics preference; verify same-window render stability, no black frame/deadlock | Not run |
| Errors/recovery | Missing mpv DLL, missing dependent DLL, corrupt/deleted file, unsupported codec, stop/reopen | Not run |
| Paths | Unicode/Indian/Polish characters, spaces, long paths; local files only | Not run |
| Native window stacking | Toolbar/errors/diagnostics never overlap video; future immersive overlays remain blocked by native airspace | Not run |
| Resources/idle | CPU/GPU/VRAM through external profilers, 2-hour playback, paused and minimized; no application counters fabricated | Not run |
| SDR/HDR input | Inspect SDR colors; HDR source maps to SDR policy; no passthrough claim or HDR acceptance yet | Not run |
| Privacy | Network trace with local fixtures and reference-containing files; no unsolicited requests | Not run |
| Shutdown | Quit while playing, paused, loading and after decoder failure; verify workers exit and HWND is released | Not run |

M0 acceptance requires an actual Windows build, local video with synchronized audible audio, seeking, fullscreen and functional controls. Do not start M1 on the assumption that Linux tests satisfy that gate. The environment limitation is documented so the owner can perform this verification locally.
