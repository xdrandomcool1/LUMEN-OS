# Installs a QML module built with qt_add_qml_module (backing library +
# plugin + qmldir + qmltypes). QML/JS/shader files are compiled into the
# backing library's resources (qmldir uses `prefer :/qt/qml/...`).
function(lumen_install_qml_module backing plugin uri_path)
    install(TARGETS ${backing} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(TARGETS ${plugin} LIBRARY DESTINATION ${LUMEN_QML_INSTALL_DIR}/${uri_path})
    install(FILES ${CMAKE_BINARY_DIR}/qml/${uri_path}/qmldir
                  ${CMAKE_BINARY_DIR}/qml/${uri_path}/${backing}.qmltypes
            DESTINATION ${LUMEN_QML_INSTALL_DIR}/${uri_path})
endfunction()
