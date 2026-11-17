# lumend (Rust) — built with cargo, installed like any other component.
find_program(CARGO cargo REQUIRED)
set(LUMEND_TARGET_DIR ${CMAKE_BINARY_DIR}/cargo)
add_custom_target(lumend ALL
    COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${LUMEND_TARGET_DIR} ${CARGO} build --release --locked
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lumend
    COMMENT "Building lumend (cargo)"
    USES_TERMINAL)
install(PROGRAMS ${LUMEND_TARGET_DIR}/release/lumend DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(PROGRAMS nvidia/lumen-nvidia-setup DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(FILES lumend/lumend.service DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/user)
install(FILES lumend/org.lumen.Search.service DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
if(LUMEN_BUILD_TESTS)
    add_test(NAME lumend-unit COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${LUMEND_TARGET_DIR} ${CARGO} test --release --locked
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lumend)
endif()
