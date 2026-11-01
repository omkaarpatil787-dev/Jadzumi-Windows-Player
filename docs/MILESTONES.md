# Milestone status

| Milestone | Status | Acceptance evidence / limit |
| --- | --- | --- |
| M0 Architecture and playback foundation | Native compilation and real-media backend tests passed | Physical audible A/V synchronization and target GPU remain separate verification |
| M1 Design foundation | Implemented and automated UI validation passed | Actual layouts inspected; native theme/fullscreen/visible-video smoke passed |
| M2 Complete playback | Implemented development build; Windows media/control and native UI tests passed | Portable x64 EXE; physical acceptance limits remain in M2-REPORT.md |
| M3 Media library | Not started | SQLite playback history exists, but folder scanning/indexing/posters/search do not |
| M4 Advanced playback/HDR | Not started | Output remains SDR; no verified HDR passthrough/device claims |
| M5 Performance/stability | Not started | Target hardware and long-run CPU/GPU/resource profiling required |
| M6 Security/licensing/release | Not started | Final audits, stable dependency selection, signing and release qualification required |

No motion interpolation, optical flow, frame generation or frame-rate conversion belongs to any milestone. Development packaging makes the M2 application runnable; it is not a release certification.
