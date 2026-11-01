# Repository setup

Official repository: https://github.com/omkaarpatil787-dev/Jadzumi-Windows-Player

The owner created this public repository on 2026-11-01. Its existing `.gitattributes` and initial commit are preserved. M0 source is added on `main`; no Android project is modified.

```powershell
git clone https://github.com/omkaarpatil787-dev/Jadzumi-Windows-Player.git
cd Jadzumi-Windows-Player
```

Continue with [BUILD-WINDOWS.md](BUILD-WINDOWS.md), then run the automated tests and manual acceptance checklist in [TESTING.md](TESTING.md). The source includes no runtime DLLs, test media, build caches or signing credentials.

The source being public does not select an open-source license or certify any binary distribution. Project licensing and the exact bundled runtime's license/security review remain unresolved; see [LICENSES/README.md](../LICENSES/README.md).

`packaging/Create-GitHubRepository.ps1` is an optional helper for creating a separate private repository from a repository with no remote. It is not needed for this existing repository and refuses to replace configured remotes.
