#!/usr/bin/env python3
# Guest driver for scripts/qemu-glass-test.sh (runs next to QEMU in the test
# container): waits for the LUMEN session, drives it with QMP key events,
# captures screenshots and runs checks in the guest through the root debug
# shell on the second serial port (/tmp/dbg.sock).
import json
import os
import socket
import subprocess
import time

OUT = "/src/out/glass-test"

for _ in range(60):
    if os.path.exists("/tmp/qmp.sock"):
        break
    time.sleep(1)
qs = socket.socket(socket.AF_UNIX)
qs.connect("/tmp/qmp.sock")
qf = qs.makefile("rw")
qf.readline()


def qmp(cmd, **args):
    qf.write(json.dumps({"execute": cmd, "arguments": args}) + "\n")
    qf.flush()
    while True:
        r = json.loads(qf.readline())
        if "return" in r or "error" in r:
            return r


qmp("qmp_capabilities")


def shot(name):
    qmp("screendump", filename="/tmp/s.ppm")
    subprocess.run(["magick", "/tmp/s.ppm", f"{OUT}/{name}.png"], check=False)
    print("captured", name, flush=True)


KEYMAP = {" ": "spc", "-": "minus", "'": "apostrophe", ".": "dot", "/": "slash"}


def keys(*names):
    qmp("send-key", keys=[{"type": "qcode", "data": k} for k in names])
    time.sleep(0.4)


def type_text(text):
    for ch in text:
        if ch == "'":
            keys("apostrophe")
        else:
            keys(KEYMAP.get(ch, ch))


def journal():
    with open(f"{OUT}/journal.log", errors="replace") as j:
        return j.read()


def guest(cmd, timeout=90):
    """Run a shell command as root in the guest, return its output."""
    d = socket.socket(socket.AF_UNIX)
    d.connect("/tmp/dbg.sock")
    d.settimeout(2)
    marker = "__END_%d__" % int(time.time() * 1000)
    d.sendall(("\n" + cmd + "; echo " + marker + "\n").encode())
    buf = b""
    end = time.time() + timeout
    while time.time() < end and buf.count(marker.encode()) < 2:
        try:
            buf += d.recv(65536)
        except socket.timeout:
            pass
    d.close()
    out = buf.decode(errors="replace")
    parts = out.split(marker)
    return parts[1] if len(parts) >= 3 else out


AS_USER = ("U=$(id -u liveuser); sudo -u liveuser XDG_RUNTIME_DIR=/run/user/$U "
           "DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$U/bus ")


def diag():
    checks = [
        ("lumen_glass D-Bus Available()",
         AS_USER + "busctl --user call org.kde.KWin /org/lumen/Glass org.lumen.Glass Available"),
        ("loaded KWin effects",
         AS_USER + "busctl --user get-property org.kde.KWin /Effects org.kde.kwin.Effects loadedEffects"),
        ("KWin compositing",
         AS_USER + "busctl --user call org.kde.KWin /KWin org.kde.KWin supportInformation"
         " | sed 's/\\\\n/\\n/g' | grep -iE 'Compositing Type|OpenGL renderer|OpenGL version' | head"),
        ("user kdeglobals", "cat /home/liveuser/.config/kdeglobals"),
        ("look-and-feel packages", "ls /usr/share/plasma/look-and-feel"),
        ("welcome center running", "pgrep -a plasma-welcome || echo no"),
    ]
    with open(f"{OUT}/diag.txt", "w") as out:
        for title, cmd in checks:
            out.write("== " + title + "\n" + guest(cmd).strip() + "\n\n")
    print("diagnostics written", flush=True)


# Wait for the shell, then let the session settle.
end = time.time() + int(os.environ.get("BOOT_MIN", "12")) * 60
while time.time() < end and "lumen-shell" not in journal():
    time.sleep(10)
time.sleep(150)
shot("desktop")

# Terminal via Spotlight, maximised, filled with text.
keys("meta_l", "spc")
time.sleep(20)
type_text("terminal")
time.sleep(15)
keys("ret")
time.sleep(90)
shot("terminal")
keys("meta_l", "pgup")
time.sleep(45)
type_text("seq -s ' LUMEN glass ' 4000")
keys("ret")
time.sleep(30)
shot("maximized")

# Control Center over the text: refracted by the compositor.
keys("meta_l", "c")
time.sleep(45)
shot("controlcenter")
diag()
