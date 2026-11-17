#!/usr/bin/env python3
# SPDX-License-Identifier: CC-BY-SA-4.0
"""Generates the LUMEN icon theme (original artwork).

App icons follow the modern desktop icon grid: a 1024×1024 canvas holding an
824×824 superellipse ("squircle", n=5) with layered gradients, a specular
top sheen and a thin glass rim. Glyphs are composed from the vendored symbol
set (themes/symbols, MIT). Run:

    python3 themes/icons/generate_icons.py
"""
import math
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
SYMBOLS = os.path.join(ROOT, 'themes', 'symbols')
OUT = os.path.join(HERE, 'Lumen')


def squircle_path(cx, cy, r, n=5.0, steps=192):
    pts = []
    for i in range(steps):
        t = 2 * math.pi * i / steps
        c, s = math.cos(t), math.sin(t)
        x = cx + r * (abs(c) ** (2 / n)) * (1 if c >= 0 else -1)
        y = cy + r * (abs(s) ** (2 / n)) * (1 if s >= 0 else -1)
        pts.append(f'{x:.2f},{y:.2f}')
    return 'M' + ' L'.join(pts) + ' Z'


def glyph_paths(name, weight='fill'):
    path = os.path.join(SYMBOLS, weight, name + '.svg')
    with open(path) as f:
        svg = f.read()
    inner = re.search(r'<svg[^>]*>(.*)</svg>', svg, re.S).group(1)
    return inner


SQ = squircle_path(512, 512, 412)


def defs_common(uid, top, bottom, angle=90):
    x2, y2 = (0, 1) if angle == 90 else (1, 1)
    return f'''
    <linearGradient id="bg{uid}" x1="0" y1="0" x2="{x2}" y2="{y2}">
      <stop offset="0" stop-color="{top}"/><stop offset="1" stop-color="{bottom}"/>
    </linearGradient>
    <linearGradient id="sheen{uid}" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#ffffff" stop-opacity="0.38"/>
      <stop offset="0.45" stop-color="#ffffff" stop-opacity="0.06"/>
      <stop offset="1" stop-color="#ffffff" stop-opacity="0"/>
    </linearGradient>
    <linearGradient id="rim{uid}" x1="0" y1="0" x2="1" y2="1">
      <stop offset="0" stop-color="#ffffff" stop-opacity="0.75"/>
      <stop offset="0.5" stop-color="#ffffff" stop-opacity="0.08"/>
      <stop offset="1" stop-color="#ffffff" stop-opacity="0.45"/>
    </linearGradient>
    <filter id="gs{uid}" x="-20%" y="-20%" width="140%" height="140%">
      <feDropShadow dx="0" dy="14" stdDeviation="16" flood-color="#000" flood-opacity="0.28"/>
    </filter>
    <clipPath id="clip{uid}"><path d="{SQ}"/></clipPath>'''


def app_icon(uid, top, bottom, glyph, glyph_fill='#ffffff', scale=0.56, weight='fill', extra='', glyph_dy=0, angle=90,
             glyph_gradient=None):
    size = 1024 * scale
    off = (1024 - size) / 2
    gfill = glyph_fill
    gdef = ''
    if glyph_gradient:
        gdef = f'''<linearGradient id="gg{uid}" x1="0" y1="0" x2="0" y2="1">
          <stop offset="0" stop-color="{glyph_gradient[0]}"/><stop offset="1" stop-color="{glyph_gradient[1]}"/></linearGradient>'''
        gfill = f'url(#gg{uid})'
    glyph_svg = ''
    if glyph:
        glyph_svg = f'''<g filter="url(#gs{uid})" fill="{gfill}">
      <g transform="translate({off:.1f},{off + glyph_dy:.1f}) scale({size / 256:.4f})">{glyph_paths(glyph, weight).replace('fill="currentColor"', '')}</g></g>'''
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1024 1024">
  <defs>{defs_common(uid, top, bottom, angle)}{gdef}</defs>
  <g>
    <path d="{SQ}" fill="url(#bg{uid})"/>
    <g clip-path="url(#clip{uid})">
      {extra}
      <ellipse cx="512" cy="230" rx="520" ry="300" fill="url(#sheen{uid})"/>
    </g>
    {glyph_svg}
    <path d="{SQ}" fill="none" stroke="url(#rim{uid})" stroke-width="6"/>
  </g>
