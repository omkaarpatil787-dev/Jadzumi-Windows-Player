# Synthetic compatibility fixtures

These files were generated for this project with FFmpeg from `testsrc2`, sine-wave and silent audio sources. They contain no third-party film or music content.

- `multitrack.mkv`: 160×90 H.264 at 24 fps, 12 seconds; AAC tone track, AC-3 5.1 silent track, embedded English SRT and two six-second chapters.
- `hevc-main10.mkv`: 160×90 HEVC Main10 at 24 fps, three seconds; 10-bit SDR input. This is not an HDR correctness test.

The raw AVI used by CI is independently generated with Python's standard library by `tests/make_fixture.py`.

Reproduction commands are in `tests/make_compatibility_fixtures.sh`. FFmpeg is a test-generation tool and is not a second playback engine in Jadzumi.
