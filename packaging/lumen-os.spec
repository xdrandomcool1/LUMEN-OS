# LUMEN OS desktop — RPM packaging (Fedora 45).
Name:           lumen-os
Version:        1.0.0
Release:        1%{?dist}
Summary:        LUMEN OS desktop environment
License:        GPL-3.0-or-later AND MIT AND CC-BY-SA-4.0
URL:            https://github.com/xdrandomcool1/LUMEN-OS
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake ninja-build gcc-c++ extra-cmake-modules
BuildRequires:  qt6-qtbase-devel qt6-qtbase-private-devel qt6-qtdeclarative-devel qt6-qtshadertools-devel
BuildRequires:  qt6-qtwayland-devel qt6-qtsvg-devel qt6-qtmultimedia-devel
BuildRequires:  layer-shell-qt-devel kwin-devel kdecoration-devel libdrm-devel libepoxy-devel wayland-devel
BuildRequires:  kf6-kwindowsystem-devel kf6-kglobalaccel-devel kf6-kconfig-devel kf6-kcoreaddons-devel
BuildRequires:  kf6-kio-devel kf6-kservice-devel kf6-networkmanager-qt-devel kf6-bluez-qt-devel
BuildRequires:  pulseaudio-qt-qt6-devel libkscreen-devel plasma-workspace-devel libdbusmenu-lxqt-devel
BuildRequires:  libvterm-devel libxcrypt-devel
BuildRequires:  rust cargo python3-pillow python3-numpy librsvg2-tools

%description
LUMEN OS is a Fedora-based desktop with a Liquid Glass interface: the
lumen-shell (menu bar, Dock, Control Center, Spotlight, Notification Center),
KWin integration, design system, animation engine and native applications.

%package -n lumen-desktop
Summary:        LUMEN shell, compositor integration, themes and session
Requires:       kwin >= 6.8
Requires:       plasma-workspace
Requires:       plasma-integration
Requires:       xdg-desktop-portal-kde
Requires:       polkit-kde
Requires:       powerdevil
Requires:       power-profiles-daemon
Requires:       kscreen
Requires:       sddm
Requires:       layer-shell-qt
Requires:       kf6-bluez-qt
Requires:       kf6-kirigami
Requires:       pipewire-pulseaudio
Requires:       wireplumber
Requires:       NetworkManager
Requires:       upower
Requires:       accountsservice
Requires:       flatpak
Requires:       rsms-inter-vf-fonts
Requires:       rsms-inter-fonts
Requires:       jetbrains-mono-fonts-all
Requires:       plymouth-plugin-script
Requires:       breeze-icon-theme
Requires:       zip

%description -n lumen-desktop
lumen-shell, the Lumen QML modules (Design, Motion, Glass, Controls, System),
the LUMEN KWin decoration, gestures and window effects, lumend, SDDM and
Plymouth themes, icon theme, colour schemes and wallpapers.

%package -n lumen-apps
Summary:        LUMEN native applications
Requires:       lumen-desktop = %{version}-%{release}
Requires:       kf6-syntax-highlighting
Requires:       qt6-qtmultimedia

%description -n lumen-apps
Finder, System Settings, Terminal, App Store, Activity Monitor, Calculator,
Calendar, Text Editor, Image Viewer, Media Player and Screenshot.

%package -n lumen-installer
Summary:        LUMEN OS installer configuration (Calamares)
Requires:       calamares
BuildArch:      noarch

%description -n lumen-installer
Calamares settings, modules and branding used on the LUMEN OS live image.

%prep
%autosetup -n %{name}-%{version}

%build
%cmake -DLUMEN_BUILD_TESTS=ON -DLUMEN_WALLPAPER_SIZE=3840x2160 -DCMAKE_INSTALL_SYSCONFDIR=%{_sysconfdir}
%cmake_build

%check
export QT_QPA_PLATFORM=offscreen
%ctest

%install
%cmake_install

%files -n lumen-desktop
%license LICENSE themes/symbols/LICENSE.phosphor
%doc README.md docs/ARCHITECTURE.md docs/DESIGN_SYSTEM.md
%{_bindir}/lumen-shell
%{_bindir}/startlumen
%{_libexecdir}/lumend
%{_libdir}/libLumen*.so*
%{_libdir}/qt6/qml/Lumen/
%{_libdir}/qt6/plugins/org.kde.kdecoration3/org.lumen.decoration.so
%{_libdir}/qt6/plugins/kwin/effects/plugins/lumen_gestures.so
%{_libdir}/qt6/plugins/kwin/effects/plugins/lumen_glass.so
%{_datadir}/kwin/effects/lumen_windows/
%{_datadir}/kwin/tabbox/lumen-switcher/
%{_datadir}/applications/lumen-shell.desktop
%{_datadir}/wayland-sessions/lumen.desktop
%{_datadir}/dbus-1/services/org.lumen.Search.service
%{_datadir}/icons/Lumen/
%{_datadir}/lumen/
%{_datadir}/backgrounds/lumen/
%{_datadir}/color-schemes/Lumen*.colors
%{_datadir}/pixmaps/lumen-logo.svg
%{_datadir}/sddm/themes/lumen/
%{_datadir}/plymouth/themes/lumen/
%{_userunitdir}/lumen-shell.service
%{_userunitdir}/lumend.service
%{_userunitdir}/lumen-shell.service.d/
%{_userunitdir}/plasma-plasmashell.service.d/
%config(noreplace) %{_sysconfdir}/xdg/kwinrc
%config(noreplace) %{_sysconfdir}/xdg/kglobalshortcutsrc
%config(noreplace) %{_sysconfdir}/xdg/kdeglobals
%config(noreplace) %{_sysconfdir}/xdg/kcminputrc
%config(noreplace) %{_sysconfdir}/xdg/kscreenlockerrc
%config(noreplace) %{_sysconfdir}/xdg/lumen/

%files -n lumen-apps
%{_bindir}/lumen-finder
%{_bindir}/lumen-settings
%{_bindir}/lumen-terminal
%{_bindir}/lumen-store
%{_bindir}/lumen-activity-monitor
%{_bindir}/lumen-calculator
%{_bindir}/lumen-calendar
%{_bindir}/lumen-texteditor
%{_bindir}/lumen-imageviewer
%{_bindir}/lumen-mediaplayer
%{_bindir}/lumen-screenshot
%{_datadir}/applications/lumen-finder.desktop
%{_datadir}/applications/lumen-settings.desktop
%{_datadir}/applications/lumen-terminal.desktop
%{_datadir}/applications/lumen-store.desktop
%{_datadir}/applications/lumen-activity-monitor.desktop
%{_datadir}/applications/lumen-calculator.desktop
%{_datadir}/applications/lumen-calendar.desktop
%{_datadir}/applications/lumen-texteditor.desktop
%{_datadir}/applications/lumen-imageviewer.desktop
%{_datadir}/applications/lumen-mediaplayer.desktop
%{_datadir}/applications/lumen-screenshot.desktop

%files -n lumen-installer
%config(noreplace) %{_sysconfdir}/calamares/
%{_datadir}/calamares/branding/lumen/

%changelog
* Mon Nov 16 2026 LUMEN OS <noreply@lumen-os.invalid> - 1.0.0-1
- LUMEN OS 1.0
