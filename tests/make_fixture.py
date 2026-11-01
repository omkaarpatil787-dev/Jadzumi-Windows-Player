"""Generate a tiny raw RGB/PCM AVI with the Python standard library; no downloads."""
import math
import struct
import sys
from pathlib import Path


def chunk(tag, body):
    return tag + struct.pack('<I', len(body)) + body + (b'\0' if len(body) % 2 else b'')


def listing(tag, body):
    return chunk(b'LIST', tag + body)


def write_fixture(destination):
    width, height, fps, duration, rate = 160, 90, 24, 12, 48000
    frames, frame_size = fps * duration, width * height * 3
    avih = struct.pack('<14I', 1000000 // fps, frame_size * fps + rate * 2, 0, 0x10,
                       frames, 0, 2, frame_size, width, height, 0, 0, 0, 0)
    def strh(kind, handler, scale, speed, length, buffer, sample_size, rectangle):
        return struct.pack('<4s4sIHH8I4h', kind, handler, 0, 0, 0, 0, scale, speed,
                           0, length, buffer, 0xFFFFFFFF, sample_size, *rectangle)
    video = listing(b'strl', chunk(b'strh', strh(b'vids', b'DIB ', 1, fps, frames,
                    frame_size, 0, (0, 0, width, height))) + chunk(b'strf', struct.pack(
                    '<IiiHHIIiiII', 40, width, height, 1, 24, 0, frame_size, 0, 0, 0, 0)))
    audio = listing(b'strl', chunk(b'strh', strh(b'auds', b'\0' * 4, 2, rate * 2,
                    rate * duration, rate // fps * 2, 2, (0, 0, 0, 0))) +
                    chunk(b'strf', struct.pack('<HHIIHH', 1, 1, rate, rate * 2, 2, 16)))
    header = listing(b'hdrl', chunk(b'avih', avih) + video + audio)
    data, index, offset = bytearray(), bytearray(), 4
    for frame in range(frames):
        rgb = bytes(channel for y in range(height) for x in range(width)
                    for channel in ((x + frame) % 256, (y * 2) % 256, 90))
        samples = b''.join(struct.pack('<h', int(2000 * math.sin(2 * math.pi * 440 * n / rate)))
                           for n in range(frame * rate // fps, (frame + 1) * rate // fps))
        for tag, raw in [(b'00db', rgb), (b'01wb', samples)]:
            encoded = chunk(tag, raw)
            index += struct.pack('<4sIII', tag, 0x10, offset, len(raw))
            data += encoded
            offset += len(encoded)
    body = b'AVI ' + header + listing(b'movi', data) + chunk(b'idx1', index)
    path = Path(destination)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(chunk(b'RIFF', body))
    print(path, path.stat().st_size)


if __name__ == '__main__':
    write_fixture(sys.argv[1])
