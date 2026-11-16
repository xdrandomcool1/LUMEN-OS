# LUMEN OS

LUMEN OS is a Fedora-based Linux distribution with its own desktop: a
Liquid Glass interface, fluid spring-based motion, a menu bar, a floating
Dock, Control Center, Spotlight, Notification Center, Mission Control and a
suite of native applications. It's inspired by the modern macOS Liquid Glass
design and uses only original artwork, open fonts (Inter, JetBrains Mono)
and its own code.

![LUMEN OS desktop](docs/screenshots/m3-apps.png)

## What's inside

| Area | Components |
|---|---|
| Design system | `Lumen.Design` (tokens, appearance, typography, symbols), `Lumen.Controls` (25 controls + window chrome) |
| Rendering | `Lumen.Glass`: dual-Kawase blur pyramid + glass composite shader (refraction, dispersion, adaptive tint, specular rim, shadows), 8 materials |
| Motion | `Lumen.Motion`: vsync-driven spring engine, motion tokens, reduced motion |
| Shell | `lumen-shell`: wallpaper & widgets, menu bar (global menus), Dock, Control Center, Spotlight, notifications |
| Compositor | KWin 6 + LUMEN decoration, gestures plugin, window-motion effect, app switcher, defaults |
| Services | `lumend` (Rust search index), NetworkManager, BlueZ, PipeWire, UPower, PowerDevil, AccountsService, firewalld, Flatpak |
| Apps | Finder, System Settings, Terminal, App Store, Activity Monitor, Calculator, Calendar, Text Editor, Image Viewer, Media Player, Screenshot |
| Boot & install | Plymouth theme, SDDM theme, Calamares installer, kiwi live ISO |

## Building

Everything builds inside a Fedora 45 container that matches the target:

```sh
docker build -t lumen-dev:45 scripts/devcontainer   # once
scripts/build.sh                                   # configure, build, run tests
scripts/run-session.sh --compositor sway --shot out.png   # headless session + screenshot
scripts/build-rpms.sh                              # RPMs → out/repo
iso/build-iso.sh                                   # live/installer ISO → out/iso
```

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md), [docs/DESIGN_SYSTEM.md](docs/DESIGN_SYSTEM.md),
[docs/IMPLEMENTATION_PLAN.md](docs/IMPLEMENTATION_PLAN.md) and the evidence-based
[docs/DEVELOPMENT_LOG.md](docs/DEVELOPMENT_LOG.md).

## License

GPL-3.0-or-later for code; CC-BY-SA-4.0 for generated artwork; UI symbols
from Phosphor Icons (MIT). See [LICENSE](LICENSE).
