# LUMEN OS — Design System

The visual target is the modern macOS "Liquid Glass" era desktop: a quiet,
content-first layout where controls float on layered, light-bending glass.
LUMEN reproduces those *proportions, materials and motion* with original
artwork, open fonts and its own code. No Apple artwork, fonts, icons, sounds
or trademarks are used or redistributed.

All values are in logical pixels (pt). The canonical machine-readable source is
`design-system/qml/Lumen/Design/Tokens.qml` (+ `Motion`, `Glass` modules);
this document explains them.

## 1. Typography

| Role            | Family                    | Size | Weight    | Tracking |
|-----------------|---------------------------|------|-----------|----------|
| Menu bar        | Inter Variable            | 13   | 500 / 700 (app name) | -0.08 |
| Body            | Inter Variable            | 13   | 400       | -0.08    |
| Callout         | Inter Variable            | 12   | 400       | 0        |
| Caption         | Inter Variable            | 11   | 400       | +0.06    |
| Headline        | Inter Variable            | 13   | 600       | -0.08    |
| Title 3         | Inter Variable            | 15   | 600       | -0.23    |
| Title 2         | Inter Variable            | 17   | 600       | -0.43    |
| Title 1         | Inter Variable            | 22   | 700       | -0.26    |
| Large title     | Inter Variable            | 26   | 700       | +0.22    |
| Clock widget    | Inter Variable (display)  | 56–96| 600       | -1.0     |
| Monospace       | JetBrains Mono            | 12–13| 400       | 0        |

Inter is the closest freely licensed match for the neo-grotesque system face
(similar x-height, tight default tracking). Tracking values replicate the
"optical size" tightening of the reference typography.

## 2. Geometry

| Element                       | Value                                         |
|-------------------------------|-----------------------------------------------|
| Menu bar height               | 24 (34 on HiDPI notched panels)               |
| Menu bar item padding         | 10 horizontal, 3 vertical, radius 6 highlight |
| Menu (dropdown) radius        | 12 outer, 6 item highlight, 5 inset           |
| Menu item height              | 22, separator 11 (1px line)                   |
| Dock tile size (default)      | 54 icon in 64 tile, magnified up to 96        |
| Dock padding / radius         | 6 inner, radius = (tile + 2·6) · 0.42         |
| Dock bottom gap               | 8                                             |
| Running indicator             | 4 × 4 dot, 3 below tile                       |
| Window corner radius          | 16 (title bar), 24 (unified toolbar)          |
| Title bar height              | 32; with toolbar 52                           |
| Traffic lights                | Ø 12, centre spacing 20, inset 14 from left   |
| Sidebar (inset, floating)     | width 220, inset 8, radius 18                  |
| Control Center                | width 324, padding 12, module radius 20, gap 12 |
| Spotlight                     | width 680, field height 54, radius 27 (capsule) |
| Notification banner           | width 352, radius 22, padding 14               |
| Widgets                       | small 164², medium 344 × 164, radius 22        |
| App icon grid (squircle)      | superellipse n = 5, 82 % content box           |

## 3. Color

### Semantic colors

| Token               | Light            | Dark             |
|---------------------|------------------|------------------|
| `label`             | #000000 · 85 %   | #FFFFFF · 85 %   |
| `secondaryLabel`    | #000000 · 50 %   | #FFFFFF · 55 %   |
| `tertiaryLabel`     | #000000 · 26 %   | #FFFFFF · 25 %   |
| `separator`         | #000000 · 10 %   | #FFFFFF · 12 %   |
| `windowBackground`  | #F5F5F7          | #1E1E20          |
| `contentBackground` | #FFFFFF          | #161618          |
| `controlFill`       | #000000 · 5 %    | #FFFFFF · 10 %   |
| `accent` (blue)     | #0A7CFF          | #0A84FF          |

Accent palette: blue `#0A84FF`, purple `#BF5AF2`, pink `#FF375F`, red `#FF453A`,
orange `#FF9F0A`, yellow `#FFD60A`, green `#30D158`, graphite `#8E8E93`.

Window controls: close `#FF5F57`, minimize `#FEBC2E`, zoom `#28C840`
(inactive windows: `#D0D0D2` light / `#4A4A4D` dark).

## 4. Liquid Glass materials

Liquid Glass is *clear* glass that bends light, not frosted glass that
blurs it. The surface is modelled as a thin slab lying on the backdrop whose
rounded edge is a convex bevel, and a glass surface is rendered in five
ordered layers (`shaders/glass.frag`):

1. **Backdrop** – the content behind the surface, captured once per window at
   full resolution (`BackdropBlur.sharp`) plus a dual-Kawase blur pyramid
   (`BackdropBlur.texture`). Each material's **frost** (0…1) mixes between the
   two: clear materials show the sharp backdrop, frosted ones the blur. The
   bevel always adds a little extra softening so the rim reads as thick glass.
