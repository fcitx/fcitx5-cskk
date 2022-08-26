/*
 * SPDX-FileCopyrightText: 2013~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef FCITX_SKK_GUI_MAIN_H_
#define FCITX_SKK_GUI_MAIN_H_

#include <fcitxqtconfiguiplugin.h>

namespace fcitx {

class CSkkConfigPlugin : public FcitxQtConfigUIPlugin {
    Q_OBJECT
public:
    Q_PLUGIN_METADATA(IID FcitxQtConfigUIFactoryInterface_iid FILE
                      "cskk-config.json")
    explicit CSkkConfigPlugin(QObject *parent = 0);
    FcitxQtConfigUIWidget *create(const QString &key) override;
};

} // namespace fcitx

#endif // FCITX_TOOLS_GUI_MAIN_H_
