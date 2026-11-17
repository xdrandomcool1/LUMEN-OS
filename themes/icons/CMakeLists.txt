# Lumen icon theme. The SVGs are generated (themes/icons/generate_icons.py)
# and shipped as scalable sources; rsvg-convert (librsvg2-tools) additionally
# renders 128/256 px PNGs with the full artwork (shadows, streaks) for icon
# contexts that want raster sizes.
set(LUMEN_ICONS ${CMAKE_CURRENT_SOURCE_DIR}/Lumen)

install(DIRECTORY ${LUMEN_ICONS} DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)

find_program(RSVG_CONVERT rsvg-convert)
if(RSVG_CONVERT AND UNIX)
    set(_all_pngs)
    foreach(_size 128 256)
        foreach(_ctx apps places)
            set(_png_dir ${CMAKE_CURRENT_BINARY_DIR}/Lumen/${_size}x${_size}/${_ctx})
            file(MAKE_DIRECTORY ${_png_dir})
            file(GLOB _svgs ${LUMEN_ICONS}/scalable/${_ctx}/*.svg)
            set(_pngs)
            foreach(_svg ${_svgs})
                get_filename_component(_name ${_svg} NAME_WE)
                set(_out ${_png_dir}/${_name}.png)
                add_custom_command(OUTPUT ${_out}
                    COMMAND ${RSVG_CONVERT} -w ${_size} -h ${_size} ${_svg} -o ${_out}
                    DEPENDS ${_svg} VERBATIM)
                list(APPEND _pngs ${_out})
            endforeach()
            install(DIRECTORY ${_png_dir}/
                    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/Lumen/${_size}x${_size}/${_ctx}
                    FILES_MATCHING PATTERN "*.png")
            list(APPEND _all_pngs ${_pngs})
        endforeach()
    endforeach()
    add_custom_target(lumen-icon-pngs ALL DEPENDS ${_all_pngs})
endif()
