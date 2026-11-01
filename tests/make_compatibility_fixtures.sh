#!/usr/bin/env sh
set -eu
fixture_dir="$(dirname "$0")/fixtures"
task_tmp="$(mktemp -d)"
trap 'rm -rf "$task_tmp"' EXIT
mkdir -p "$fixture_dir"
cat > "$task_tmp/subtitles.srt" <<'SRT'
1
00:00:00,200 --> 00:00:05,500
Jadzumi M2 subtitle fixture

2
00:00:06,000 --> 00:00:11,800
Second chapter
SRT
cat > "$task_tmp/chapters.ffmetadata" <<'META'
;FFMETADATA1
title=Jadzumi synthetic compatibility fixture
[CHAPTER]
TIMEBASE=1/1000
START=0
END=6000
title=Opening
[CHAPTER]
TIMEBASE=1/1000
START=6000
END=12000
title=Second chapter
META
ffmpeg -hide_banner -loglevel error -y -f lavfi -i testsrc2=size=160x90:rate=24 \
 -f lavfi -i sine=frequency=440:sample_rate=48000 -f lavfi -i anullsrc=channel_layout=5.1:sample_rate=48000 \
 -i "$task_tmp/subtitles.srt" -i "$task_tmp/chapters.ffmetadata" \
 -map 0:v -map 1:a -map 2:a -map 3:s -map_metadata 4 -map_chapters 4 -t 12 \
 -c:v libx264 -preset veryfast -threads 1 -crf 28 -c:a:0 aac -b:a:0 64k -c:a:1 ac3 -b:a:1 192k -c:s srt \
 -metadata:s:a:0 title=Tone -metadata:s:a:0 language=eng -metadata:s:a:1 title=Surround \
 -metadata:s:a:1 language=hin -metadata:s:s:0 language=eng "$fixture_dir/multitrack.mkv"
ffmpeg -hide_banner -loglevel error -y -f lavfi -i testsrc2=size=160x90:rate=24 -t 3 \
 -vf format=yuv420p10le -c:v libx265 -preset ultrafast -x265-params pools=none:frame-threads=1:log-level=error \
 -crf 32 "$fixture_dir/hevc-main10.mkv"
