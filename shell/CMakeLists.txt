# lumen-shell
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(LayerShellQt REQUIRED)
# LibTaskManager / LibNotificationManager do not find their own KF6
# dependencies; satisfy their link interfaces explicitly.
foreach(_kf GlobalAccel Config CoreAddons I18n WindowSystem Service ItemModels KIO)
    find_package(KF6${_kf} REQUIRED)
endforeach()
find_package(LibTaskManager REQUIRED)
find_package(LibNotificationManager REQUIRED)
find_package(dbusmenu-lxqt REQUIRED)

qt_add_executable(lumen-shell src/main.cpp)
file(GLOB_RECURSE SHELL_QML RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} qml/*.qml qml/*.js)
foreach(f ${SHELL_QML})
    get_filename_component(_n ${f} NAME)
    set_source_files_properties(${f} PROPERTIES QT_RESOURCE_ALIAS ${_n})
endforeach()
set_source_files_properties(qml/ShellState.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE QT_RESOURCE_ALIAS ShellState.qml)
set_source_files_properties(qml/MenuDefinitions.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE QT_RESOURCE_ALIAS MenuDefinitions.qml)

qt_add_qml_module(lumen-shell
    URI Lumen.Shell
    VERSION 1.0
    SOURCES
        src/layersurface.h src/layersurface.cpp
        src/shellcontroller.h src/shellcontroller.cpp
        src/regionwatcher.h src/regionwatcher.cpp
        src/appmenu.h src/appmenu.cpp
    QML_FILES ${SHELL_QML}
    IMPORTS Lumen.Design Lumen.Motion Lumen.Glass Lumen.Controls Lumen.System
)
target_include_directories(lumen-shell PRIVATE src)
target_link_libraries(lumen-shell PRIVATE
    Qt6::Quick Qt6::Qml Qt6::DBus Qt6::Widgets Qt6::Concurrent
    LayerShellQt::Interface
    KF6::GlobalAccel
    PW::LibTaskManager PW::LibNotificationManager dbusmenu-lxqt
    LumenDesign LumenGlass LumenControls LumenMotion LumenSystem)

install(TARGETS lumen-shell RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES data/lumen-shell.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES data/lumen-shell.service DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/user)
