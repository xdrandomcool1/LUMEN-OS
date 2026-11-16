find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(KF6KIO REQUIRED)

qt_add_executable(lumen-finder src/main.cpp)
file(GLOB FINDER_QML RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} qml/*.qml)
foreach(f ${FINDER_QML})
    get_filename_component(_n ${f} NAME)
    set_source_files_properties(${f} PROPERTIES QT_RESOURCE_ALIAS ${_n})
endforeach()
qt_add_qml_module(lumen-finder
    URI Lumen.Finder
    VERSION 1.0
    QML_FILES ${FINDER_QML}
    SOURCES src/foldermodel.h src/foldermodel.cpp
    IMPORTS Lumen.Design Lumen.Motion Lumen.Glass Lumen.Controls Lumen.System
)
target_include_directories(lumen-finder PRIVATE src)
target_link_libraries(lumen-finder PRIVATE Qt6::Quick Qt6::Widgets KF6::KIOCore KF6::KIOGui KF6::KIOWidgets KF6::KIOFileWidgets
    LumenControls LumenSystem)
install(TARGETS lumen-finder RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES lumen-finder.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
