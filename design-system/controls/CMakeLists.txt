# Lumen.Controls — native controls and window chrome
find_package(KF6WindowSystem REQUIRED)

qt_add_library(LumenControls SHARED)
qt_add_qml_module(LumenControls
    URI Lumen.Controls
    VERSION 1.0
    PLUGIN_TARGET LumenControlsPlugin
    SOURCES
        src/windoweffects.h src/windoweffects.cpp
    QML_FILES
        qml/AppWindow.qml
        qml/Button.qml
        qml/CheckBox.qml
        qml/FormRow.qml
        qml/FormSection.qml
        qml/Menu.qml
        qml/MenuItem.qml
        qml/MenuSeparator.qml
        qml/PopUpButton.qml
        qml/ProgressBar.qml
        qml/RoundedClip.qml
        qml/ScrollBar.qml
        qml/SearchField.qml
        qml/SegmentedControl.qml
        qml/Separator.qml
        qml/SidebarHeader.qml
        qml/SidebarItem.qml
        qml/Slider.qml
        qml/Spinner.qml
        qml/TextField.qml
        qml/Toggle.qml
        qml/ToolTip.qml
        qml/ToolbarButton.qml
        qml/ToolbarGroup.qml
        qml/TrafficLights.qml
    IMPORTS
        Lumen.Design
        Lumen.Motion
        Lumen.Glass
        QtQuick.Templates
)
qt_add_shaders(LumenControls "lumen_controls_shaders"
    PREFIX "/lumen/shaders"
    BASE ${CMAKE_SOURCE_DIR}/shaders
    FILES ${CMAKE_SOURCE_DIR}/shaders/roundedclip.frag
)
target_include_directories(LumenControls PUBLIC src)
target_link_libraries(LumenControls PUBLIC Qt6::Quick Qt6::QuickTemplates2 KF6::WindowSystem LumenDesign LumenGlass)
set_target_properties(LumenControls PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)
lumen_install_qml_module(LumenControls LumenControlsPlugin Lumen/Controls)
