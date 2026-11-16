install(DIRECTORY sddm/lumen DESTINATION ${CMAKE_INSTALL_DATADIR}/sddm/themes)
install(FILES plymouth/lumen/lumen.plymouth plymouth/lumen/lumen.script plymouth/lumen/logo.png plymouth/lumen/glow.png
              plymouth/lumen/track.png plymouth/lumen/fill.png plymouth/lumen/field.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/plymouth/themes/lumen)
