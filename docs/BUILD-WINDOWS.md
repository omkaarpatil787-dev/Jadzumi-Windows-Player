# Windows build instructions

## Baseline

- Windows 11 x64, Visual Studio 2022 C++ Desktop workload, Windows 11 SDK, x64 developer shell.
- Qt **6.8.3**, `msvc2022_64`, including Qt Quick, Controls, QML and Qt Test. Dynamic Qt libraries.
- CMake **3.31.6**, Ninja **1.11.1** (the Linux validation tool package was 1.11.1.3).
- Windows x64 libmpv build based on **mpv 0.40.0**, with FFmpeg **7.1.1**, libass and D3D11 output enabled. The shipped binary, codec configuration and exact dependency versions must be recorded and reviewed. These Windows binary artifacts were unavailable in this workspace.
- Optional FFmpeg command-line tool for generating the integration fixture. It is a build/test tool, not a second application playback engine.

`third_party/dependencies.json` records intended pins. **Windows runtime acquisition is not yet reproducible:** no verified DLL download URL/hash or complete transitive FFmpeg/libass build lock is invented. Obtain a trusted/audited runtime or build the pinned sources and all dependencies in your own reproducible toolchain. Verify its actual mpv/FFmpeg versions in diagnostics. Do not install a random rolling “latest” DLL and call it the pinned build. Review current security fixes before release; the M0 baseline is not a security endorsement of older releases.

Only the C API header is needed to compile. MpvApi resolves required exports at runtime, allowing MSVC Qt to consume a compatible C-ABI libmpv DLL without linking a MinGW import library. DLL dependencies and C runtimes must still be supplied correctly.

## Configure and build

Run from the repository in an x64 Visual Studio Developer PowerShell:

```powershell
$env:CMAKE_PREFIX_PATH = 'C:\Qt\6.8.3\msvc2022_64'
cmake --preset windows-msvc
cmake --build --preset windows-msvc
```

Qt is required at exactly 6.8.3 for the Windows application. Use x64 dependencies throughout. If CMake reports missing Qt or the compiler environment, fix setup and rerun; do not bypass dependencies or disable the player.

Place `libmpv-2.dll` and its reviewed dependent DLLs beside `build\windows-msvc\jadzumi.exe`. The binary must export client API major 2. Do not copy unrelated DLLs or mpv user scripts/config. The app uses an absolute path for libmpv and restricts dependent DLL search to the application directory and System32.

Deploy Qt for local testing:

```powershell
& 'C:\Qt\6.8.3\msvc2022_64\bin\windeployqt.exe' --qmldir .\qml --compiler-runtime .\build\windows-msvc\jadzumi.exe
.\build\windows-msvc\jadzumi.exe 'C:\Videos\local-test.mp4'
```

If startup fails before a window appears, check the DLL architecture and missing transitive dependencies using an appropriate Windows dependency inspection tool. If QML or mpv initialization fails, inspect the local log or the visible engine error. Preferences use QSettings under `HKEY_CURRENT_USER\Software\Jadzumi\Jadzumi Player Windows`. Logs are under Qt's AppLocalDataLocation, typically `%LOCALAPPDATA%\Jadzumi\Jadzumi Player Windows\logs`, capped at two approximately 1 MiB files.

## Test on Windows

Generate a 12-second synthetic video plus audio, then point tests at actual absolute paths:

```powershell
ffmpeg -hide_banner -loglevel error -y -f lavfi -i 'testsrc2=size=640x360:rate=24' -f lavfi -i 'sine=frequency=440:sample_rate=48000' -t 12 -c:v mpeg4 -q:v 4 -c:a aac -shortest .\build\windows-msvc\fixture.mp4
$env:JADZUMI_TEST_MPV = (Resolve-Path .\build\windows-msvc\libmpv-2.dll).Path
$env:JADZUMI_TEST_MEDIA = (Resolve-Path .\build\windows-msvc\fixture.mp4).Path
ctest --preset windows-msvc
```

The integration tests deliberately use `vo=null`, `ao=null`, `hwdec=no`. They verify demux/decode, playback state, controls and lifecycle, **not visible output, hardware decoding or audible A/V sync**. Run the full manual matrix in TESTING.md with the actual Windows application.

## Portable development handoff

`packaging/Stage-M0.ps1` stages the executable, audited mpv DLL folder, runtime hashes, notices and Qt deployment into a development ZIP. This is a staging helper, not release certification. Installer/uninstaller, final icon, signing, associations, full source/license obligations and release security review belong to M6. Never distribute the candidate until the actual binary set has been reviewed.

## Portable backend validation on Linux

Qt Core/Test >=6.4 and a client-API-2 libmpv runtime can test the shared backend:

```sh
cmake -S . -B build/backend -DJADZUMI_BUILD_APP=OFF -DBUILD_TESTING=ON
cmake --build build/backend
sh tests/make-fixture.sh build/backend/fixture.mp4
export JADZUMI_TEST_MPV=/absolute/path/to/libmpv.so.2
export JADZUMI_TEST_MEDIA="$PWD/build/backend/fixture.mp4"
ctest --test-dir build/backend --output-on-failure
```

This lower Qt version is solely for backend compatibility testing and cannot build/run the Qt 6.8 WindowContainer application.
