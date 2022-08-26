set(CSKK_CONFIG_SRCS
  main.cpp
  dictwidget.cpp
  adddictdialog.cpp
  dictmodel.cpp
  )

if(NOT ENABLE_QT)
  return()
endif()

add_library(fcitx5-cskk-config
  MODULE ${CSKK_CONFIG_SRCS})

set_target_properties(fcitx5-cskk-config PROPERTIES
                      AUTOMOC TRUE
                      AUTOUIC TRUE
                      AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h"
)
target_link_libraries(fcitx5-cskk-config
  Qt5::Core
  Qt5::Widgets
  Fcitx5Qt5::WidgetsAddons
  Fcitx5::Utils
  ${LIBCSKK_LIBRARIES}
  )

install(TARGETS fcitx5-cskk-config DESTINATION ${CMAKE_INSTALL_LIBDIR}/fcitx5/qt5)
