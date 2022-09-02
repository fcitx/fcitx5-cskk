/*
 * SPDX-FileCopyrightText: 2013~2022 CSSlayer <wengxt@gmail.com>, Naoaki Iwakiri
 * <naokiri@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef ADDDICTDIALOG_H
#define ADDDICTDIALOG_H

#include "ui_adddictdialog.h"
#include <QDialog>
#include <QMap>
#include <memory>

namespace fcitx {

class AddDictDialog : public QDialog {
  Q_OBJECT
public:
  explicit AddDictDialog(QWidget *parent = 0);
  QMap<QString, QString> dictionary();
  void setDictionary(QMap<QString, QString> &dict);

public Q_SLOTS:
  void browseClicked();
  void indexChanged([[maybe_unused]] int);
  void validate();

private:
  std::unique_ptr<Ui::AddDictDialog> m_ui;
};

} // namespace fcitx

#endif // ADDDICTDIALOG_H
