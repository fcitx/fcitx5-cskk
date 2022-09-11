cmake_minimum_required(VERSION 3.17)
cmake_policy(SET CMP0063 NEW)
project(fcitx5-cskk VERSION 0.6.1)
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_STANDARD 17)

find_package(ECM 1.0.0 REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(FeatureSummary)
include(GNUInstallDirs)
include(ECMUninstallTarget)

find_package(PkgConfig REQUIRED)
find_package(Fcitx5Core 5.0.6 REQUIRED)

pkg_check_modules(LIBCSKK REQUIRED IMPORTED_TARGET "cskk>=0.7.0")

option(ENABLE_QT "Enable Qt for GUI configuration" On)

option(ENABLE_QT "Enable Qt for GUI configuration" On)

include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

# GUI related libs
if (ENABLE_QT)
    find_package(Qt5 5.7 REQUIRED COMPONENTS Core Gui Widgets)
    find_package(Fcitx5Qt5WidgetsAddons REQUIRED)
endif ()

# make default value header searchable
set(SKK_DICT_DEFAULT_PATH "/usr/share/skk/SKK-JISYO.L" CACHE STRING "Default path of SKK Dictionary file")
set(SKK_DICT_DEFAULT_ENCODING "euc-jp" CACHE STRING "Default encoding of SKK Dictionary file")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/skk_dict_config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/skk_dict_config.h"
        IMMEDIATE @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Googletest submodule setup
find_package(Git QUIET)
if (GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    enable_testing()
    option(GOOGLETEST "Check Google Test during build" OFF)
    # -DGOOGLETEST=on を付けて実行したらsubmoduleを最新版にする
    if (GOOGLETEST)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if (NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif ()
    endif ()
endif ()

if (NOT EXISTS "${PROJECT_SOURCE_DIR}/third_party/googletest/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GOOGLETEST was turned off or failed. Please update submodules (git submodules update --init --recurseive) and try again.")
endif ()


# Not sure what this does. Need translation?
find_package(Gettext REQUIRED)
add_definitions(-DFCITX_GETTEXT_DOMAIN=\"fcitx5-cskk\" -D_GNU_SOURCE)
fcitx5_add_i18n_definition()

add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(po)
add_subdirectory(test)
add_subdirectory(gui)


fcitx5_translate_desktop_file(
        org.fcitx.Fcitx5.Addon.Cskk.metainfo.xml.in
        org.fcitx.Fcitx5.Addon.Cskk.metainfo.xml XML)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/org.fcitx.Fcitx5.Addon.Cskk.metainfo.xml" DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
