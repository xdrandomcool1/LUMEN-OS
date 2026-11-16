install(FILES calamares/settings.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/calamares)
install(DIRECTORY calamares/modules DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/calamares)
install(DIRECTORY calamares/branding/lumen DESTINATION ${CMAKE_INSTALL_DATADIR}/calamares/branding)
