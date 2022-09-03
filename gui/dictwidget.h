/*
 * SPDX-FileCopyrightText: <text>2013~2022 CSSlayer <wengxt@gmail.com>, Naoaki Iwakiri
 * <naokiri@gmail.com></text>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef FCITX_SKK_GUI_DICTWIDGET_H
#define FCITX_SKK_GUI_DICTWIDGET_H

#include "ui_dictwidget.h"
#include <fcitxqtconfiguiwidget.h>
#include <memory>

namespace fcitx {

class SkkDictModel;

class SkkDictWidget : public FcitxQtConfigUIWidget {
  Q_OBJECT
public:
  explicit SkkDictWidget(QWidget *parent = 0);

  void load() override;
  void save() override;
  QString title() override;
  QString icon() override;

private Q_SLOTS:
  void addDictClicked();
  void defaultDictClicked();
  void removeDictClicked();
  void moveUpDictClicked();
  void moveDownClicked();
  void editDictClicked();

private:
  std::unique_ptr<Ui::SkkDictWidget> m_ui;
  SkkDictModel *m_dictModel;
};

} // namespace fcitx

#endif // FCITX_SKK_GUI_DICTWIDGET_H
