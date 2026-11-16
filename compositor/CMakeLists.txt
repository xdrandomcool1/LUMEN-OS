# KWin integration: decoration, gestures plugin, scripted effects, switcher
# layout and distribution defaults.
add_subdirectory(decoration)
add_subdirectory(effects/gestures)
install(DIRECTORY effects/lumen_windows DESTINATION ${CMAKE_INSTALL_DATADIR}/kwin/effects)
install(DIRECTORY tabbox/lumen-switcher DESTINATION ${CMAKE_INSTALL_DATADIR}/kwin/tabbox)
install(FILES config/kwinrc config/kglobalshortcutsrc config/kdeglobals config/kcminputrc config/kscreenlockerrc
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/xdg)
install(FILES config/lumenrc DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/xdg/lumen)
