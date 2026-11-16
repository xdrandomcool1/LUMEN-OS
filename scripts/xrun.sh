#!/bin/bash
# Run a GUI command under a private Xvfb with Mesa software rendering
# (used for headless visual QA inside the dev container).
set -euo pipefail
DISP=:$((90 + RANDOM % 9))
Xvfb $DISP -screen 0 ${XRUN_SIZE:-2880x1800x24} -nolisten tcp >/dev/null 2>&1 &
XPID=$!
trap 'kill $XPID 2>/dev/null || true' EXIT
for i in $(seq 50); do [ -e /tmp/.X11-unix/X${DISP#:} ] && break; sleep 0.1; done
export DISPLAY=$DISP QT_QPA_PLATFORM=xcb LIBGL_ALWAYS_SOFTWARE=1 QSG_RHI_BACKEND=opengl
export QML_IMPORT_PATH=${QML_IMPORT_PATH:-/src/build/qml}
"$@"
