/*
 * SPDX-FileCopyrightText: 2013~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "adddictdialog.h"
#include "skk_dict_config.h"
#include <QDebug>
#include <QFileDialog>
#include <fcitx-utils/standardpath.h>
#include <fcitxqti18nhelper.h>

#define FCITX_CONFIG_DIR "$FCITX_CONFIG_DIR"

namespace fcitx {

enum DictType { DictType_Static, DictType_User };

AddDictDialog::AddDictDialog(QWidget *parent)
    : QDialog(parent), m_ui(std::make_unique<Ui::AddDictDialog>()) {
  m_ui->setupUi(this);
  m_ui->typeComboBox->addItem(_("System"));
  m_ui->typeComboBox->addItem(_("User"));

  indexChanged(0);

  connect(m_ui->browseButton, &QPushButton::clicked, this,
          &AddDictDialog::browseClicked);
  connect(m_ui->typeComboBox,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &AddDictDialog::indexChanged);
  connect(m_ui->urlLineEdit, &QLineEdit::textChanged, this,
          &AddDictDialog::validate);
}

QMap<QString, QString> AddDictDialog::dictionary() {
  int idx = m_ui->typeComboBox->currentIndex();
  idx = idx < 0 ? 0 : idx;
  idx = idx > 1 ? 0 : idx;

  QMap<QString, QString> dict;
  const char *type[] = {"readonly", "readwrite"};

  dict["type"] = "file";
  dict["file"] = m_ui->urlLineEdit->text();
  dict["mode"] = type[idx];
  dict["encoding"] = m_ui->encodingEdit->text();

  return dict;
}

void AddDictDialog::indexChanged([[maybe_unused]] int _idx) {
  validate();
}

void AddDictDialog::validate() {
  const auto index = m_ui->typeComboBox->currentIndex();
  bool valid = true;
  switch (index) {
  case DictType_Static:
  case DictType_User:
    if (m_ui->urlLineEdit->text().isEmpty() || m_ui->encodingEdit->text().isEmpty()) {
      valid = false;
    }
    break;
  }
  m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

void AddDictDialog::browseClicked() {
  QString path = m_ui->urlLineEdit->text();
  if (m_ui->typeComboBox->currentIndex() == DictType_Static) {
    QString dir;
    if (path.isEmpty()) {
      path = SKK_DICT_DEFAULT_PATH;
    }
    QFileInfo info(path);
    path = QFileDialog::getOpenFileName(this, _("Select Dictionary File"),
                                        info.path());
  } else {
    auto fcitxBasePath = stringutils::joinPath(
        StandardPath::global().userDirectory(StandardPath::Type::PkgData),
        "cskk");
    fs::makePath(fcitxBasePath);
    QString fcitxConfigBasePath =
        QDir::cleanPath(QString::fromStdString(fcitxBasePath));

    if (path.isEmpty()) {
      auto baseDataPath = stringutils::joinPath(
          StandardPath::global().userDirectory(StandardPath::Type::Data), "cskk");
      fs::makePath(baseDataPath);
      QString basePath = QDir::cleanPath(QString::fromStdString(baseDataPath));
      path = basePath;
    } else if (path.startsWith(FCITX_CONFIG_DIR "/")) {

      QDir dir(fcitxConfigBasePath);
      path = dir.filePath(path.mid(strlen(FCITX_CONFIG_DIR) + 1));
    }
    path =
        QFileDialog::getOpenFileName(this, _("Select Dictionary File"), path);
    if (path.startsWith(fcitxConfigBasePath + "/")) {
      path = FCITX_CONFIG_DIR + path.mid(fcitxConfigBasePath.length(), -1);
    }
  }

  if (!path.isEmpty()) {
    m_ui->urlLineEdit->setText(path);
  }
}

} // namespace fcitx
