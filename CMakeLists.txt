cmake_minimum_required(VERSION 3.25)
project(LumenOS VERSION 1.0.0 DESCRIPTION "LUMEN OS desktop" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(LumenQml)

find_package(Qt6 6.8 REQUIRED COMPONENTS
    Core Gui Qml Quick QuickControls2 QuickTemplates2 ShaderTools DBus Svg Concurrent Network Test QuickTest)
qt_standard_project_setup(REQUIRES 6.8)
qt_policy(SET QTP0004 NEW)

# All QML modules are emitted into one tree so that apps and tests run from the
# build directory without installation (QML_IMPORT_PATH=<build>/qml).
set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml)
set(LUMEN_QML_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/qt6/qml CACHE PATH "QML module install dir")
set(LUMEN_DATA_DIR ${CMAKE_INSTALL_DATADIR}/lumen)

option(LUMEN_BUILD_SHELL        "Build lumen-shell"              ON)
option(LUMEN_BUILD_APPS         "Build native applications"      ON)
option(LUMEN_BUILD_KWIN         "Build KWin integration"         ON)
option(LUMEN_BUILD_TESTS        "Build tests"                    ON)

add_compile_definitions(LUMEN_VERSION="${PROJECT_VERSION}"
                        QT_NO_KEYWORDS_DISABLED)

add_subdirectory(animation-engine)
add_subdirectory(design-system)
add_subdirectory(rendering)
add_subdirectory(design-system/controls)
if(LUMEN_BUILD_SHELL)
    add_subdirectory(shell)
endif()
if(LUMEN_BUILD_APPS)
    add_subdirectory(applications)
endif()
if(LUMEN_BUILD_KWIN)
    add_subdirectory(compositor)
endif()
if(LUMEN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