</svg>
'''


def calendar_icon(uid):
    head = '<rect x="100" y="100" width="824" height="250" fill="#FF3B30"/>'
    dots = ''
    for row in range(3):
        for col in range(5):
            x = 250 + col * 131
            y = 470 + row * 120
            color = '#FF3B30' if (row, col) == (1, 2) else '#3A3A3C'
            dots += f'<circle cx="{x}" cy="{y}" r="30" fill="{color}" fill-opacity="{1 if color != "#3A3A3C" else 0.75}"/>'
    rings = ''.join(f'<rect x="{x}" y="160" width="40" height="130" rx="20" fill="#ffffff" fill-opacity="0.9"/>' for x in (330, 654))
    return app_icon(uid, '#FFFFFF', '#ECECEF', None, extra=head + rings + dots)


def editor_icon(uid):
    lines = ''.join(f'<rect x="250" y="{360 + i * 90}" width="{520 if i % 3 != 2 else 340}" height="26" rx="13" fill="#8E8E93" fill-opacity="0.55"/>' for i in range(5))
    margin = '<rect x="200" y="100" width="10" height="824" fill="#FF6B6B" fill-opacity="0.5"/>'
    pencil = f'''<g transform="translate(560,560) rotate(-45)">
        <rect x="-40" y="-260" width="80" height="380" rx="14" fill="#FFB340"/>
        <rect x="-40" y="-260" width="80" height="70" rx="14" fill="#FF6B6B"/>
        <path d="M-40,120 L40,120 L0,200 Z" fill="#F2D3A2"/><path d="M-12,176 L12,176 L0,200 Z" fill="#3A3A3C"/></g>'''
    return app_icon(uid, '#FFFFFF', '#F1F1F4', None, extra=margin + lines + pencil)


def terminal_icon(uid):
    prompt = '''<g fill="none" stroke="#E5E5EA" stroke-width="54" stroke-linecap="round" stroke-linejoin="round">
        <path d="M290,400 L430,510 L290,620"/></g>
        <rect x="470" y="600" width="240" height="54" rx="27" fill="#30D158"/>
        <rect x="100" y="100" width="824" height="120" fill="#ffffff" fill-opacity="0.07"/>'''
    return app_icon(uid, '#3A3B42', '#0D0E12', None, extra=prompt)


def activity_icon(uid):
    grid = ''.join(f'<rect x="100" y="{260 + i * 140}" width="824" height="4" fill="#ffffff" fill-opacity="0.08"/>' for i in range(5))
    line = '<path d="M180,620 L320,620 L380,460 L460,760 L560,330 L640,640 L700,560 L850,560" fill="none" stroke="#30D158" stroke-width="40" stroke-linejoin="round" stroke-linecap="round"/>'
    return app_icon(uid, '#2C2C30', '#0B0B0D', None, extra=grid + line)


def calculator_icon(uid):
    keys = ''
    colors = ['#636366', '#636366', '#636366', '#FF9F0A']
    for r in range(4):
        for c in range(4):
            x = 205 + c * 160
            y = 300 + r * 160
            keys += f'<circle cx="{x + 57}" cy="{y + 57}" r="62" fill="{colors[c] if r else ("#D4D4D2" if c < 3 else "#FF9F0A")}"/>'
    return app_icon(uid, '#2B2B2E', '#0F0F11', None, extra=keys)


def finder_icon(uid):
    # Two-tone split face of light: an original "lens" motif in a folder-blue field.
    split = '<path d="M512,100 C470,400 560,620 512,924 L924,924 L924,100 Z" fill="#0A4BD6"/>'
    lens = '''<circle cx="512" cy="512" r="210" fill="none" stroke="#ffffff" stroke-width="44"/>
        <circle cx="560" cy="470" r="64" fill="#ffffff"/>
        <path d="M660,660 L760,760" stroke="#ffffff" stroke-width="56" stroke-linecap="round"/>'''
    return app_icon(uid, '#3FB2FF', '#1F7BFF', None, extra=split + lens, angle=45)


def store_icon(uid):
    return app_icon(uid, '#2FC8FF', '#0A5BFF', 'shopping-bag', scale=0.52, angle=45)


def trash_icon(full=False):
    paper = ''
    if full:
        paper = '''<g>
          <path d="M360,300 L470,180 L600,250 L520,330 Z" fill="#ffffff" stroke="#d0d0d4" stroke-width="6"/>
          <path d="M520,320 L640,210 L720,300 L620,360 Z" fill="#f4f4f6" stroke="#d0d0d4" stroke-width="6"/>
          <path d="M300,330 C380,250 460,300 560,260 L600,340 L330,370 Z" fill="#e9e9ee" stroke="#cfcfd4" stroke-width="6"/>
        </g>'''
    ribs = ''.join(f'<path d="M{x},380 L{x + (x - 512) * 0.06:.0f},860" stroke="#ffffff" stroke-opacity="0.55" stroke-width="10" stroke-linecap="round"/>' for x in (380, 446, 512, 578, 644))
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1024 1024">
  <defs>
    <linearGradient id="body" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0" stop-color="#C9CBD3" stop-opacity="0.85"/>
      <stop offset="0.35" stop-color="#F4F5F8" stop-opacity="0.75"/>
      <stop offset="0.65" stop-color="#E4E6EC" stop-opacity="0.7"/>
      <stop offset="1" stop-color="#B9BCC6" stop-opacity="0.9"/>
    </linearGradient>
    <linearGradient id="rim" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#ffffff"/><stop offset="1" stop-color="#D5D7DE"/>
    </linearGradient>
    <filter id="sh" x="-20%" y="-20%" width="140%" height="140%"><feDropShadow dx="0" dy="18" stdDeviation="20" flood-color="#000" flood-opacity="0.3"/></filter>
  </defs>
  <g filter="url(#sh)">
    {paper}
    <path d="M300,360 L724,360 L680,880 Q676,912 640,912 L384,912 Q348,912 344,880 Z" fill="url(#body)" stroke="#ffffff" stroke-opacity="0.8" stroke-width="6"/>
    {ribs}
    <rect x="270" y="320" width="484" height="70" rx="35" fill="url(#rim)" stroke="#C0C3CB" stroke-width="4"/>
  </g>
</svg>
'''


