[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$MpvRuntime,
    [Parameter(Mandatory=$true)][string]$QtBin,
    [string]$BuildDirectory = 'build/windows-msvc'
)
$ErrorActionPreference = 'Stop'
$repository = Split-Path $PSScriptRoot -Parent
$runtimePath = (Resolve-Path $MpvRuntime).Path
$qtPath = (Resolve-Path $QtBin).Path
$buildPath = (Resolve-Path (Join-Path $repository $BuildDirectory)).Path
$executable = Join-Path $buildPath 'jadzumi.exe'
if (!(Test-Path $executable)) { throw 'Build jadzumi.exe first.' }
if (!(Test-Path (Join-Path $runtimePath 'libmpv-2.dll'))) { throw 'Missing libmpv-2.dll.' }
if (!(Test-Path (Join-Path $runtimePath 'licenses'))) { throw 'Supply the actual runtime dependency notices in a licenses directory.' }
$deploy = Join-Path $qtPath 'windeployqt.exe'
if (!(Test-Path $deploy)) { throw 'QtBin must contain windeployqt.exe from the pinned x64 Qt kit.' }
$distRoot = Join-Path $repository 'dist'
$stage = Join-Path $distRoot 'Jadzumi-Player-Windows-M0'
if (Test-Path $stage) { throw 'Staging directory already exists. Choose whether to preserve/remove it manually.' }
New-Item -ItemType Directory -Path $stage -Force | Out-Null
Copy-Item $executable $stage
Get-ChildItem -LiteralPath $runtimePath -Filter '*.dll' -File | Copy-Item -Destination $stage
Copy-Item (Join-Path $runtimePath 'licenses') (Join-Path $stage 'runtime-licenses') -Recurse
Copy-Item (Join-Path $repository 'LICENSES') $stage -Recurse
Copy-Item (Join-Path $repository 'docs') $stage -Recurse
Copy-Item (Join-Path $repository 'README.md') $stage
& $deploy --qmldir (Join-Path $repository 'qml') --compiler-runtime (Join-Path $stage 'jadzumi.exe')
if ($LASTEXITCODE -ne 0) { throw 'windeployqt failed; staging left for inspection. No ZIP created.' }
$files = @(Get-ChildItem -LiteralPath $stage -Filter '*.dll' -Recurse -File | Sort-Object FullName | ForEach-Object {
    @{path=$_.FullName.Substring($stage.Length + 1); sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()}
})
@{schema=1; status='M0 development candidate; Windows acceptance and license review required'; binaries=$files} |
    ConvertTo-Json -Depth 5 | Set-Content (Join-Path $stage 'runtime-fingerprints.json') -Encoding UTF8
$zip = Join-Path $distRoot 'Jadzumi-Player-Windows-M0-dev.zip'
if (Test-Path $zip) { throw 'ZIP already exists; it will not be overwritten.' }
Compress-Archive -Path $stage -DestinationPath $zip
Write-Output "Staged development candidate: $zip"
Write-Output 'Record actual mpv/FFmpeg build versions, configuration, provenance and compliance materials before sharing.'
