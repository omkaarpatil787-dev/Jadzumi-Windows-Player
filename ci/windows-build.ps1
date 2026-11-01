$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
Set-Location $root
$qt = 'C:/Qt/6.8.3/msvc2022_64'
cmake -S . -B build/windows -G 'Visual Studio 17 2022' -A x64 "-DCMAKE_PREFIX_PATH=$qt" -DJADZUMI_BUILD_APP=ON -DBUILD_TESTING=ON
if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed.' }
cmake --build build/windows --config Release --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Windows compilation failed.' }
$download = Join-Path $root 'build/libmpv-lgpl.zip'
$url = 'https://github.com/mpv-player/mpv/releases/download/git-release/libmpv-v0.41.0-dev-g9e2ed65eb-41103006513-x86_64-w64-mingw32-lgpl.zip'
Invoke-WebRequest $url -OutFile $download
if ((Get-FileHash $download -Algorithm SHA256).Hash.ToLowerInvariant() -ne '15a2dcd9c4649a6ca73e53ad6b07ad6470ba6354e414f6c1ef54b6f21bd621e3') { throw 'libmpv digest mismatch.' }
Expand-Archive $download build/mpv -Force
$mpv = Get-ChildItem build/mpv -Filter libmpv-2.dll -Recurse | Select-Object -First 1
if (!$mpv) { throw 'Missing libmpv DLL in verified archive.' }
$bin = Join-Path $root 'build/windows/Release'
Copy-Item $mpv.FullName $bin
& "$qt/bin/windeployqt.exe" --release --qmldir "$root/qml" --compiler-runtime "$bin/jadzumi.exe"
if ($LASTEXITCODE -ne 0) { throw 'Qt application deployment failed.' }
& "$qt/bin/windeployqt.exe" --release --compiler-runtime "$bin/playback_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Qt test deployment failed.' }
python tests/make_fixture.py build/windows/fixture.avi
if ($LASTEXITCODE -ne 0) { throw 'Fixture creation failed.' }
$env:JADZUMI_TEST_MPV = "$bin/libmpv-2.dll"
$env:JADZUMI_TEST_MEDIA = "$root/build/windows/fixture.avi"
ctest --test-dir build/windows -C Release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Windows playback integration tests failed.' }
New-Item -ItemType Directory dist/Jadzumi-Player -Force | Out-Null
Copy-Item "$bin/*" dist/Jadzumi-Player -Recurse -Force
Remove-Item dist/Jadzumi-Player/playback_tests.exe -ErrorAction SilentlyContinue
Remove-Item dist/Jadzumi-Player/*.pdb -ErrorAction SilentlyContinue
New-Item -ItemType Directory dist/Jadzumi-Player/licenses/mpv -Force | Out-Null
Copy-Item build/mpv/LICENSE.LGPL dist/Jadzumi-Player/licenses/mpv/
Copy-Item LICENSES dist/Jadzumi-Player/licenses/project -Recurse
Copy-Item docs dist/Jadzumi-Player/ -Recurse
Copy-Item README.md dist/Jadzumi-Player/
Copy-Item third_party/dependencies.json dist/Jadzumi-Player/
Compress-Archive dist/Jadzumi-Player dist/Jadzumi-Player-Windows-x64.zip -Force