FOLDER_GLYPHS = {
    'folder': None, 'folder-blue': None, 'inode-directory': None,
    'folder-documents': 'file-text', 'folder-download': 'download-simple', 'folder-downloads': 'download-simple',
    'folder-pictures': 'image', 'folder-music': 'music-notes', 'folder-videos': 'film-strip',
    'folder-desktop': 'desktop', 'user-desktop': 'desktop', 'user-home': 'house', 'folder-home': 'house',
    'folder-applications': 'squares-four', 'folder-cloud': 'cloud', 'folder-publicshare': 'share-network',
    'folder-templates': 'stack', 'folder-open': None,
}


def folder_icon(glyph):
    g = ''
    if glyph:
        g = f'''<g fill="#2E7FD9" fill-opacity="0.75" transform="translate(372,430) scale(1.09)">{glyph_paths(glyph, "fill").replace('fill="currentColor"', '')}</g>'''
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1024 1024">
  <defs>
    <linearGradient id="back" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#5DB4F5"/><stop offset="1" stop-color="#3D97E6"/></linearGradient>
    <linearGradient id="front" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#8FD0FF"/><stop offset="1" stop-color="#66B8F7"/></linearGradient>
    <filter id="sh" x="-20%" y="-20%" width="140%" height="140%"><feDropShadow dx="0" dy="14" stdDeviation="16" flood-color="#0a3a70" flood-opacity="0.28"/></filter>
  </defs>
  <g filter="url(#sh)">
    <path d="M120,230 Q120,190 160,190 L400,190 Q430,190 450,215 L480,255 L864,255 Q904,255 904,295 L904,800 Q904,840 864,840 L160,840 Q120,840 120,800 Z" fill="url(#back)"/>
    <path d="M110,330 Q110,300 140,300 L884,300 Q914,300 914,330 L904,812 Q902,852 862,852 L162,852 Q122,852 120,812 Z" fill="url(#front)"/>
    <path d="M110,330 Q110,300 140,300 L884,300 Q914,300 914,330" fill="none" stroke="#ffffff" stroke-opacity="0.7" stroke-width="6"/>
    {g}
  </g>
