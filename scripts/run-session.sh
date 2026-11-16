#!/bin/bash
# Run a complete LUMEN session headlessly (KWin Wayland virtual backend +
# lumen-shell + optional apps) from the build tree, optionally capturing
# screenshots. Runs inside the dev container (scripts/dev.sh).
#
#   scripts/run-session.sh --shot build/shots/desktop.png
#   scripts/run-session.sh --size 1728x1117 --scale 1 --appearance light \
#       --exec "lumen-finder" --panel controlcenter --shot out.png
#
# Options:
#   --compositor C    kwin (default; the production compositor) or sway
#                     (wlroots headless+pixman: software screen capture)
#   --size WxH        virtual output size (default 1440x900)
#   --scale S         output scale (default 1)
#   --appearance M    light | dark | auto
#   --exec CMD        start a client (repeatable; build binaries are on PATH)
#   --panel NAME      open a shell panel before capturing (spotlight, …)
#   --dbus CMD        run a shell command against the session bus before capture
#   --wait SECONDS    settle time before capture (default 6)
#   --shot FILE       capture the workspace into FILE (repeatable with --then)
#   --capture P=FILE  open shell panel P (or "none"), then capture FILE (repeatable)
#   --interactive     keep running until killed
set -euo pipefail
if [ ! -f /.dockerenv ] && [ ! -f /run/.containerenv ]; then
    exec env LUMEN_DEV_ARGS="${LUMEN_DEV_ARGS:-}" "$(dirname "$0")/dev.sh" "$(basename "$0" | sed 's|^|scripts/|')" "$@"
fi
if [ -z "${DBUS_SESSION_BUS_ADDRESS:-}" ]; then
    exec dbus-run-session -- "$0" "$@"
fi

COMPOSITOR=kwin; SIZE=1440x900; SCALE=1; APPEARANCE=dark; WAIT=6; PANEL=""; INTERACTIVE=0
EXECS=(); SHOTS=(); DBUSCMDS=(); CAPTURES=()
while [ $# -gt 0 ]; do
    case "$1" in
        --compositor) COMPOSITOR=$2; shift 2;;
        --size) SIZE=$2; shift 2;;
        --scale) SCALE=$2; shift 2;;
        --appearance) APPEARANCE=$2; shift 2;;
        --exec) EXECS+=("$2"); shift 2;;
        --panel) PANEL=$2; shift 2;;
        --dbus) DBUSCMDS+=("$2"); shift 2;;
        --wait) WAIT=$2; shift 2;;
        --shot) SHOTS+=("$2"); shift 2;;
        --capture) CAPTURES+=("$2"); shift 2;;
        --interactive) INTERACTIVE=1; shift;;
        *) echo "unknown option $1"; exit 2;;
    esac
done

B=/src/build
export LUMEN_WALLPAPER_DIR=${LUMEN_WALLPAPER_DIR:-$B/wallpapers}
export HOME=/tmp/lumen-home
export XDG_RUNTIME_DIR=/tmp/lumen-xdg
export XDG_CONFIG_HOME=$HOME/.config XDG_DATA_HOME=$HOME/.local/share XDG_CACHE_HOME=$HOME/.cache
mkdir -p "$XDG_RUNTIME_DIR" "$XDG_CONFIG_HOME/lumen" "$XDG_DATA_HOME" "$XDG_CACHE_HOME" && chmod 700 "$XDG_RUNTIME_DIR"
for d in Desktop Documents Downloads Pictures Music Videos; do mkdir -p "$HOME/$d"; done
# Sample content for QA screenshots (throwaway home under /tmp).
if [ ! -f "$HOME/.seeded" ]; then
    mkdir -p "$HOME/Documents/Projects/lumen-notes" "$HOME/Documents/Invoices"
    printf '# Meeting notes\n\n- Ship LUMEN 1.0\n- Polish the Dock\n' > "$HOME/Documents/Meeting Notes.md"
    printf 'Quarter,Revenue\nQ1,120\nQ2,135\nQ3,150\n' > "$HOME/Documents/Report Q3.csv"
    printf 'Hello from LUMEN OS\n' > "$HOME/Documents/Projects/lumen-notes/README.txt"
    printf '#!/bin/sh\necho hi\n' > "$HOME/Documents/Projects/build.sh"
    touch "$HOME/Documents/Invoices/Invoice 2026-10.pdf" "$HOME/Downloads/lumen-os-1.0.iso.torrent"
    for w in lumen-dawn aurora dunes peaks; do
        [ -f "$LUMEN_WALLPAPER_DIR/$w.jpg" ] && cp "$LUMEN_WALLPAPER_DIR/$w.jpg" "$HOME/Pictures/$w.jpg"
    done
    touch "$HOME/.seeded"
fi

# Development data dir: desktop entries pointing into the build tree, icon
# theme and wallpapers, ahead of the system dirs.
DEV=$B/dev-data
mkdir -p "$DEV/applications" "$DEV/icons" "$DEV/lumen"
ln -sfn /src/themes/icons/Lumen "$DEV/icons/Lumen"
for f in $(find /src/shell/data /src/applications -name '*.desktop'); do
    sed -E "s|^Exec=([a-z-]+)|Exec=$B/bin/\1|" "$f" > "$DEV/applications/$(basename "$f")"
