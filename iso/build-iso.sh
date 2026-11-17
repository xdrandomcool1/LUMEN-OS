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
mkdir -p "$ROOT/out/iso"; rm -rf "$ROOT/out/kiwi"; mkdir -p "$ROOT/out/kiwi"
# The kiwi workspace peaks at ~15 GB; never leave it behind, even on failure.
trap 'rm -rf "$ROOT/out/kiwi"' EXIT
"$RUNTIME" run --rm --privileged \
    -v "$ROOT":/src -v "$ROOT/out/repo":/repo:ro -v "$ROOT/out/kiwi":/work -v "$ROOT/scripts/devcontainer/extra-ca":/extra-ca:ro \
    -w /src registry.fedoraproject.org/fedora:45 bash -euxc '
        cp /extra-ca/*.crt /etc/pki/ca-trust/source/anchors/ 2>/dev/null || true; update-ca-trust
        # kiwi < 11.1.16 (Fedora 45 GA) crashes in "system create" (KeyError: help);
        # require the fixed update and retry with fresh metadata (stale mirrors).
        KIWI="python3-kiwi >= 11.1.16"
        for attempt in 1 2 3; do
            dnf -y -q --refresh install "$KIWI" kiwi-cli kiwi-systemdeps-iso-media kiwi-systemdeps-bootloaders kiwi-systemdeps-filesystems && break
            [ $attempt = 3 ] && exit 1; sleep 30
        done
        rpm -q python3-kiwi
        rm -rf /work/root /work/out
        mkdir -p /work/root/var/tmp
        # Build-environment trust anchors (TLS-intercepting proxies only);
        # removed again by config.sh before the image is sealed.
        if ls /extra-ca/*.crt >/dev/null 2>&1; then cp /etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem /work/root/var/tmp/lumen-build-ca.crt; cp /etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem /var/tmp/lumen-build-ca.crt; fi
        kiwi-ng --type iso system prepare --description /src/iso/lumen --root /work/root --allow-existing-root
        kiwi-ng system create --root /work/root --target-dir /work/out
        mv /work/out/*.iso /src/out/iso/
        rm -rf /work/root /work/out
        cd /src/out/iso && sha256sum *.iso > SHA256SUMS'
ls -la "$ROOT/out/iso"
