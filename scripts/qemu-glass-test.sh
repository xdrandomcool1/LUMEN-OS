#!/bin/bash
# End-to-end check of compositor-side Liquid Glass (KWin effect lumen_glass)
# on the real ISO: boots it in QEMU (UEFI), waits for the live session, opens
# Terminal and maximises it so it sits behind the Dock, opens Control Center
# over it, and saves screenshots plus the system journal (serial console).
#   scripts/qemu-glass-test.sh [iso] [boot-minutes]
# Output: out/glass-test/{desktop,terminal,maximized,controlcenter}.png, journal.log
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
    dnf -y -q install qemu-system-x86-core edk2-ovmf python3 xorriso ImageMagick >/dev/null
    # Boot the ISO kernel directly so the journal goes to the serial console.
    xorriso -osirrox on -indev "$ISO" -extract /boot/x86_64/loader/linux /tmp/linux \
            -extract /boot/x86_64/loader/initrd /tmp/initrd >/dev/null 2>&1
    LABEL=$(xorriso -indev "$ISO" -pvd_info 2>/dev/null | sed -n "s/^Volume Id *: *//p")
    cp /usr/share/edk2/ovmf/OVMF_VARS.fd /tmp/vars.fd
    ACCEL="tcg,thread=multi"; CPU="max"; [ -e /dev/kvm ] && { ACCEL=kvm; CPU=host; }
    qemu-system-x86_64 -machine q35 -accel $ACCEL -cpu $CPU -smp 4 -m 6144 \
        -drive if=pflash,format=raw,readonly=on,file=/usr/share/edk2/ovmf/OVMF_CODE.fd \
        -drive if=pflash,format=raw,file=/tmp/vars.fd \
        -cdrom "$ISO" -kernel /tmp/linux -initrd /tmp/initrd \
        -append "root=live:CDLABEL=$LABEL rd.live.image console=ttyS0 systemd.journald.forward_to_console=1 systemd.log_color=0" \
        -device virtio-vga,xres=1440,yres=900 -display none \
        -qmp unix:/tmp/qmp.sock,server,nowait -serial file:/src/out/glass-test/journal.log &
    QEMU=$!
    python3 - <<PY
import json, os, socket, subprocess, time
for _ in range(60):
    if os.path.exists("/tmp/qmp.sock"): break
    time.sleep(1)
s = socket.socket(socket.AF_UNIX); s.connect("/tmp/qmp.sock"); f = s.makefile("rw")
f.readline()
def qmp(cmd, **args):
    f.write(json.dumps({"execute": cmd, "arguments": args}) + "\n"); f.flush()
    while True:
        r = json.loads(f.readline())
        if "return" in r or "error" in r: return r
qmp("qmp_capabilities")
def shot(name):
    qmp("screendump", filename="/tmp/s.ppm")
    subprocess.run(["magick", "/tmp/s.ppm", f"/src/out/glass-test/{name}.png"], check=False)
    print("captured", name, flush=True)
def keys(*names):
    qmp("send-key", keys=[{"type": "qcode", "data": k} for k in names]); time.sleep(0.4)
def type_text(text):
    for ch in text:
        keys({" ": "spc"}.get(ch, ch))
def journal():
    with open("/src/out/glass-test/journal.log", errors="replace") as j: return j.read()
# Wait for the shell (the Dock registers with KWin), then let it settle.
end = time.time() + int(os.environ["BOOT_MIN"]) * 60
while time.time() < end and "lumen-shell" not in journal():
    time.sleep(10)
time.sleep(150)
shot("desktop")
keys("meta_l", "spc"); time.sleep(20)
type_text("terminal"); time.sleep(15); keys("ret"); time.sleep(90)
shot("terminal")
keys("meta_l", "pgup"); time.sleep(45)
shot("maximized")
keys("meta_l", "c"); time.sleep(45)
shot("controlcenter")
PY
    kill $QEMU || true'
ls -la "$OUT"
grep -iE "lumen_glass|lumen.kwin.glass|org.lumen.Glass|OpenGL|compositing" "$OUT/journal.log" | tail -20 || true
