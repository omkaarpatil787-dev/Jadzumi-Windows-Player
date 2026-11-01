# Vendored public C header

Unmodified `include/mpv/client.h` from official mpv v0.40.0:

https://raw.githubusercontent.com/mpv-player/mpv/v0.40.0/include/mpv/client.h

SHA-256: `3c073236a09cb456c6e80587d5523c8771c32fb22ca014e0f99cc3d43905b75c`

The original copyright/license notice is retained in the file. No playback implementation is stubbed: MpvApi loads a real client-API-major-2 runtime at execution. The application fails visibly when the DLL or a required export is missing. No import library or embedded fake engine is supplied.
