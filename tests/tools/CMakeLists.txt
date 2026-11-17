# lumen-qmlgrab: renders a QML scene and writes PNG frames (visual QA).
qt_add_executable(lumen-qmlgrab qmlgrab.cpp)
target_link_libraries(lumen-qmlgrab PRIVATE Qt6::Quick Qt6::Qml Qt6::Gui)
