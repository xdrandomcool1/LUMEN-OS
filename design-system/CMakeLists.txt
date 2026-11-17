# Lumen.Design — tokens, appearance, typography, symbols
find_package(Qt6 REQUIRED COMPONENTS Svg)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_library(LumenDesign SHARED)
qt_add_qml_module(LumenDesign
    URI Lumen.Design
    VERSION 1.0
    PLUGIN_TARGET LumenDesignPlugin
    SOURCES
        src/appearance.h src/appearance.cpp
        src/symbol.h src/symbol.cpp
    QML_FILES
        qml/Theme.qml
        qml/LText.qml
    IMPORTS
        Lumen.Motion
)

# Compile the symbol set into the library (≈250 glyphs × 3 weights, ~1 MB).
file(GLOB LUMEN_SYMBOLS RELATIVE ${CMAKE_SOURCE_DIR}/themes
     ${CMAKE_SOURCE_DIR}/themes/symbols/regular/*.svg
     ${CMAKE_SOURCE_DIR}/themes/symbols/bold/*.svg
     ${CMAKE_SOURCE_DIR}/themes/symbols/fill/*.svg)
set(_sym_abs)
foreach(s ${LUMEN_SYMBOLS})
    set_source_files_properties(${CMAKE_SOURCE_DIR}/themes/${s} PROPERTIES QT_RESOURCE_ALIAS ${s})
    list(APPEND _sym_abs ${CMAKE_SOURCE_DIR}/themes/${s})
endforeach()
qt_add_resources(LumenDesign "lumen_symbols" PREFIX "/lumen" FILES ${_sym_abs})

target_include_directories(LumenDesign PUBLIC src)
target_link_libraries(LumenDesign PUBLIC Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::Svg LumenMotion)
set_target_properties(LumenDesign PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)
lumen_install_qml_module(LumenDesign LumenDesignPlugin Lumen/Design)

