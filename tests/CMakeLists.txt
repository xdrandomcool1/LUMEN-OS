add_subdirectory(tools)

qt_add_executable(tst_core unit/tst_core.cpp)
target_link_libraries(tst_core PRIVATE Qt6::Test LumenMotion LumenDesign LumenGlass LumenSystem)
add_test(NAME core-unit COMMAND tst_core)

find_program(QMLTESTRUNNER qmltestrunner PATHS /usr/lib64/qt6/bin)
if(QMLTESTRUNNER)
    add_test(NAME js-engines COMMAND ${QMLTESTRUNNER} -input ${CMAKE_CURRENT_SOURCE_DIR}/qml -import ${CMAKE_BINARY_DIR}/qml)
endif()

find_package(KF6Config REQUIRED)
find_package(KF6Screen REQUIRED)
find_library(CRYPT_LIB crypt REQUIRED)
qt_add_executable(tst_apps unit/tst_apps.cpp
    ${CMAKE_SOURCE_DIR}/applications/activity-monitor/src/processmodel.cpp
    ${CMAKE_SOURCE_DIR}/applications/store/src/flatpak.cpp
    ${CMAKE_SOURCE_DIR}/applications/settings/src/backends.cpp
    ${CMAKE_SOURCE_DIR}/applications/utilities/texteditor/src/document.cpp)
target_include_directories(tst_apps PRIVATE ${CMAKE_SOURCE_DIR}/applications/settings/src)
target_link_libraries(tst_apps PRIVATE Qt6::Test Qt6::Qml Qt6::DBus KF6::ConfigCore KF6::Screen ${CRYPT_LIB})
add_test(NAME apps-unit COMMAND tst_apps)
