# Lumen.Glass — Liquid Glass rendering engine
qt_add_library(LumenGlass SHARED)
qt_add_qml_module(LumenGlass
    URI Lumen.Glass
    VERSION 1.0
    PLUGIN_TARGET LumenGlassPlugin
    SOURCES
        src/materials.h
        src/glass.h src/glass.cpp
        src/backdroptracker.h src/backdroptracker.cpp
    QML_FILES
        qml/BackdropBlur.qml
        qml/GlassSurface.qml
    IMPORTS
        Lumen.Design
)
qt_add_shaders(LumenGlass "lumen_glass_shaders"
    PREFIX "/lumen/shaders"
    BASE ${CMAKE_SOURCE_DIR}/shaders
    FILES
        ${CMAKE_SOURCE_DIR}/shaders/kawase_down.frag
        ${CMAKE_SOURCE_DIR}/shaders/kawase_up.frag
        ${CMAKE_SOURCE_DIR}/shaders/glass.frag
)
target_include_directories(LumenGlass PUBLIC src)
target_link_libraries(LumenGlass PUBLIC Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick LumenDesign)
set_target_properties(LumenGlass PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)
lumen_install_qml_module(LumenGlass LumenGlassPlugin Lumen/Glass)
