# LUMEN OS — Architecture

LUMEN OS is a Fedora-based Linux distribution with its own desktop shell,
compositor integration, design system, animation engine and application suite.
This document describes how the pieces fit together and *why* they are built
the way they are.

## 1. Layered overview

```
┌────────────────────────────────────────────────────────────────────────────┐
│ Applications (Qt 6 / QML)                                                  │
│  Finder · Settings · Terminal · Store · Activity Monitor · Calculator ·    │
│  Calendar · Text Editor · Image Viewer · Media Player · Screenshot         │
├────────────────────────────────────────────────────────────────────────────┤
│ Lumen Shell (lumen-shell, Qt 6 / QML / C++)                                │
│  Menu bar · Dock · Control Center · Spotlight · Notification Center ·     │
│  Widgets · App switcher · OSD · Desktop/wallpaper engine                  │
├──────────────────────┬─────────────────────────────────────────────────────┤
│ Shared QML modules   │ Lumen.Design  — tokens, typography, controls        │
│ (libLumenUi)         │ Lumen.Motion  — springs, curves, durations          │
│                      │ Lumen.Glass   — Liquid Glass materials + shaders    │
├──────────────────────┴─────────────────────────────────────────────────────┤
│ Compositor: KWin 6 (Wayland)                                               │
│  + lumen-glass effect (C++ KWin effect: blur + refraction behind surfaces) │
│  + Lumen scripted effects (open/close/minimize-to-Dock, workspace slide)   │
│  + Lumen Mission Control (declarative KWin scene effect)                   │
│  + Lumen window decoration (Aurorae theme: glass title bar, traffic lights)│
├────────────────────────────────────────────────────────────────────────────┤
│ System services                                                            │
│  lumend (Rust, session bus): search index, settings store, wallpaper       │
│         scheduler, system statistics                                       │
│  Upstream: systemd · logind · NetworkManager · BlueZ · PipeWire/WirePlumber│
│            UPower · Polkit · firewalld · Flatpak · AccountsService        │
├────────────────────────────────────────────────────────────────────────────┤
│ Boot & install: Plymouth theme · SDDM theme · Calamares (Lumen branding)   │
│ Base: Fedora 45 (kernel, Mesa, NVIDIA via RPM Fusion akmods optional)      │
└────────────────────────────────────────────────────────────────────────────┘
```

## 2. Key decisions

### 2.1 Fedora + KWin rather than a new compositor
KWin 6 already provides production-quality Wayland support: multi-monitor,
HiDPI, VRR, 120–240 Hz, explicit sync for NVIDIA, direct scan-out, input
methods, tablet/touchpad gestures, screencasting and session restore. Writing
a new compositor would cost years and deliver worse hardware support. LUMEN
therefore extends KWin through its *supported* extension points:

| Need                         | KWin extension point                                  |
|------------------------------|-------------------------------------------------------|
| Glass behind shell surfaces  | C++ effect plugin (`compositor/effects/lumen-glass`)  |
| Window open/close/minimize   | Scripted effects (JavaScript, `animate()` API)       |
| Mission Control              | Declarative scene effect (QML, `KWin.SceneEffect`)    |
| Title bar / traffic lights   | Aurorae decoration theme (SVG)                        |
| Shortcuts (Super+Space …)    | KGlobalAccel actions exported by the shell            |
| Panels / Dock / menu bar     | `wlr-layer-shell` via LayerShellQt                    |

### 2.2 The shell is a separate process from the compositor
`lumen-shell` is an ordinary Wayland client using layer-shell surfaces. A crash
in the shell never takes down the session or user windows; systemd restarts it
(`lumen-shell.service`, `Restart=on-failure`). The shell never runs privileged
code: everything privileged goes through D-Bus services guarded by Polkit
(NetworkManager, BlueZ, hostnamed, timedated, AccountsService, UDisks2).

### 2.3 Two rendering paths for Liquid Glass
Liquid Glass needs the pixels *behind* a surface. Two cases exist:

