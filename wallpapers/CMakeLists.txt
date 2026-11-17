# Renders the procedural wallpaper collection (cached: re-runs only when the
# generator or the collection description changes).
option(LUMEN_RENDER_WALLPAPERS "Render wallpapers during the build" ON)
set(LUMEN_WALLPAPER_SIZE "3840x2160" CACHE STRING "Wallpaper resolution")
if(LUMEN_RENDER_WALLPAPERS)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    string(REPLACE "x" ";" _wh ${LUMEN_WALLPAPER_SIZE})
    list(GET _wh 0 _w)
    list(GET _wh 1 _h)
    set(_out ${CMAKE_BINARY_DIR}/wallpapers)
    add_custom_command(OUTPUT ${_out}/.stamp
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/generate.py --out ${_out} --width ${_w} --height ${_h}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/collection.json ${_out}/collection.json
        COMMAND ${CMAKE_COMMAND} -E touch ${_out}/.stamp
        DEPENDS generate.py collection.json
        COMMENT "Rendering LUMEN wallpapers (${LUMEN_WALLPAPER_SIZE})")
    add_custom_target(wallpapers ALL DEPENDS ${_out}/.stamp)
    install(DIRECTORY ${_out}/ DESTINATION ${CMAKE_INSTALL_DATADIR}/lumen/wallpapers FILES_MATCHING PATTERN "*.jpg" PATTERN "*.json")
    # Also expose them to KDE/SDDM pickers.
    install(DIRECTORY ${_out}/ DESTINATION ${CMAKE_INSTALL_DATADIR}/backgrounds/lumen FILES_MATCHING PATTERN "*.jpg")
endif()
