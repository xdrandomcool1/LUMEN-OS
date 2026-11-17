find_package(Qt6 REQUIRED COMPONENTS Widgets)
foreach(_kf Config Screen)
    find_package(KF6${_kf} REQUIRED)
endforeach()
find_library(CRYPT_LIBRARY crypt REQUIRED)

qt_add_executable(lumen-settings src/main.cpp)
file(GLOB SETTINGS_QML RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} qml/*.qml)
foreach(f ${SETTINGS_QML})
    get_filename_component(_n ${f} NAME)
    set_source_files_properties(${f} PROPERTIES QT_RESOURCE_ALIAS ${_n})
endforeach()
qt_add_qml_module(lumen-settings
    URI Lumen.Settings
    VERSION 1.0
    QML_FILES ${SETTINGS_QML}
    SOURCES src/backends.h src/backends.cpp
    IMPORTS Lumen.Design Lumen.Motion Lumen.Glass Lumen.Controls Lumen.System
)
target_include_directories(lumen-settings PRIVATE src)
target_link_libraries(lumen-settings PRIVATE Qt6::Quick Qt6::Widgets Qt6::DBus KF6::ConfigCore KF6::Screen ${CRYPT_LIBRARY}
    LumenControls LumenSystem)
install(TARGETS lumen-settings RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES lumen-settings.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
