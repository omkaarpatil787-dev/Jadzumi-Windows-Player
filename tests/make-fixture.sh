#!/usr/bin/env sh
set -eu
# Synthetic local video + audio; no copyrighted sample downloads.
ffmpeg -hide_banner -loglevel error -y \
  -f lavfi -i 'testsrc2=size=640x360:rate=24' \
  -f lavfi -i 'sine=frequency=440:sample_rate=48000' \
  -t 12 -c:v mpeg4 -q:v 4 -c:a aac -shortest "${1:-fixture.mp4}"
