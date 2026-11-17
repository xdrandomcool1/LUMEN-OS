# LUMEN OS — Hardware Test Plan

LUMEN OS was developed and tested in a cloud container with no GPU, no
`/dev/dri`, no KVM and no physical peripherals. Everything below needs real
hardware, so it hasn't been run yet. Each item lists the expected behaviour
and how to check it.

## Performance tiers (automatic)

Behaviour no longer depends on the test machine having a GPU:

| Renderer | Tier | What runs |
|---|---|---|
| NVIDIA (proprietary) / AMD / Intel, real GPU | Full | full Liquid Glass: `lumen_glass` + in-scene backdrops |
| virtio / virgl / venus / SVGA3D | Lite | same as Full, flagged as paravirtualised |
| llvmpipe / softpipe / swrast / SwiftShader | Software | `lumen_glass` does not load (KWin's own blur does the same); `GlassSurface` draws plain translucent panels; no `blurBehind` requests |

The tier is shown in Settings › About and Control Center › Graphics;
`LUMEN_PERF=full|lite|software` overrides the probe. On the software tier the
session stays responsive with reduced visuals — that is the expected look in
a VM without 3D acceleration, not a bug.

## NVIDIA (RTX / GTX, proprietary driver)

The ISO ships nouveau only. With NVIDIA hardware on the software tier,
Control Center › Graphics offers **Install NVIDIA Driver** (one click via
pkexec): it adds RPM Fusion free/nonfree, installs `kernel-devel` for the
running kernel plus `akmod-nvidia`, builds the module (`akmods`) and asks for
a reboot. Terminal equivalent: `pkexec /usr/libexec/lumen-nvidia-setup`.
Secure Boot must be disabled (or a MOK key enrolled) for the module to load;
the helper detects this and warns in the journal.

| # | Area | Test | Expected | Status |
|---|---|---|---|---|
| 1 | UEFI boot | Boot ISO with Secure Boot on (shim) | GRUB menu → Plymouth LUMEN splash → live session | ☐ |
| 2 | Intel graphics | Live session on Intel Xe/Arc | KWin OpenGL compositing; blur & window effects active (`qdbus org.kde.KWin /KWin supportInformation`) | ☐ |
| 3 | AMD graphics | RDNA2/3 | As above; Activity Monitor GPU tab reports `gpu_busy_percent` | ☐ |
| 4 | NVIDIA RTX | Control Center › Graphics → Install NVIDIA Driver (or `pkexec /usr/libexec/lumen-nvidia-setup`), reboot | Proprietary driver loads (`nvidia-smi`), KWin reports full tier, effects active | ☐ |
| 5 | NVIDIA (nouveau) | Out of the box | Session starts; software fallbacks acceptable | ☐ |
| 6 | HiDPI | 2× panel | Shell, Dock, glass and apps crisp; scale set in Settings › Displays | ☐ |
| 7 | High refresh | 120/144/240 Hz panel | Dock magnification and springs advance per frame (no 60 Hz cap) | ☐ |
| 8 | Multi-monitor | Two outputs, mixed scale | Menu bar and wallpaper on each screen; Dock on primary; Settings changes resolution/scale | ☐ |
| 9 | Wi-Fi | Join WPA2/WPA3 from Control Center | Password prompt, connects, menu bar icon shows strength | ☐ |
| 10 | Bluetooth | Pair headphones | Device listed in Control Center/Settings, audio output switches | ☐ |
| 11 | Audio | Volume keys / slider | PipeWire volume changes, output device switch works | ☐ |
| 12 | Brightness | Laptop panel | Control Center slider drives PowerDevil/backlight | ☐ |
| 13 | Suspend/resume | Lid close / Sleep | Locks on resume (kscreenlocker), Wi-Fi reconnects | ☐ |
| 14 | Touchpad | 3-finger swipe up / left / right, 4-finger pinch | Mission Control / desktop switch / overview | ☐ |
| 15 | Video playback | 4K H.264/AV1 in Media Player and Firefox | VA-API decode where codecs are available (Fedora ships only free codecs; RPM Fusion for others) | ☐ |
| 16 | Installer | Calamares: erase disk, LUKS2, Btrfs | Summary + confirm before writes; installed system boots to SDDM LUMEN theme | ☐ |
| 17 | Installer | Manual partitioning (ext4/XFS) | Correct fstab, bootloader on ESP | ☐ |
