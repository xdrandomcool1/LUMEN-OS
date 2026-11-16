qt_add_executable(lumen-screenshot main.cpp capture.h capture.cpp)
qt_add_qml_module(lumen-screenshot
    URI Lumen.Screenshot
    VERSION 1.0
    QML_FILES Main.qml
    SOURCES screenshooter.h screenshooter.cpp
    IMPORTS Lumen.Design Lumen.Glass Lumen.Controls
)
target_include_directories(lumen-screenshot PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lumen-screenshot PRIVATE Qt6::Quick Qt6::DBus Qt6::Gui LumenControls)
install(TARGETS lumen-screenshot RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES lumen-screenshot.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