</svg>
'''


APPS = {
    'lumen-finder': lambda u: finder_icon(u),
    'system-file-manager': lambda u: finder_icon(u),
    'lumen-settings': lambda u: app_icon(u, '#9EA2AA', '#4F535B', 'gear-six', scale=0.62, glyph_gradient=('#F2F2F4', '#C9CBD0')),
    'preferences-system': lambda u: app_icon(u, '#9EA2AA', '#4F535B', 'gear-six', scale=0.62, glyph_gradient=('#F2F2F4', '#C9CBD0')),
    'lumen-terminal': lambda u: terminal_icon(u),
    'utilities-terminal': lambda u: terminal_icon(u),
    'lumen-store': lambda u: store_icon(u),
    'lumen-activity-monitor': lambda u: activity_icon(u),
    'utilities-system-monitor': lambda u: activity_icon(u),
    'lumen-calculator': lambda u: calculator_icon(u),
    'accessories-calculator': lambda u: calculator_icon(u),
    'lumen-calendar': lambda u: calendar_icon(u),
    'office-calendar': lambda u: calendar_icon(u),
    'lumen-texteditor': lambda u: editor_icon(u),
    'accessories-text-editor': lambda u: editor_icon(u),
    'lumen-imageviewer': lambda u: app_icon(u, '#FF7AA8', '#FF9E3D', 'image', scale=0.56, angle=45),
    'lumen-mediaplayer': lambda u: app_icon(u, '#FF4F7A', '#B44CF2', 'play', scale=0.50, angle=45),
    'lumen-screenshot': lambda u: app_icon(u, '#76777D', '#38393E', 'selection', weight='bold', scale=0.56),
    'lumen-installer': lambda u: app_icon(u, '#7FB2FF', '#8A5CFF', 'lumen-logo', scale=0.58, angle=45),
    'lumen-logo': lambda u: app_icon(u, '#20264A', '#0B0D1E', 'lumen-logo', scale=0.6, glyph_gradient=('#9FC3FF', '#C78BFF')),
    'start-here': lambda u: app_icon(u, '#20264A', '#0B0D1E', 'lumen-logo', scale=0.6, glyph_gradient=('#9FC3FF', '#C78BFF')),
    'preferences-desktop-notification': lambda u: app_icon(u, '#FF5C5C', '#E0302F', 'bell', scale=0.54),
    'help-browser': lambda u: app_icon(u, '#50C0FF', '#1478FF', 'question', scale=0.56),
}


def main():
    apps_dir = os.path.join(OUT, 'scalable', 'apps')
    places_dir = os.path.join(OUT, 'scalable', 'places')
    os.makedirs(apps_dir, exist_ok=True)
    os.makedirs(places_dir, exist_ok=True)
    for i, (name, fn) in enumerate(sorted(APPS.items())):
        with open(os.path.join(apps_dir, name + '.svg'), 'w') as f:
            f.write(fn(str(i)))
    for name, glyph in FOLDER_GLYPHS.items():
        with open(os.path.join(places_dir, name + '.svg'), 'w') as f:
            f.write(folder_icon(glyph))
    for name, full in (('user-trash', False), ('user-trash-full', True)):
        with open(os.path.join(places_dir, name + '.svg'), 'w') as f:
            f.write(trash_icon(full))
    with open(os.path.join(OUT, 'index.theme'), 'w') as f:
        f.write('''[Icon Theme]
Name=Lumen
Comment=LUMEN OS icon theme
Inherits=breeze,hicolor
Example=lumen-finder
DisplayDepth=32
Directories=scalable/apps,scalable/places

[scalable/apps]
Size=128
MinSize=16
MaxSize=1024
Context=Applications
Type=Scalable

[scalable/places]
Size=128
MinSize=16
MaxSize=1024
Context=Places
Type=Scalable
''')
    print('icons written to', OUT)


if __name__ == '__main__':
    main()
