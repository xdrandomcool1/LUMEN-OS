# LUMEN OS 1.0 — Release Notes

## Highlights
* **Liquid Glass everywhere.** A GPU glass renderer (blur pyramid, edge
  refraction, chromatic dispersion, adaptive tint, specular rims) shared by
  the shell, apps, login screen and installer.
* **Fluid motion.** A spring engine driven by the display's frame clock, so
  springs advance every frame at 60, 120 or 240 Hz, with reduced-motion support.
* **The desktop.** Transparent adaptive menu bar with global app menus,
  magnifying Dock with launch bounce and minimize-to-icon, Control Center,
  Spotlight (apps, files via `lumend`, settings, actions, calculator),
  notifications and widgets.
* **Apps.** Finder, System Settings, Terminal, App Store (Flatpak/Flathub),
  Activity Monitor, Calculator, Calendar, Text Editor, Image Viewer, Media
  Player and Screenshot.
* **Installer.** A live ISO with the Calamares installer: Btrfs/ext4/XFS,
  LUKS2 encryption, user creation, timezone, keyboard and bootloader.

## Under the hood
Fedora 45 base, KWin 6.8 compositor with LUMEN decoration, gestures and
effects, Qt 6.11, Plasma session services (without plasmashell), PipeWire,
NetworkManager, BlueZ, UPower, PowerDevil, firewalld and Flatpak.

## Known limitations
* Visual verification ran on a software-rendered headless compositor. KWin
  blur and effect plugins need a GPU and are verified to load, not yet seen
  running on hardware (see `docs/HARDWARE_TEST_PLAN.md`).
* The Dock's position setting supports the bottom edge only.
* Mission Control uses KWin's Overview effect, with LUMEN gestures and
  shortcuts.
* Proprietary NVIDIA drivers and non-free codecs aren't included (licensing);
  the driver installs from RPM Fusion in one click (Control Center ›
  Graphics › Install NVIDIA Driver).
* On machines without GPU acceleration (VMs without 3D, unknown drivers)
  LUMEN automatically drops to a software tier: plain translucent panels
  replace Liquid Glass so the session stays responsive.
