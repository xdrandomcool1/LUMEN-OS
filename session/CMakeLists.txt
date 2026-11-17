install(PROGRAMS startlumen DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES lumen.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/wayland-sessions)
install(FILES plasmashell-lumen.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/user/plasma-plasmashell.service.d RENAME lumen.conf)
install(FILES lumen-shell-condition.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/user/lumen-shell.service.d RENAME session.conf)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/themes/symbols/regular ${CMAKE_SOURCE_DIR}/themes/symbols/bold ${CMAKE_SOURCE_DIR}/themes/symbols/fill
        DESTINATION ${CMAKE_INSTALL_DATADIR}/lumen/symbols)
install(FILES ${CMAKE_SOURCE_DIR}/themes/color-schemes/LumenLight.colors ${CMAKE_SOURCE_DIR}/themes/color-schemes/LumenDark.colors
        DESTINATION ${CMAKE_INSTALL_DATADIR}/color-schemes)
install(FILES ${CMAKE_SOURCE_DIR}/assets/logo/lumen-logo.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps)
