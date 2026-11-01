# M0 rendering decision record

## Verified inputs, 2026-11-01

The official mpv **v0.40.0** `include/mpv/render.h` was downloaded and inspected. Its render API exposes OpenGL and software rendering, with no public D3D11 render backend. The development `master` header seen during research does expose D3D11; this must not be assumed for a pinned stable runtime. The official client header is vendored unchanged from v0.40.0.

Qt's `WindowContainer` exists since **6.8**. It embeds a QWindow in Qt Quick but native child windows occupy their own composition layer. Ordinary sibling QML items cannot reliably overlap the child window. It also cannot be combined with QQuickWidget or custom Qt Quick render targets.

## Selected M0 path

Qt Quick uses its D3D11 backend. A dedicated QWindow is hosted with WindowContainer, and its live HWND is passed to libmpv through `wid` before initialization. mpv owns video decoding, timing, subtitle composition and its D3D11 video output. Qt owns its separate scene graph and window lifetime. The top toolbar, error area, diagnostics and playback strip live outside the native video rectangle.

This is a real implementation candidate; the Linux host cannot prove Win32 embedding correctness. Do not call it verified until the Windows matrix passes. mpv's own documentation prefers its render API where supported because embedding can have toolkit/platform issues.

The application does not move video frames between Qt and mpv or perform CPU readbacks. The mpv D3D11 backend can still perform an internal GPU-to-GPU copy; **zero-copy is not claimed**. Do not enable the documented experimental `d3d11va-zero-copy` optimization without driver/content testing.

Use `hwdec=d3d11va` to request the native D3D11 decoding path. Unsupported streams use mpv's software fallback. An explicit Software recovery selection sets `hwdec=no`. The observed `hwdec-current` property is evidence of the selected decoder; a requested preference alone is not evidence. No hardware AV1 support is assumed for the GTX 1650 Max-Q.

## Alternatives and fallback

| Path | M0 disposition | Required follow-up |
| --- | --- | --- |
| Native D3D11 output via `wid` | Implemented candidate | Windows focus, DPI, resize, fullscreen, recovery and hybrid-GPU tests |
| D3D11 render API plus Qt RHI | Not implemented; absent from pinned 0.40.0 header | Select a supporting pinned release, verify DLL build support, agree device/context ownership, render thread, texture format, synchronization and HDR color handling |
| OpenGL render API | Available API, not selected automatically | Explicit compatibility investigation only; test ANGLE/native GL, decode interop, adapter choice and HDR constraints before adopting |
| Software render API | Not selected | Avoid CPU frame copies as a production video route |

If embedded native D3D11 fails, first try the software **decoder**, preserving the same D3D11 renderer. A GPU-output failure remains a visible error; the app does not silently switch to an unrelated renderer. Retain this native-window host as a compatibility path when future shared-texture work is evaluated. If it remains unstable, record the failing case and stop milestone acceptance. No automatic untested texture-sharing fallback exists.

## HDR policy

M0 fixes output to SDR: `target-prim=bt.709`, `target-trc=bt.1886`, `d3d11-output-csp=srgb`, `d3d11-output-format=rgba8`. This asks mpv to map HDR content into SDR rather than advertise passthrough. Actual HDR color/tone-map quality is unverified. Do not interpret Main10, GPU capability or Windows HDR alone as a verified HDR pipeline. Windows display capability/active-HDR interrogation and calibrated output are M4 work. CPU/GPU usage, VRAM and adapter identity are not fabricated in diagnostics.

## Threading and lifetime

Normal playback commands/property writes are asynchronous. mpv's wakeup callback only coalesces a queued Qt event drain. The GUI drains nonblocking events in bounded batches, emitting one state update per batch. The application never calls render-API entry points or holds Qt's render thread waiting on mpv.

The callback is unregistered and mpv workers are terminated while the Player and video HWND still exist. Queued functors use the Player QObject as context. Startup/shutdown library calls are synchronous; shutdown can wait for engine workers. Paused idle behavior and long-run resource use still need Windows profiling.

## Primary references

- https://github.com/mpv-player/mpv/blob/v0.40.0/include/mpv/client.h
- https://github.com/mpv-player/mpv/blob/v0.40.0/include/mpv/render.h
- https://github.com/mpv-player/mpv/blob/v0.40.0/DOCS/man/options.rst
- https://doc.qt.io/qt-6.8/qml-qtquick-windowcontainer.html
- https://doc.qt.io/qt-6/qsgrendererinterface.html

Do not substitute the moving `master` manual for the headers/options shipped with a chosen binary.
