#!/bin/bash
# End-to-end check of compositor-side Liquid Glass (KWin effect lumen_glass)
# on the real ISO: boots it in QEMU (UEFI), waits for the live session, opens
# Terminal and maximises it so it sits behind the Dock, opens Control Center
# over it, and saves screenshots plus the system journal (serial console).
#   scripts/qemu-glass-test.sh [iso] [boot-minutes]
# Output: out/glass-test/{desktop,terminal,maximized,controlcenter}.png,
#         journal.log (system journal) and diag.txt (checks run in the guest
#         through a root debug shell on the second serial port)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ISO="${1:-$(ls "$ROOT"/out/iso/*.iso | head -1)}"
BOOT_MIN="${2:-12}"
RUNTIME="${CONTAINER_RUNTIME:-$(command -v podman || command -v docker)}"
OUT="$ROOT/out/glass-test"
rm -rf "$OUT"; mkdir -p "$OUT"
KVM=""; [ -e /dev/kvm ] && KVM="--device /dev/kvm"
"$RUNTIME" run --rm $KVM -v "$ROOT":/src -v "$ROOT/scripts/devcontainer/extra-ca":/extra-ca:ro \
    -e ISO="/src/${ISO#$ROOT/}" -e BOOT_MIN="$BOOT_MIN" \
    registry.fedoraproject.org/fedora:45 bash -euc '
    cp /extra-ca/*.crt /etc/pki/ca-trust/source/anchors/ 2>/dev/null || true; update-ca-trust
    dnf -y -q install qemu-system-x86-core qemu-device-display-virtio-vga qemu-device-display-virtio-gpu edk2-ovmf python3 xorriso ImageMagick >/dev/null
    # Boot the ISO kernel directly so the journal goes to the serial console.
    xorriso -osirrox on -indev "$ISO" -extract /boot/x86_64/loader/linux /tmp/linux \
            -extract /boot/x86_64/loader/initrd /tmp/initrd >/dev/null 2>&1
    LABEL=$(xorriso -indev "$ISO" -pvd_info 2>/dev/null | sed -n "s/^Volume Id *: *//p")
    # Boot with the same live options as the GRUB menu of the ISO (the installer
    # depends on them, e.g. the OverlayFS root base at /run/rootfsbase).
    CFG=$(xorriso -indev "$ISO" -find / -name grub.cfg 2>/dev/null | head -1 | sed "s/^.//;s/.$//" || true)
    xorriso -osirrox on -indev "$ISO" -extract "$CFG" /tmp/grub.cfg >/dev/null 2>&1 || true
    LIVEARGS=$(grep -m1 -E "^[[:space:]]*linux" /tmp/grub.cfg 2>/dev/null | tr " " "\n" | grep -E "^(rd[.]|root=)" | tr "\n" " " || true)
    [ -n "$LIVEARGS" ] || LIVEARGS="root=live:CDLABEL=$LABEL rd.live.image"
    echo "grub.cfg: $CFG  live args: $LIVEARGS"
    cp /usr/share/edk2/ovmf/OVMF_VARS.fd /tmp/vars.fd
    ACCEL="tcg,thread=multi"; CPU="max"; [ -e /dev/kvm ] && { ACCEL=kvm; CPU=host; }
    qemu-system-x86_64 -machine q35 -accel $ACCEL -cpu $CPU -smp 4 -m 6144 \
        -drive if=pflash,format=raw,readonly=on,file=/usr/share/edk2/ovmf/OVMF_CODE.fd \
        -drive if=pflash,format=raw,file=/tmp/vars.fd \
        -cdrom "$ISO" -kernel /tmp/linux -initrd /tmp/initrd \
        -append "$LIVEARGS console=ttyS0 systemd.journald.forward_to_console=1 systemd.log_color=0 systemd.debug_shell=/dev/ttyS1" \
        -device virtio-vga,xres=1440,yres=900 -display none \
        -qmp unix:/tmp/qmp.sock,server,nowait -serial file:/src/out/glass-test/journal.log \
        -chardev socket,id=dbg,path=/tmp/dbg.sock,server=on,wait=off -serial chardev:dbg &
    QEMU=$!
    BOOT_MIN=$BOOT_MIN python3 /src/scripts/qemu-glass-test.py
    kill $QEMU || true'
ls -la "$OUT"
grep -iE "lumen_glass|lumen.kwin.glass|org.lumen.Glass|OpenGL|compositing" "$OUT/journal.log" | tail -20 || true
