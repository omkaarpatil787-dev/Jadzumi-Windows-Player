# M2 development dependency provenance

Jadzumi dynamically loads Qt and libmpv. Replacement compatible DLLs are allowed; no signature restriction prevents relinking or debugging a modified library. No user media is included in the application package.

## Qt 6.8.3

The Windows build uses the unmodified `msvc2022_64` SDK. Qt Core, Gui, QML, Quick, Quick Controls, Shapes, SQL and bundled plugins are dynamically deployed. LGPLv3 and incorporated GPLv3 texts are included here. Qt Shader Tools is used as a build tool; the application does not link its optional baking API.

Exact corresponding Qt source archives, including third-party notices:

- https://download.qt.io/archive/qt/6.8/6.8.3/submodules/qtbase-everywhere-src-6.8.3.tar.xz
- https://download.qt.io/archive/qt/6.8/6.8.3/submodules/qtdeclarative-everywhere-src-6.8.3.tar.xz
- https://download.qt.io/archive/qt/6.8/6.8.3/submodules/qtsvg-everywhere-src-6.8.3.tar.xz
- https://download.qt.io/archive/qt/6.8/6.8.3/submodules/qtshadertools-everywhere-src-6.8.3.tar.xz

## libmpv LGPL development build

This is the upstream **LGPL** build, not its GPL variant. Source commit:

https://github.com/mpv-player/mpv/tree/9e2ed65eb0c02f99a352dfa1ad3c85ecceefe005

Upstream binary build run: https://github.com/mpv-player/mpv/actions/runs/41103006513

The exact archive URL, SHA-256, DLL SHA-256, source commit and runtime version are recorded in `dependencies.json` in the portable folder, and `third_party/dependencies.json` in the source repository. The archive's `LICENSE.LGPL` is copied to `licenses/mpv`. FFmpeg and libass are bundled by upstream; actual reported FFmpeg version is included in the CI evidence and player diagnostics.

The public client header in the repository remains v0.40.0 with its permissive header notice; the compatible client-API-major-2 runtime is the newer pinned development build. The native D3D11 output route is retained.

## Microsoft runtime

The native MSVC build deploys the x64 Microsoft Visual C++ redistributable CRT DLLs from the installed Visual Studio redistributable directory. Microsoft redistribution terms apply to those files. Qt and libmpv do not replace Windows licensing.

## Release gate

This M2 package is an unsigned development build for validation. Final transitive-source/notice completeness, codec patent review, security review, stable-runtime selection, signing and installer review remain mandatory M6 work. This file records provenance; it does not certify general redistribution compliance or HDR/GPU capability.
