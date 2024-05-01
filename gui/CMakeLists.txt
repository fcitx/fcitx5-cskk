set(CSKK_CONFIG_SRCS
        main.cpp
        dictwidget.cpp
        adddictdialog.cpp
        dictmodel.cpp
        )

if (NOT ENABLE_QT)
    return()
endif ()

add_library(fcitx5-cskk-config
        MODULE ${CSKK_CONFIG_SRCS})

set_target_properties(fcitx5-cskk-config PROPERTIES
        AUTOMOC TRUE
        AUTOUIC TRUE
        AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h"
        )
target_link_libraries(fcitx5-cskk-config
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Widgets
        Fcitx5Qt${QT_MAJOR_VERSION}::WidgetsAddons
        Fcitx5::Utils
        PkgConfig::LIBCSKK
        )

# Must install to same lib dir that Fcitx5Utils specify using FCITX_*DIR so that fcitx5 can find.
install(TARGETS fcitx5-cskk-config DESTINATION ${FCITX_INSTALL_LIBDIR}/fcitx5/qt${QT_MAJOR_VERSION})
