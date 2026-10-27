"""Turn frames written by `PBDynamics --record <dir>` into an animated GIF.

Usage: python scripts/make_gif.py <frames_dir> docs/cloth.gif [width]
"""
import glob
import os
import sys

from PIL import Image

frames_dir, out = sys.argv[1], sys.argv[2]
width = int(sys.argv[3]) if len(sys.argv) > 3 else 480
paths = sorted(glob.glob(os.path.join(frames_dir, "frame_*.png")))
frames = []
for p in paths:
    im = Image.open(p).convert("RGB")
    im = im.resize((width, round(im.height * width / im.width)), Image.LANCZOS)
    frames.append(im.quantize(colors=128))
# Frames were saved every other simulation step of 1/60 s, i.e. 30 fps
frames[0].save(out, save_all=True, append_images=frames[1:], duration=33, loop=0, optimize=True)
print(f"{len(frames)} frames -> {out} ({os.path.getsize(out) / 1e6:.2f} MB)")
