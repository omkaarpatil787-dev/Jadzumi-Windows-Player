# Known limitations

1. Windows executable not compiled here; no Windows SDK, compiler, display session or target GPU. D3D11 embedding and Qt 6.8.3 application integration require local verification.
2. Exact audited Windows libmpv/FFmpeg/libass binary artifacts and their transitive dependency hashes are not available. Intended source pins do not constitute a reproducible runtime distribution.
3. Native WindowContainer stacking prevents normal in-video QML overlays. M0 keeps controls in an external strip. M1/M2 immersive design must resolve this explicitly rather than placing invisible controls over the HWND.
4. Native child-window mouse/focus/keyboard forwarding, including double-click, requires Windows testing. Fullscreen button and shortcuts are implemented; their runtime behavior is unverified.
5. Output is intentionally SDR-only for M0. Windows display HDR capability, active HDR state and color correctness are not verified. Dolby Vision is not advertised.
6. Diagnostics show only observed mpv properties. Missing values are unavailable; CPU/GPU utilization, VRAM, actual GPU adapter and end-to-end output latency are not measured.
7. Local file opening accepts a conservative video-extension list and rejects explicit URLs/network-share paths/playlists. Network-mapped drives and link traversal have not had a Windows privacy audit. External stream references are disabled. No drag/drop or library scanning in M0.
8. Only volume and logical window size persist. Remembered position, customization, resume history, themes, subtitles/audio panels, media-session integration and file associations remain later work.
9. No installer, release signing or final icon. Development ZIP helper is untested on Windows. The application loads native DLLs; source provenance and license/security review are required before distributing a runtime.
10. `terminate_destroy` can block while mpv joins workers. Long playback, slow storage, hybrid GPU, resource leak and paused-idle profiling remain untested.
11. Source distribution licensing has not been chosen by Omkar Patil. Third-party rights are separate. Nothing here is a final codec/legal compliance certification.
12. Automatic renderer replacement is not implemented. Rendering errors remain visible and block acceptance. Software decoder selection is available; it cannot repair a broken GPU output path.
