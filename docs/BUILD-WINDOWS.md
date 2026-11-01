# Windows build and package

## Tested toolchain

- Windows x64 with Visual Studio 2022 C++ Desktop workload and Windows SDK.
- Qt **6.8.3**, `msvc2022_64`, dynamically linked: Core/Gui/QML/Quick/Controls/Shapes/SQL.
- CMake **3.31.6**; the CI generator is Visual Studio 17 2022.
- Exact upstream **LGPL** libmpv development archive, source commit and SHA-256 recorded in `third_party/dependencies.json`. Do not substitute a moving “latest” binary.

The source keeps the official v0.40.0 public client header and resolves compatible client-API-major-2 functions dynamically. The packaged runtime is the explicitly recorded newer snapshot. FFmpeg/libass are supplied by that upstream runtime, not a separate replacement engine.

## Reproduce native CI

From the source root in PowerShell, with Visual Studio installed:

```powershell
python -m pip install aqtinstall==3.3.0 cmake==3.31.6
python -m aqt install-qt windows desktop 6.8.3 win64_msvc2022_64 -O C:/Qt --archives qtbase qtdeclarative qtsvg qtshadertools
./ci/windows-build.ps1
```

The script configures/builds the app and tests, verifies/extracts libmpv, deploys Qt and the MSVC redistributable CRT DLLs, generates real synthetic media, runs the integration suite and then the native UI/playback smoke test. It refuses to package after a failed check. Test evidence is written under `dist/test-evidence`; the portable folder/ZIP are under `dist`.

For a normal developer build, the existing `windows-msvc` CMake preset uses Ninja from an x64 developer shell. Set `CMAKE_PREFIX_PATH` to the same Qt kit, configure/build, then deploy the pinned runtime and Qt as done in the CI script.

## Run the development application

Extract the complete portable folder and launch `jadzumi.exe`. Keep all DLLs and plugin/QML directories beside it. No SDK or Python is needed on the playback computer. The EXE is unsigned; code signing is a later release task.

Application preferences use QSettings under the Jadzumi organization/application name. History is stored in Qt's AppLocalDataLocation as `playback.sqlite`; local logs are capped at two approximately 1 MiB files. On Windows the data directory is typically `%LOCALAPPDATA%\Jadzumi\Jadzumi Player Windows`. Screenshots go to `Pictures\Jadzumi Player`.

## Tests without the full Windows UI

Qt Core/Gui/SQL/Test >=6.4 can compile the portable backend; the UI test host requires exactly Qt 6.8.3:

```sh
cmake -S . -B build/backend -DJADZUMI_BUILD_APP=OFF -DBUILD_TESTING=ON
cmake --build build/backend
python3 tests/make_fixture.py build/backend/fixture.avi
export JADZUMI_TEST_MPV=/absolute/path/to/libmpv.so.2
export JADZUMI_TEST_MEDIA="$PWD/build/backend/fixture.avi"
ctest --test-dir build/backend --output-on-failure
```

The committed compatibility fixtures are synthetic; their generation is documented in `tests/fixtures/README.md`. Backend tests use null presentation outputs but real demuxing/decoding. They do not prove visible rendering or audible synchronization.

`JADZUMI_BUILD_UI_TEST_HOST=ON` builds `ui_smoke_host` from the same QML and real controller code for local software/offscreen layout checks. It is never distributed as the Windows player or used to claim native D3D11 validation.

## Release limits

See TESTING.md, KNOWN-ISSUES.md and M2-REPORT.md. HDR output stays SDR until M4 verification. Physical target-GPU performance, hybrid adapters, multi-monitor behavior, long playback and audible A/V sync are not established by a hosted Windows runner. Final dependency security, source/license completeness, signing and stable release packaging remain M6 requirements.
