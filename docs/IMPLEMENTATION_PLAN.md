# LUMEN OS — Implementation Plan

Status legend: ☐ planned · ◐ in progress · ☑ done and verified (see
`DEVELOPMENT_LOG.md` for evidence: build logs, test runs, screenshots).

## Environment & tooling
- Build host: any Linux with Docker/Podman. All compilation happens inside the
  `lumen-dev` container (`scripts/devcontainer/Containerfile`), which is Fedora
  45 — the same base as the ISO — so binaries link against shipped libraries.
- `scripts/build.sh` — configure + build + unit tests (CMake/Ninja, cargo).
- `scripts/run-session.sh` — starts a real nested/headless session: KWin
  Wayland (virtual or nested backend) + lumen-shell + lumend.
- `scripts/screenshot.sh` — captures the headless session for visual QA.
- `iso/build-iso.sh` — reproducible ISO build in a privileged container.

## Milestone 1 — Desktop foundation
1. ☑ Repository skeleton, CMake super-project, dev container.
2. ☑ `Lumen.Design`: tokens (type, color, geometry), light/dark theme, controls
   (Button, Toggle, Slider, SegmentedControl, TextField, Sidebar, Toolbar,
   Menu, ContextMenu, Tooltip, ScrollBar, ListRow).
3. ☑ `Lumen.Motion`: C++ spring integrator on Qt's animation driver (vsync-
   aligned, interruptible, velocity preserving), curve/duration tokens,
   reduced-motion switch.
4. ☑ `Lumen.Glass`: shared dual-Kawase blur pyramid, glass composite shader
   (SDF continuous-corner shape, refraction, dispersion, adaptive tint,
   specular/rim, shadow), 8 materials, accessibility fallbacks.
5. ☑ `lumen-shell`: layer-shell surfaces for wallpaper, menu bar, Dock.
   Dock: magnification, running indicators (via KWin window list), pinned apps,
   launch bounce, context menu, drag reorder, Trash, auto-hide, position/size.
   Menu bar: Lumen menu, app name, app menus (DBusMenu / global menu),
   status items, clock.
6. ☑ Window decoration (Aurorae) with original traffic-light buttons;
   KWin config for rounded corners, shadows.
7. ☑ Headless session runner + first screenshots.

## Milestone 2 — Motion & system panels
1. ☑ KWin scripted effects: open/close scale-fade, minimize-to-Dock (target =
   Dock icon geometry published by the shell), workspace slide.
2. ☑ Control Center (Wi-Fi, Bluetooth, sound, brightness, appearance, Focus,
   output device, display, keyboard, battery).
3. ☑ Spotlight (apps, files via lumend, settings, calculator, actions).
4. ◐ Mission Control: KWin Overview effect bound to Ctrl/Super+Up and 3-finger swipe (LUMEN gestures plugin); app switcher layout (Super+Tab) ☑.
5. ☑ Gestures: `lumen_gestures` KWin plugin (3-finger up/down/left/right, 4-finger pinch) — builds and installs; needs touchpad hardware to exercise.
6. ☑ Notification server + Notification Center + widgets.

## Milestone 3 — Applications & services
1. ☑ `lumend` (Rust): settings, search index, system monitor, wallpaper
   scheduler.
2. ☑ Finder, Settings, Terminal (libvterm), Store (Flatpak), Activity Monitor.
3. ☑ Calculator, Calendar, Text Editor, Image Viewer, Media Player,
   Screenshot.

## Milestone 4 — Login & boot
1. ☑ Plymouth theme (animated logo).
2. ☑ SDDM greeter theme (avatars, password, session, power, glass).
3. ☑ Wallpaper collection + dynamic (time-of-day) wallpapers.

## Milestone 5 — Installer & ISO
1. ☑ RPM packaging for every component, local repository.
2. ☑ Calamares configuration + Lumen branding (disk selection, partitioning,
   filesystem, user, timezone, keyboard, progress, bootloader). All destructive
   steps require explicit confirmation (Calamares summary page).
3. ◐ Kiwi image description → live ISO (UEFI with shim), `iso/build-iso.sh`.
4. ◐ Boot test in QEMU (OVMF UEFI) — software emulation only (no KVM).

## Milestone 6 — Hardening & release
1. ◐ Performance budget checks (shell idle CPU, memory, frame times).
2. ☑ Hardware notes (docs/HARDWARE_TEST_PLAN.md): NVIDIA (RPM Fusion akmod + `nvidia-drm.modeset=1`),
   HiDPI, high refresh rate (KWin), multi-monitor.
3. ☑ Release notes, screenshots, demo video.

## Known external limitations
- Physical hardware tests (NVIDIA RTX, suspend/resume on laptops, Wi-Fi chips,
  real 120/144/240 Hz panels) cannot be executed in a cloud container; they are
  tracked as an explicit checklist in `docs/HARDWARE_TEST_PLAN.md`.
