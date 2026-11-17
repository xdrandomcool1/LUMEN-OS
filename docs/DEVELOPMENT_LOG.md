# LUMEN OS — Development Log

Chronological engineering log. Every claim here is backed by a build, test
run or screenshot produced in the dev container (`scripts/devcontainer`).

## Environment findings (2026-11-16)

* Host: cloud container, 4 vCPU / 15 GiB, no GPU, no `/dev/dri`, no KVM.
  Docker works (privileged mode, loop devices available).
* Target base: **Fedora 45** — Qt 6.11.2, KWin 6.8.3, LayerShellQt 6.8.3,
  SDDM 0.21, Plymouth 26, Rust 1.99.
* All compilation happens in `lumen-dev:45` (Fedora 45) so binaries link
  against exactly what ships on the ISO.

### Problems found and how they were solved
| Problem | Root cause | Resolution |
|---|---|---|
| `dnf` TLS failures in container build | TLS-intercepting proxy | `scripts/devcontainer/extra-ca/` trust anchors |
| KWin "Operation not permitted" | `kwin_wayland` has file capability CAP_SYS_NICE | run container with `--cap-add SYS_NICE` |
| QML errors invisible | Fedora Qt logs to journald when stderr is not a TTY | `QT_FORCE_STDERR_LOGGING=1` in dev env |
| KWin segfault at start (`QStyleHintsPrivate::update`) | Qt 6.11's built-in `QKdeTheme` emits a theme change before style hints exist when `KDE_FULL_SESSION` is set | ship `plasma-integration` and set `QT_QPA_PLATFORMTHEME=kde` (also needed for global menus) |
| KWin segfault on screenshot (`RenderDevice::eglContext`) | no render node → KWin's virtual backend composites with QPainter; the screenshot plugin assumes EGL | visual QA uses sway (wlroots headless + pixman) + `grim`; KWin used for integration runs |
| Shell segfault on start | `Qt.application.screens[i]` is a `QQuickScreenInfo`, not a `QScreen` | `LayerSurface.screen` accepts both and resolves by name |

## Milestone 1 — progress

* ☑ `Lumen.Motion` spring engine, `Lumen.Design` tokens/appearance/symbols,
  `Lumen.Glass` renderer (dual-Kawase pyramid + composite shader), verified by
  `tests/visual/glass-materials.qml` renders in dark and light appearance.
* ☑ `Lumen.Controls` — 25 controls + `AppWindow` chrome, verified by
  `tests/visual/controls-gallery.qml`.
* ☑ Original wallpaper collection (procedural, 11 wallpapers + dynamic set).
* ☑ Original icon theme (squircle app icons, folders, Trash).
* ☑ `lumen-shell` on a live Wayland compositor: wallpaper/desktop widgets
  (background layer), transparent adaptive menu bar (top layer, exclusive
  zone), Liquid Glass Dock (magnification, indicators, context menus,
  auto-hide, Trash), overlay panel host (Control Center, Spotlight,
  Notification Center, menus). Screenshots: `docs/screenshots/m1-*.png`.
* ☑ `lumen-screenshot` (KWin ScreenShot2 client, CLI + GUI).

### Verification limits (honest notes)
* Running-app indicators, window lists and KWin blur cannot be exercised in
  the sway QA session (they need KWin's privileged protocols, and KWin can't
  use OpenGL without a GPU here). They are covered by code paths tested in
  the KWin integration run (shell starts, registers D-Bus, loads all panels).

## Milestones 2–5 — progress (2026-11-16/17)

* ☑ Control Center, Spotlight (apps, files via lumend, settings, actions,
  calculator), Notification Center + banners, widgets — captured on a live
  compositor (`docs/screenshots/m1-*`, `m2-spotlight.png`).
* ☑ Applications: Finder, Settings, Terminal, Activity Monitor, App Store,
  Calculator, Calendar, Text Editor, Image Viewer, Media Player, Screenshot
  (`docs/screenshots/m3-*`). Smoke-tested together in one session, no QML
  errors except the optional syntax-highlighting module where absent.
