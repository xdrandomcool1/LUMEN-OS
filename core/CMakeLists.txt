# Lumen.System — system integration back-ends shared by the shell and apps:
# settings store, wallpapers, NetworkManager, BlueZ (via BluezQt in QML),
# PipeWire (PulseAudioQt), UPower, PowerDevil brightness, KWin keyboard
# layouts, /proc statistics, application catalogue and file search.
foreach(_kf GlobalAccel KIO Service NetworkManagerQt PulseAudioQt)
    find_package(KF6${_kf} REQUIRED)
endforeach()

qt_add_library(LumenSystem SHARED)
qt_add_qml_module(LumenSystem
    URI Lumen.System
    VERSION 1.0
    PLUGIN_TARGET LumenSystemPlugin
    SOURCES
        src/shellsettings.h src/shellsettings.cpp
        src/wallpaperengine.h src/wallpaperengine.cpp
        src/battery.h src/battery.cpp
        src/keyboardlayout.h src/keyboardlayout.cpp
        src/audio.h src/audio.cpp
        src/network.h src/network.cpp
        src/brightness.h src/brightness.cpp
        src/sysstats.h src/sysstats.cpp
        src/appsmodel.h src/appsmodel.cpp
        src/filesearch.h src/filesearch.cpp
        src/systemactions.h src/systemactions.cpp
        src/themesync.h src/themesync.cpp
)
target_include_directories(LumenSystem PUBLIC src)
target_compile_definitions(LumenSystem PRIVATE LUMEN_LIBEXECDIR="${CMAKE_INSTALL_FULL_LIBEXECDIR}")
target_link_libraries(LumenSystem PUBLIC LumenDesign Qt6::Core Qt6::Gui Qt6::Qml Qt6::DBus Qt6::Concurrent
    KF6::KIOGui KF6::KIOCore KF6::Service KF6::NetworkManagerQt KF6::PulseAudioQt)
set_target_properties(LumenSystem PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)
lumen_install_qml_module(LumenSystem LumenSystemPlugin Lumen/System)
