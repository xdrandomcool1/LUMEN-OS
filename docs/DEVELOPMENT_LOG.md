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

## Liquid Glass refraction (2026-11-17)

The glass composite looked like frosted glass: every material sampled the
blurred pyramid with a heavy tint, and the rim only nudged samples outward.
`shaders/glass.frag` now models a slab with a convex-squircle bevel
(`h(x) = T·(1 − (1 − x)⁴)^¼`) and refracts an orthogonal view ray with
Snell's law (`refract()`, IOR 1.5, per-channel IOR for dispersion) to find
the backdrop sample, so the rim magnifies and bends the content behind it
while the centre stays clear. `BackdropBlur` captures the backdrop once at
full resolution (`sharp`), and each material's new `frost` mixes sharp →
blurred. Schlick Fresnel replaces the milky inner glow. Verified with
`tests/visual/liquid-glass.qml` (stripes/text backdrop) and live shell
captures (`run-session.sh --wallpaper lumen-dusk|peaks`).

| Problem | Resolution |
|---|---|
| `docker build scripts/devcontainer` fails ("open Dockerfile") | README passes `-f …/Containerfile` |
| ~120 Qt "non-existent target KF6::* / X11::X11" CMake warnings | shared KF6 packages found at top-level scope |
| White menu-bar text on light wallpapers in dark appearance | transparent bar picks text colour from wallpaper luminance only |
| Glass in `xvfb`/offscreen QA renders nothing | visual QA must run through `scripts/xrun.sh` (Mesa GL); `offscreen` uses the software scene graph without shaders |

## Compositor-side Liquid Glass and ISO delivery (2026-11-17)

* **KWin effect `lumen_glass`** (`compositor/effects/glass`): before a LUMEN
  shell surface is drawn, the already composited pixels behind each published
  glass shape are blitted into a texture (device resolution, mip chain when
  frosted) and drawn back through the convex-squircle bevel / Snell model. A
  `BackgroundEffectItem` makes any damage under the glass repaint the whole
  shape, as KWin's blur does. Shapes come from the shell over D-Bus
  (`org.kde.KWin /org/lumen/Glass`, `Lumen.Glass.CompositorGlass`); the shell
  then draws only tint and light. Without OpenGL compositing the previous
  in-process / blur fallback is used.
* **ISO in CI**: `.github/workflows/iso.yml` builds the dev container, RPMs
  and the kiwi ISO and publishes it as a GitHub (pre-)release (ISO + SHA256SUMS;
  images ≥ 2 GiB would be split).
* **`scripts/qemu-glass-test.sh`** boots the ISO (OVMF, GRUB's live options,
  journal on ttyS0, root debug shell on ttyS1), opens Terminal and Control
  Center with QMP key events, captures screenshots and runs checks in the guest.

| Problem (found by booting the ISO) | Resolution |
|---|---|
| KWin: `Cannot load library lumen_glass.so: undefined symbol LayerShellV1Window::staticMetaObject` | that class is not exported by libkwin; scopes are learnt from the exported `LayerShellV1Interface::surfaceCreated`; all plugin KWin symbols checked against libkwin's exports |
| Dock icons blank (Breeze instead of Lumen) | `kdeglobals` named the look-and-feel package `org.lumen.desktop`, which did not exist; package added, skeleton `kdeglobals` for new users |
| Plasma Welcome Center opened on first login | `/etc/xdg/plasma-welcomerc` marks it seen |
| Calamares `unpackfs` source `/run/rootfsbase` missing | the ISO boots dmsquash-live in device-mapper mode → `/dev/mapper/live-base`; both sources listed as optional |
| Shell stayed on the blur fallback with the effect loaded | 2 s `Available()` probe timed out while KWin started (llvmpipe); 15 s timeout + retry with backoff |
| CI ISO step: `kiwi-ng system create` → `KeyError: 'help'` | stale mirror served kiwi 11.1.1; `build-iso.sh` requires ≥ 11.1.16 |
| CI ISO step: `mkdir out/iso: Permission denied` | containerised RPM build leaves `out/` root-owned; ISO step runs with sudo |
| Local ISO builds ran out of disk at the squashfs step | ~16 GB peak plus the 4.7 GB dev image; prune build caches first |
| Frosted compositor glass (Control Center) drew uniform dark grey | on GLES KWin allocates BGRA textures for which `glGenerateMipmap` is unreliable; frost now uses a half-size blit pyramid sampled from a second texture unit |

