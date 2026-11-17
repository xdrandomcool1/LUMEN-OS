install(PROGRAMS startlumen DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES lumen.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/wayland-sessions)
install(FILES plasmashell-lumen.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/user/plasma-plasmashell.service.d RENAME lumen.conf)
install(FILES lumen-shell-condition.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/user/lumen-shell.service.d RENAME session.conf)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/themes/icons/Lumen DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/themes/symbols/regular ${CMAKE_SOURCE_DIR}/themes/symbols/bold ${CMAKE_SOURCE_DIR}/themes/symbols/fill
        DESTINATION ${CMAKE_INSTALL_DATADIR}/lumen/symbols)
install(FILES ${CMAKE_SOURCE_DIR}/themes/color-schemes/LumenLight.colors ${CMAKE_SOURCE_DIR}/themes/color-schemes/LumenDark.colors
        DESTINATION ${CMAKE_INSTALL_DATADIR}/color-schemes)
install(FILES ${CMAKE_SOURCE_DIR}/assets/logo/lumen-logo.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps)
# Global theme (look-and-feel) package: Plasma copies its defaults into
# ~/.config/kdedefaults, which overrides /etc/xdg.
install(DIRECTORY ${CMAKE_SOURCE_DIR}/themes/look-and-feel/org.lumen.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/plasma/look-and-feel)
# App icons also in hicolor so they resolve under any icon theme.
file(GLOB LUMEN_APP_ICONS ${CMAKE_SOURCE_DIR}/themes/icons/Lumen/scalable/apps/lumen-*.svg)
install(FILES ${LUMEN_APP_ICONS} DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
