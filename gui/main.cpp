/*
 * SPDX-FileCopyrightText: 2013~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "main.h"
extern "C" {
#include <libcskk.h>
}
#include "dictwidget.h"
#include <fcitx-utils/i18n.h>
#include <qplugin.h>

namespace fcitx {

CskkConfigPlugin::CskkConfigPlugin(QObject *parent)
    : FcitxQtConfigUIPlugin(parent) {

  registerDomain("fcitx5-cskk", FCITX_INSTALL_LOCALEDIR);
}

FcitxQtConfigUIWidget *CskkConfigPlugin::create(const QString &key) {
  if (key == "dictionary_list") {
    return new SkkDictWidget;
  }
  return nullptr;
}

} // namespace fcitx
