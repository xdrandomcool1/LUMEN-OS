#!/bin/bash
# Reproducible LUMEN OS ISO build.
#   1. scripts/build-rpms.sh   → out/repo (LUMEN RPMs + repodata)
#   2. iso/build-iso.sh        → out/iso/LUMEN-OS*.iso
# The image is assembled by kiwi-ng in a privileged Fedora 45 container,
# from Fedora 45 + updates repositories plus the local LUMEN repository.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
RUNTIME="${CONTAINER_RUNTIME:-$(command -v podman || command -v docker)}"
[ -f "$ROOT/out/repo/repodata/repomd.xml" ] || { echo "run scripts/build-rpms.sh first"; exit 1; }
mkdir -p "$ROOT/out/iso"
"$RUNTIME" run --rm --privileged \
    -v "$ROOT":/src -v "$ROOT/out/repo":/repo:ro -v "$ROOT/scripts/devcontainer/extra-ca":/extra-ca:ro \
    -w /src registry.fedoraproject.org/fedora:45 bash -euxc '
        cp /extra-ca/*.crt /etc/pki/ca-trust/source/anchors/ 2>/dev/null || true; update-ca-trust
        dnf -y -q install kiwi-cli kiwi-systemdeps-iso-media kiwi-systemdeps-bootloaders kiwi-systemdeps-filesystems
        rm -rf /tmp/kiwi-out
        kiwi-ng --type iso system build --description /src/iso/lumen --target-dir /tmp/kiwi-out
        cp /tmp/kiwi-out/*.iso /src/out/iso/
        cd /src/out/iso && sha256sum *.iso > SHA256SUMS'
ls -la "$ROOT/out/iso"
