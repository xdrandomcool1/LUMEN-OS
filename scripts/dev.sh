#!/bin/bash
# Run a command inside the LUMEN dev container with the repo mounted at /src.
#   scripts/dev.sh cmake --build build
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
IMAGE="${LUMEN_DEV_IMAGE:-lumen-dev:45}"
RUNTIME="${CONTAINER_RUNTIME:-$(command -v podman || command -v docker)}"
TTY=""; [ -t 0 ] && TTY="-it"
exec "$RUNTIME" run --rm $TTY --cap-add SYS_NICE --security-opt seccomp=unconfined \
    -e QT_FORCE_STDERR_LOGGING=1 -v "$ROOT":/src -w /src ${LUMEN_DEV_ARGS:-} "$IMAGE" "$@"
