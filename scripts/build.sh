#!/bin/bash
# Configure + build everything (inside the dev container) and run unit tests.
#   scripts/build.sh [--no-tests] [extra cmake args]
set -euo pipefail
cd "$(dirname "$0")/.."
if [ ! -f /.dockerenv ] && [ ! -f /run/.containerenv ]; then
    exec scripts/dev.sh scripts/build.sh "$@"
fi
TESTS=1
if [ "${1:-}" = "--no-tests" ]; then TESTS=0; shift; fi
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=/usr "$@"
cmake --build build
if [ "$TESTS" = 1 ]; then
    QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
fi
