cmake_minimum_required(VERSION 3.0)
project(fcitx5-cskk VERSION 0.2.0)
set(CMAKE_CXX_STANDARD 20)

find_package(ECM 1.0.0 REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(FeatureSummary)
include(GNUInstallDirs)
include(ECMUninstallTarget)

find_package(PkgConfig REQUIRED)
find_package(Fcitx5Core REQUIRED)
pkg_check_modules(LIBCSKK REQUIRED "cskk>=0.1.0")

include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

# Not sure what this does. Need translation?
find_package(Gettext REQUIRED)
add_definitions(-DFCITX_GETTEXT_DOMAIN=\"fcitx5-cskk\" -D_GNU_SOURCE)
fcitx5_add_i18n_definition()

add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(po)

fcitx5_translate_desktop_file(
        org.fcitx.Fcitx5.Addon.Cskk.metainfo.xml.in
        org.fcitx.Fcitx5.Addon.Cskk.metainfo.xml XML)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/org.fcitx.Fcitx5.Addon.Cskk.metainfo.xml" DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
