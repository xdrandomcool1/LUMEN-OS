# Lumen.Motion — animation engine QML module
qt_add_library(LumenMotion SHARED)
qt_add_qml_module(LumenMotion
    URI Lumen.Motion
    VERSION 1.0
    PLUGIN_TARGET LumenMotionPlugin
    SOURCES
        src/spring.h src/spring.cpp
        src/motion.h src/motion.cpp
)
target_include_directories(LumenMotion PUBLIC src)
target_link_libraries(LumenMotion PUBLIC Qt6::Core Qt6::Qml Qt6::Quick)
set_target_properties(LumenMotion PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)

lumen_install_qml_module(LumenMotion LumenMotionPlugin Lumen/Motion)
