#!/bin/bash
# Boots the LUMEN OS ISO in QEMU with UEFI firmware (OVMF) and saves a screen
# capture every INTERVAL seconds to out/boot/ (GRUB → Plymouth → session).
# Uses KVM when /dev/kvm exists, otherwise multi-threaded TCG (slow).
#   scripts/qemu-boot-test.sh [iso] [minutes] [interval]
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ISO="${1:-$(ls "$ROOT"/out/iso/*.iso | head -1)}"
MINUTES="${2:-20}"; INTERVAL="${3:-45}"
RUNTIME="${CONTAINER_RUNTIME:-$(command -v podman || command -v docker)}"
mkdir -p "$ROOT/out/boot"
KVM=""; [ -e /dev/kvm ] && KVM="--device /dev/kvm"
"$RUNTIME" run --rm $KVM -v "$ROOT":/src -v "$ROOT/scripts/devcontainer/extra-ca":/extra-ca:ro \
    -e ISO="/src/${ISO#$ROOT/}" -e MINUTES="$MINUTES" -e INTERVAL="$INTERVAL" \
    registry.fedoraproject.org/fedora:45 bash -euc '
    cp /extra-ca/*.crt /etc/pki/ca-trust/source/anchors/ 2>/dev/null || true; update-ca-trust
    dnf -y -q install qemu-system-x86-core edk2-ovmf python3 >/dev/null
    cp /usr/share/edk2/ovmf/OVMF_VARS.fd /tmp/vars.fd
    ACCEL="tcg,thread=multi"; CPU="max"; [ -e /dev/kvm ] && { ACCEL=kvm; CPU=host; }
    qemu-system-x86_64 -machine q35 -accel $ACCEL -cpu $CPU -smp 4 -m 6144 \
        -drive if=pflash,format=raw,readonly=on,file=/usr/share/edk2/ovmf/OVMF_CODE.fd \
        -drive if=pflash,format=raw,file=/tmp/vars.fd \
        -cdrom "$ISO" -boot d -vga std -display none \
        -qmp unix:/tmp/qmp.sock,server,nowait -serial file:/src/out/boot/serial.log &
    QEMU=$!
    sleep 3
    python3 - <<PY
import json, os, socket, time
def qmp(cmd, **args):
    s = socket.socket(socket.AF_UNIX); s.connect("/tmp/qmp.sock"); f = s.makefile("rw")
    f.readline(); f.write(json.dumps({"execute": "qmp_capabilities"}) + "\n"); f.flush(); f.readline()
    f.write(json.dumps({"execute": cmd, "arguments": args}) + "\n"); f.flush(); r = f.readline(); s.close(); return r
for _ in range(60):
    if os.path.exists("/tmp/qmp.sock"): break
    time.sleep(1)
end = time.time() + int(os.environ["MINUTES"]) * 60; n = 0
while time.time() < end:
    qmp("screendump", filename=f"/src/out/boot/frame-{n:03d}.ppm"); print("frame", n, flush=True)
    n += 1; time.sleep(int(os.environ["INTERVAL"]))
PY
    kill $QEMU || true'
ls "$ROOT/out/boot"