1. **In-scene backdrop** – the backdrop is drawn by the same process (e.g. the
   wallpaper behind the Dock in the shell, an app's content behind its
   sidebar/toolbar). `Lumen.Glass` samples the backdrop texture directly through
   a `ShaderEffectSource`, runs a separable dual-Kawase blur on the GPU and then
   the *glass composite shader* (refraction, dispersion, specular, rim light,
   adaptive tint). This is pixel-exact and works in every environment.
2. **Cross-surface backdrop** – a window from another client sits under the
   Dock. Only the compositor has those pixels. Shell surfaces publish their glass
   region and material via the standard `org_kde_kwin_blur` protocol; the
   `lumen-glass` KWin effect renders blur + refraction there. When the custom
   effect is not loaded, KWin's stock blur effect serves as a graceful fallback.

Both paths share the same material parameters (`design-system/tokens/glass.json`).

### 2.4 One design system, one animation engine
All UI code — shell, apps, SDDM greeter and installer branding — imports the
same QML modules, so a token change (corner radius, spring stiffness, blur
radius) changes the whole OS consistently. Durations/springs are also exported
as a generated JavaScript file consumed by the KWin scripted effects so that
compositor animations and shell animations share the exact same curves.

### 2.5 Rust for long-running services
`lumend` is a small async Rust daemon (zbus + tokio) because it runs for the
entire session and handles untrusted input (file names, file contents for the
search index). Memory safety and a small, predictable footprint matter more
there than in UI code.

## 3. Process model at runtime

```
systemd --user
 ├─ plasma-kwin_wayland.service   (KWin compositor, effects loaded in-process)
 ├─ lumen-shell.service           (menu bar, Dock, panels, notifications server)
 ├─ lumend.service                (org.lumen.Search / Settings / SystemMonitor)
 ├─ pipewire / wireplumber
 └─ xdg-desktop-portal(-kde)      (screenshots, file pickers, Flatpak portals)
```

Session entry point: `/usr/share/wayland-sessions/lumen.desktop` →
`startlumen` → `startplasma-wayland`-compatible bootstrap that loads LUMEN's
KWin configuration and the `lumen-session.target` user target.

## 4. D-Bus interfaces

| Service / path                     | Owner        | Purpose                                     |
|------------------------------------|--------------|---------------------------------------------|
| `org.lumen.Shell /Shell`           | lumen-shell  | Toggle Spotlight, Control Center, NC, etc.  |
| `org.freedesktop.Notifications`    | lumen-shell  | Notification server (spec 1.2)              |
| `org.lumen.Search /Search`         | lumend       | Indexed file/app search                     |
| `org.lumen.Settings /Settings`     | lumend       | Typed settings store with change signals    |
| `org.lumen.SystemMonitor /Monitor` | lumend       | CPU/RAM/net/disk samples for widgets        |
| `org.lumen.Wallpaper /Wallpaper`   | lumend       | Dynamic (time-based) wallpaper scheduling   |

## 5. Repository layout

| Path                    | Content                                                     |
|-------------------------|-------------------------------------------------------------|
| `design-system/`        | `Lumen.Design` QML module, tokens, controls                 |
| `animation-engine/`     | `Lumen.Motion` QML module (C++ spring integrator + tokens)   |
| `rendering/`            | `Lumen.Glass` QML module (materials, blur, composite)      |
| `shaders/`              | GLSL sources compiled to `.qsb` at build time              |
| `shell/`                | `lumen-shell`                                               |
| `compositor/`           | KWin effects, scripts, decoration, configuration           |
| `applications/`         | Native applications                                         |
| `system-services/`      | `lumend` (Rust)                                             |
| `themes/`               | Icons, SDDM, Plymouth, color schemes, cursors              |
| `wallpapers/`           | Procedural wallpaper generator + rendered wallpapers       |
| `installer/`            | Calamares configuration and branding                       |
| `iso/`                  | Kiwi image description + build scripts                     |
| `packaging/`            | RPM spec files                                              |
| `scripts/`              | Dev container, build, headless run & screenshot scripts    |
| `tests/`                | Unit, QML and visual regression tests                      |
