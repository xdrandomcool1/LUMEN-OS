#!/bin/bash
# LUMEN OS image configuration (runs inside the image root during kiwi build).
set -euxo pipefail

# --- Identity --------------------------------------------------------------
. /usr/lib/os-release
rm -f /etc/os-release
cat > /etc/os-release <<OS
NAME="LUMEN OS"
VERSION="1.0 (based on Fedora Linux ${VERSION_ID})"
ID=lumen
ID_LIKE=fedora
VERSION_ID=1.0
PRETTY_NAME="LUMEN OS 1.0"
LOGO=lumen-logo
HOME_URL="https://github.com/xdrandomcool1/LUMEN-OS"
LUMEN_VERSION=1.0.0
FEDORA_VERSION_ID=${VERSION_ID}
PLATFORM_ID="${PLATFORM_ID:-platform:f${VERSION_ID}}"
CPE_NAME="cpe:/o:lumen:lumen_os:1.0"
OS
echo "LUMEN OS 1.0 \\n \\l" > /etc/issue

# --- Services ----------------------------------------------------------------
systemctl enable sddm.service NetworkManager.service bluetooth.service firewalld.service power-profiles-daemon.service
systemctl set-default graphical.target
systemctl --global enable lumen-shell.service lumend.service || true

# --- Login / session -----------------------------------------------------------
# LUMEN is the only desktop session offered.
rm -f /usr/share/wayland-sessions/plasma*.desktop /usr/share/xsessions/plasma*.desktop
plymouth-set-default-theme lumen

# --- Live user -------------------------------------------------------------------
echo "liveuser ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/liveuser
chmod 0440 /etc/sudoers.d/liveuser
passwd -d liveuser
mkdir -p /home/liveuser/.config/lumen
cp /etc/skel/.config/lumen/lumenrc /home/liveuser/.config/lumen/lumenrc
cp /etc/skel/.config/kdeglobals /home/liveuser/.config/kdeglobals
# Pin the installer in the live session's Dock.
sed -i 's|^pinned=|pinned=applications:calamares.desktop,|' /home/liveuser/.config/lumen/lumenrc
chown -R liveuser: /home/liveuser

# --- Apps -------------------------------------------------------------------------
flatpak remote-add --system --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo || true

# Remove the build-environment CA seeded by build-iso.sh (never shipped).
rm -f /var/tmp/lumen-build-ca.crt

# Keep the image lean.
rm -rf /var/cache/dnf/* /usr/share/doc/* || true
