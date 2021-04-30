set(CSKK_SOURCES
        cskk.cpp
    )
add_library(fcitx-cskk MODULE ${CSKK_SOURCES})
target_link_libraries(fcitx-cskk
        Fcitx5::Core
        Fcitx5::Config
        ${LIBCSKK_LIBRARIES}
        )

install(TARGETS fcitx-cskk DESTINATION "${CMAKE_INSTALL_LIBDIR}/fcitx5")

fcitx5_translate_desktop_file(cskk.conf.in cskk.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cskk.conf" DESTINATION "${CMAKE_INSTALL_DATADIR}/fcitx5/inputmethod")

configure_file(cskk-addon.conf.in.in cskk-addon.conf.in @ONLY)
fcitx5_translate_desktop_file("${CMAKE_CURRENT_BINARY_DIR}/cskk-addon.conf.in" cskk-addon.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cskk-addon.conf" RENAME cskk.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")