done
mkdir -p $B/bin
for exe in $(find $B/shell $B/applications -maxdepth 4 -type f -perm -u+x -name 'lumen-*' 2>/dev/null); do ln -sfn "$exe" "$B/bin/$(basename "$exe")"; done
export PATH=$B/bin:$PATH
export XDG_DATA_DIRS=$DEV:/usr/local/share:/usr/share
export QML_IMPORT_PATH=$B/qml
export LUMEN_WALLPAPER_DIR=${LUMEN_WALLPAPER_DIR:-$B/wallpapers}
if [ ! -f "$LUMEN_WALLPAPER_DIR/collection.json" ]; then
    mkdir -p "$LUMEN_WALLPAPER_DIR"
    python3 /src/wallpapers/generate.py --out "$LUMEN_WALLPAPER_DIR" --width 2880 --height 1800 >/dev/null
    cp /src/wallpapers/collection.json "$LUMEN_WALLPAPER_DIR/"
fi
cat > "$XDG_CONFIG_HOME/lumen/lumenrc" <<CFG
[Appearance]
mode=$APPEARANCE
CFG
printf '[Icons]\nTheme=Lumen\n' > "$XDG_CONFIG_HOME/kdeglobals"

# Software rendering in containers; dev-only permission bypasses so that
# binaries in the build tree get the privileged protocols their installed
# desktop files grant them.
export LIBGL_ALWAYS_SOFTWARE=1
export KWIN_SCREENSHOT_NO_PERMISSION_CHECKS=1 KWIN_WAYLAND_NO_PERMISSION_CHECKS=1
export XDG_CURRENT_DESKTOP=LUMEN:KDE XDG_SESSION_TYPE=wayland KDE_FULL_SESSION=true KDE_SESSION_VERSION=6
# KDE platform theme (plasma-integration): app menus over DBusMenu, icon
# theme, fonts. Avoids Qt's built-in QKdeTheme fallback.
export QT_QPA_PLATFORMTHEME=kde
export QT_FORCE_STDERR_LOGGING=1
W=${SIZE%x*}; H=${SIZE#*x}
LOG=$B/session-logs; mkdir -p "$LOG"; rm -f "$LOG"/*.log
if [ "$COMPOSITOR" = sway ]; then
    # wlroots headless backend with the pixman renderer needs no GPU and
    # supports wlr-screencopy, so `grim` can capture the composited output.
    cat > "$LOG/sway.cfg" <<SWAY
output HEADLESS-1 resolution ${W}x${H} scale $SCALE
default_border none
default_floating_border none
for_window [app_id=".*"] floating enable
seat * hide_cursor 1
SWAY
    env -u QT_QPA_PLATFORM WLR_BACKENDS=headless WLR_RENDERER=pixman WLR_HEADLESS_OUTPUTS=1 WLR_LIBINPUT_NO_DEVICES=1 \
        sway -c "$LOG/sway.cfg" >"$LOG/sway.log" 2>&1 &
    for i in $(seq 100); do SOCK=$(ls "$XDG_RUNTIME_DIR" 2>/dev/null | grep -E '^wayland-[0-9]+$' | head -1 || true); [ -n "$SOCK" ] && break; sleep 0.1; done
    [ -n "${SOCK:-}" ] || { echo "sway failed to start"; tail -20 "$LOG/sway.log"; exit 1; }
    sleep 1
    WL=$SOCK
else
    # KWin's virtual backend. Without a DRM render node (containers, CI) KWin
    # composites with QPainter; use --compositor sway for screenshots there.
    env -u QT_QPA_PLATFORM ${LUMEN_KWIN_WRAPPER:-} kwin_wayland --virtual --width "$W" --height "$H" --scale "$SCALE" --socket lumen-0 --no-lockscreen >"$LOG/kwin.log" 2>&1 &
    for i in $(seq 300); do [ -S "$XDG_RUNTIME_DIR/lumen-0" ] && break; sleep 0.1; done
    [ -S "$XDG_RUNTIME_DIR/lumen-0" ] || { echo "KWin failed to start"; tail -20 "$LOG/kwin.log"; exit 1; }
    WL=lumen-0
fi
export WAYLAND_DISPLAY=$WL QT_QPA_PLATFORM=wayland
trap 'kill $(jobs -p) 2>/dev/null || true' EXIT

${LUMEN_SHELL_WRAPPER:-} lumen-shell >"$LOG/shell.log" 2>&1 &
sleep 2
for e in "${EXECS[@]}"; do
    bash -c "$e" >>"$LOG/apps.log" 2>&1 &
    sleep 1.5
done
sleep "$WAIT"
for c in "${DBUSCMDS[@]}"; do bash -c "$c" || true; sleep 1; done
if [ -n "$PANEL" ]; then
    gdbus call --session --dest org.lumen.Shell --object-path /Shell --method org.lumen.Shell.ShowPanel "$PANEL" >/dev/null
    sleep 2
fi
for s in "${SHOTS[@]}"; do
    mkdir -p "$(dirname "$s")"
    if [ "$COMPOSITOR" = sway ]; then grim "$s" && echo "$s"; else lumen-screenshot -o "$s"; fi
done
shoot() { mkdir -p "$(dirname "$1")"; if [ "$COMPOSITOR" = sway ]; then grim "$1" && echo "$1"; else lumen-screenshot -o "$1"; fi; }
for c in "${CAPTURES[@]}"; do
    P=${c%%=*}; F=${c#*=}
    if [ "$P" = none ]; then P=""; fi
    gdbus call --session --dest org.lumen.Shell --object-path /Shell --method org.lumen.Shell.ShowPanel "$P" >/dev/null
    sleep 2.5
    shoot "$F"
done
if [ "$INTERACTIVE" = 1 ]; then wait; fi