Known issue: the pinned Firefox launcher shows a generic icon in the Dock
although `firefox.png` is present in hicolor (not yet investigated).

## Performance tiers and hardware acceleration (2026-11-17)

The whole desktop had only ever run on llvmpipe: KWin's `isOpenGLCompositing()`
is true there (real GL API, CPU-rasterised), so the `lumen_glass` effect loaded
and captured the backdrop at device resolution, and every shell window built a
Kawase pyramid — all on the CPU. That is what made the desktop lag in VMs, and
RTX-class hardware with only nouveau hits the same wall (see
HARDWARE_TEST_PLAN.md).

* **Renderer tiers** — `Lumen.Design` gained the `Gpu` singleton
  (`design-system/src/gpu.*`): one offscreen `QOpenGLContext` probe of
  `GL_RENDERER` per process classifies Full (real GPU) / Lite
  (virtio/virgl/venus/SVGA) / Software (llvmpipe, softpipe, swrast,
  SwiftShader). `LUMEN_PERF=full|lite|software|auto` overrides the probe.
* **KWin gate** — `lumen_glass::supported()` now returns false for software
  rasterisers (same check as KWin's own blur: `effects->openglContext()` →
  `isSoftwareRenderer()`), with a `LUMEN_FORCE_GLASS=1` escape hatch. The
  shell then runs its tint fallback.
* **Software-tier shell** — on the software tier `GlassSurface` skips the
  refraction `ShaderEffect` entirely and draws a plain translucent panel (the
  macOS reduce-transparency look), `BackdropBlur` gained an `active` switch,
  and Dock/PanelHost/Desktop stop requesting `blurBehind` and stop publishing
  compositor shapes. On real GPUs everything renders exactly as before.
* **NVIDIA one-click setup** — `lumen-nvidia-setup` (libexec, pkexec) adds
  RPM Fusion free/nonfree, installs `kernel-devel` for the running kernel and
  `akmod-nvidia`, builds the module (`akmods`) and warns when Secure Boot
  would block it. `SystemActions` (Lumen.System) drives it and reports
  progress; Control Center shows a Graphics module when NVIDIA hardware runs
  on the software tier, Spotlight gains the matching action, Settings › About
  appends the GL renderer string.
* **ISO** — `intel-media-driver`, `libva-utils`, `mokutil`, `spice-vdagent`,
  `qemu-guest-agent` added; CI gained an optional `boot_test` dispatch input
  that boots the fresh ISO in QEMU (KVM on the runner) and fails if the live
  session never starts; the guest diag now records the Firefox desktop id,
  the setup helper and VA-API state.
* **Firefox Dock icon (known issue resolved)** — the skel pinned
  `applications:firefox.desktop`, but the RPM has shipped
  `org.mozilla.firefox.desktop` since Fedora 40; the unknown launcher id is
  what rendered as a generic icon. The skel now pins the right id and
  `config.sh` resolves whatever id the image actually ships; the Lumen icon
  theme also falls back to adwaita.

| Problem | Resolution |
|---|---|
| llvmpipe ran the full glass pipeline | tier gate in the effect + plain-surface path in the shell |
| Glass could still sample an inactive pyramid | `BackdropBlur.active` drops the pyramid and the full-res capture |
| pkexec needs an in-session polkit agent | polkit-kde is already a `lumen-desktop` Require (Plasma session agent) |
| Unknown pinned launcher rendered generic | skel id fix + build-time id resolution (above) |
