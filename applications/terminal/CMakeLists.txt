find_package(PkgConfig REQUIRED)
pkg_check_modules(VTERM REQUIRED IMPORTED_TARGET vterm)
qt_add_executable(lumen-terminal src/main.cpp)
qt_add_qml_module(lumen-terminal
    URI Lumen.Terminal
    VERSION 1.0
    QML_FILES qml/Main.qml
    SOURCES src/terminalitem.h src/terminalitem.cpp
    IMPORTS Lumen.Design Lumen.Motion Lumen.Controls
)
set_source_files_properties(qml/Main.qml PROPERTIES QT_RESOURCE_ALIAS Main.qml)
target_include_directories(lumen-terminal PRIVATE src)
target_link_libraries(lumen-terminal PRIVATE Qt6::Quick PkgConfig::VTERM util LumenControls)
install(TARGETS lumen-terminal RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES lumen-terminal.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