* ☑ KWin: `org.lumen.decoration` verified loaded by KWin 6.8.3 via
  `supportInformation`; `/etc/xdg` defaults verified applied; gestures plugin
  and scripted effect install. Effects cannot run without OpenGL compositing
  in this sandbox ("Compositing is not active").
* ☑ lumend: unit tests + D-Bus activation in a live session.
* ☑ Test suite (`ctest`): lumend-unit, core-unit, js-engines, apps-unit — 100 %
  passing, also inside `rpmbuild %check`.
* ☑ SDDM greeter theme rendered by `sddm-greeter-qt6 --test-mode`
  (`docs/screenshots/m4-login.png`); Plymouth script theme + assets.
* ☑ RPMs: lumen-desktop, lumen-apps, lumen-installer (`scripts/build-rpms.sh`).
* ☑ ISO: `iso/build-iso.sh` (kiwi-ng, Fedora 45 + updates + local repo):
  `LUMEN-OS.x86_64-1.0.0.iso`, 1.3 GB, hybrid BIOS/UEFI El Torito, shim +
  GRUB, dmsquash live root. SHA256 in `out/iso/SHA256SUMS`.

### More problems solved
| Problem | Resolution |
|---|---|
| kiwi passed metalink URLs as baseurl | `sourcetype="metalink"` |
| dnf inside the image root rejected the build proxy's TLS certificate (metadata resolves CA paths on the host, packages inside the root) | kiwi `customize` hook + identical CA path on host and in root, deleted by `config.sh` before sealing |
| Fedora 45 moved GPG keys / dropped `PLATFORM_ID` | `/usr/share/pki/rpm-gpg/RPM-GPG-KEY-fedora-45-primary`; default value |
| QML enums of C++-rooted controls are `undefined` inside JS expressions | documented in DESIGN_SYSTEM.md §6; avoided in code |
| `.flat()`/inline component scoping/`Window.window` on popups | fixed per component |

## Milestone 6 — ISO boot testing (QEMU, OVMF UEFI, software emulation)

`scripts/qemu-boot-test.sh` boots the ISO with OVMF and captures frames;
the ISO's own kernel/initrd were also booted with `console=ttyS0` and
`systemd.journald.forward_to_console=1` to read the full system journal.

| Run | Finding | Fix |
|---|---|---|
| 1 | UEFI → GRUB → kernel OK; GRUB warned `terminal gfxterm isn't found` | GRUB text console |
| 1 | Greeter and session died: `pam_systemd.so` missing → no logind session, no `XDG_RUNTIME_DIR`, KWin "could not add wayland socket" | `systemd-pam` + `patternType="plusRecommended"` (weak deps as in Fedora spins) |
| 2 | Live user auto-logged in; `startlumen` → KWin on DRM, lumend indexing and serving `org.lumen.Search`, LUMEN gestures plugin loaded | — |
| 2 | `lumen-shell` skipped by its ExecCondition: startplasma-wayland resets `XDG_CURRENT_DESKTOP=KDE` | `LUMEN_SESSION` marker exported to the systemd user manager before startplasma |
| 2 | Initrd contains the LUMEN Plymouth theme, `script.so`, DRM renderer, `Theme=lumen` | — |
| 3 | Full desktop reached (menu bar, widgets, dynamic wallpaper, Dock) but: stock Breeze decoration, blank pinned Dock icons, KDE Welcome Center autostarted | see run 4 |
| 4 | Root cause of the first two: `kdeglobals` named `org.lumen.desktop`, which did not exist, so Plasma wrote the Fedora global theme defaults into `~/.config/kdedefaults` (outranks `/etc/xdg`): decoration → Breeze, icon theme → breeze (no LUMEN app icons) | real look-and-feel package `org.lumen.desktop` with matching defaults; app icons also in hicolor; `plasma-welcome` removed and its kded module disabled; splash disabled; Firefox pinned by its real id `org.mozilla.firefox.desktop` |
| 4 | Verified by booting the rebuilt ISO and opening Finder via Spotlight (QMP key events): LUMEN traffic-light decoration, all pinned icons rendered, no Welcome Center (`docs/screenshots/iso-boot-finder.png`) | — |
| 4 | Open: a Finder window started from Spotlight shows as a second Dock tile instead of lighting up the pinned one | to investigate |