2. **Refraction (lensing)** – inside the bevel (width *lens*) the surface
   height follows a convex squircle, `h(x) = T·(1 − (1 − x)⁴)^¼`, where `x` is
   the distance from the rim across the bevel and `T` the glass thickness
   (*refr*). Every pixel refracts an orthogonal view ray with Snell's law
   (`refract()`, air → IOR ≈ 1.5) at the bevel's 3D normal and follows it
   through the glass to the backdrop plane. The convex bevel bends rays
   inward, so the rim magnifies and curves what is behind it; the flat centre
   passes the backdrop through undistorted. Red/green/blue use slightly
   different IORs (chromatic dispersion, exaggerated ×6 so it is visible).
   The bevel is limited to 60 % of the shape's half size so small shapes keep
   a clear centre.
3. **Tint** – a material color mixed in with an *adaptive* opacity: the shader
   measures backdrop luminance and pushes it towards the material's target
   luminance so text stays legible on any wallpaper. Clear materials use very
   little tint.
4. **Fresnel + specular** – Schlick Fresnel (F₀ = 0.04) reflects a soft bright
   environment on the steep bevel, strongest on the side facing the light;
   a crisp directional rim highlight (top-left by default) and a thinner
   counter-highlight on the opposite edge.
5. **Shadow** – an outer, very soft ambient shadow + a tighter contact shadow
   (drawn as a separate SDF pass outside the surface).

Shell surfaces whose glass sits over other clients (the Dock over a
maximised window, Control Center over a browser) are refracted by the
compositor: the KWin effect `lumen_glass` (`compositor/effects/glass`)
copies the already composited pixels behind each glass shape and draws them
back through the same bevel/Snell model (frost via the texture's mip chain).
The shell publishes its shapes with `CompositorGlass` (D-Bus
`org.kde.KWin /org/lumen/Glass org.lumen.Glass.SetShapes`) and the surface
then draws only tint, Fresnel and specular light (`compositorBlur` mode 2).
Without the effect (no OpenGL compositing) the shell falls back to
refracting its own wallpaper copy, and to KWin's blur when a window is
underneath.

Values from `rendering/src/materials.h` (refr/lens in px):

| Material      | Frost | Tint α | Refr (T) | Lens | Dispersion | Specular | Use                           |
|---------------|-------|--------|----------|------|------------|----------|-------------------------------|
| `ultraThin`   | 0.00  | 0.02   | 22       | 20   | 0.60       | 1.00     | Dock, floating controls (clear) |
| `thin`        | 0.25  | 0.08   | 16       | 16   | 0.45       | 0.90     | Menus, popovers               |
| `regular`     | 0.40  | 0.12   | 16       | 16   | 0.35       | 0.80     | Control Center, widgets, notifications |
| `frosted`     | 1.00  | 0.40   | 10       | 12   | 0.20       | 0.60     | Sidebars, Spotlight results   |
| `dark`        | 0.60  | 0.42   | 14       | 14   | 0.30       | 0.65     | HUDs, OSD, login              |
| `light`       | 0.60  | 0.38   | 14       | 14   | 0.30       | 0.80     | Light-only floating panels    |
| `floating`    | 0.05  | 0.03   | 24       | 24   | 0.65       | 1.00     | Spotlight field, toolbar pills |
| `active`      | 0.30  | 0.30   | 14       | 14   | 0.40       | 0.95     | Selected/pressed glass        |

`GlassSurface.frostBias` shifts a single surface toward frosted (e.g. behind
dense text) without changing its material.

Accessibility: *Reduce transparency* forces frost = 1 and an opaque tint;
*Increase contrast* adds a 1 px `label·30%` border and raises tint α by 0.25.

## 5. Motion

| Token                 | Spring (stiffness, damping, mass) | Approx. duration | Use                        |
|-----------------------|-----------------------------------|------------------|----------------------------|
| `snappy`              | 520, 42, 1                        | 0.25 s           | Hover, toggles, presses    |
| `smooth`              | 300, 34, 1                        | 0.38 s           | Panels, menus              |
| `bouncy`              | 380, 22, 1                        | 0.55 s           | Dock launch bounce, OSD    |
| `gentle`              | 170, 26, 1                        | 0.6 s            | Window open, overview      |

| Easing token    | Bezier                     | Use                     |
|-----------------|----------------------------|-------------------------|
| `standard`      | (0.25, 0.1, 0.25, 1.0)     | Fades                   |
| `emphasized`    | (0.2, 0.0, 0.0, 1.0)       | Panel entrance          |
| `exit`          | (0.4, 0.0, 1.0, 1.0)       | Dismissals              |

Durations: `instant` 90 ms · `fast` 160 ms · `normal` 240 ms · `slow` 360 ms ·
`window` 280 ms (open), 200 ms (close) · `minimize` 420 ms.

Rules: entrances use springs (interruptible, velocity preserving); exits are
~30 % faster than entrances; nothing animates longer than 600 ms except the
boot/login sequence; with *Reduce motion* all spatial transitions become
120 ms cross-fades.

## 6. Implementation notes

* Enums declared in QML controls whose root is a C++ template type (e.g.
  `Button.Kind`) resolve in plain bindings (`kind: Button.Prominent`) but
  evaluate to `undefined` inside JavaScript expressions on Qt 6.11. Use plain
  bindings, or numeric values with a comment, in ternaries.
* Popups (`Menu`, `ToolTip`) are not Items: resolve the window with
  `control.parent.Window.window`, never `Window.window`.
