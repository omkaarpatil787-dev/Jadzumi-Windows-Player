# Third-party notices and release gate

This repository contains no Qt, FFmpeg or libmpv binaries. It vendors only the official `mpv/client.h` from mpv v0.40.0, including the complete permissive ISC-style copyright/license notice at the top of that file. The header's license is distinct from the full libmpv library's configured license.

The publicly visible application source has no general redistribution license selected yet; retain creator attribution and resolve project licensing before a general open-source release. Do not infer MIT, GPL or LGPL terms for the application from an unrelated Android repository.

Before M6 distribution, capture the actual runtime source revisions, build flags, linked libraries, license texts and source/relinking obligations. Qt may be used under LGPLv3/GPL or commercial terms depending on modules/arrangement; libmpv's license depends on build options and dependencies; FFmpeg may be LGPL/GPL depending on configuration and nonfree components can prohibit redistribution. libass and every transitive dependency require notices. Codec patent obligations are a separate review; do not promise that shipping a codec is automatically lawful everywhere.

The packaging helper copies a supplied runtime notices folder and hashes actual DLLs but cannot decide legal compliance. Provide the complete applicable license/source materials and satisfy all obligations before release. The final audit remains mandatory and unresolved.
