#!/usr/bin/env python3
# Renders Plymouth assets from the LUMEN logo (run at build time).
import os, subprocess, sys
from PIL import Image, ImageDraw, ImageFilter
out = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.abspath(__file__))
root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
svg = os.path.join(root, 'themes', 'symbols', 'fill', 'lumen-logo.svg')
data = open(svg).read().replace('currentColor', '#ffffff')
tmp = os.path.join(out, '.logo.svg'); open(tmp, 'w').write(data)
subprocess.check_call(['rsvg-convert', '-w', '160', '-h', '160', tmp, '-o', os.path.join(out, 'logo.png')])
os.remove(tmp)
logo = Image.open(os.path.join(out, 'logo.png')).convert('RGBA')
g = Image.new('RGBA', (360, 360), (0, 0, 0, 0))
tint = Image.new('RGBA', logo.size, (150, 170, 255, 255)); tint.putalpha(logo.getchannel('A'))
g.paste(tint, (100, 100), tint)
g = g.filter(ImageFilter.GaussianBlur(28))
g.save(os.path.join(out, 'glow.png'))
def capsule(name, w, h, rgba):
    im = Image.new('RGBA', (w, h), (0, 0, 0, 0))
    ImageDraw.Draw(im).rounded_rectangle((0, 0, w - 1, h - 1), radius=h // 2, fill=rgba)
    im.save(os.path.join(out, name))
capsule('track.png', 220, 6, (255, 255, 255, 255))
capsule('fill.png', 220, 6, (255, 255, 255, 255))
capsule('field.png', 260, 36, (255, 255, 255, 46))
print('plymouth assets written to', out)
