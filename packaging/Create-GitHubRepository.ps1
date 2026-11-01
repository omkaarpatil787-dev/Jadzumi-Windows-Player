[CmdletBinding()]
param([string]$RepositoryName = 'Jadzumi-Player-Windows')
$ErrorActionPreference = 'Stop'
$repository = Split-Path $PSScriptRoot -Parent
Push-Location $repository
try {
    git rev-parse --verify HEAD | Out-Null
    if ($LASTEXITCODE -ne 0) { throw 'Restore the bundled Git repository or initialize and commit this source first.' }
    $remotes = @(git remote)
    if ($remotes.Count -gt 0) { throw 'Repository already has a remote. Inspect it manually; this helper will not replace it.' }
    if ((git status --porcelain).Length -gt 0) { throw 'Commit or review local changes before pushing.' }
    gh auth status
    if ($LASTEXITCODE -ne 0) { throw 'Authenticate GitHub CLI with your own account first (gh auth login).' }
    gh repo create $RepositoryName --private --source . --remote origin --push
    if ($LASTEXITCODE -ne 0) { throw 'GitHub creation/push failed. Inspect gh output; do not assume the remote exists or was pushed.' }
} finally {
    Pop-Location
}
