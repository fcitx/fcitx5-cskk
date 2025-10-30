/*
 * SPDX-FileCopyrightText: <text>2013~2022 CSSlayer <wengxt@gmail.com>, Naoaki
 * Iwakiri <naokiri@gmail.com></text>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "adddictdialog.h"
#include "dictmodel.h"
#include "skk_dict_config.h"
#include <QComboBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <fcitx-utils/standardpaths.h>
#include <fcitxqti18nhelper.h>

#define FCITX_CONFIG_DIR "$FCITX_CONFIG_DIR"

namespace fcitx {

const char *mode_type[] = {"readonly", "readwrite"};

enum DictType { DictType_Static, DictType_User };

AddDictDialog::AddDictDialog(QWidget *parent)
    : QDialog(parent), m_ui(std::make_unique<Ui::AddDictDialog>()) {
  m_ui->setupUi(this);
  m_ui->typeComboBox->addItem(_("System"));
  m_ui->typeComboBox->addItem(_("User"));
  m_ui->typeComboBox->setCurrentIndex(1);
  // m_ui->completeNoRadio

  indexChanged();

  connect(m_ui->browseButton, &QPushButton::clicked, this,
          &AddDictDialog::browseClicked);
  connect(m_ui->typeComboBox,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &AddDictDialog::indexChanged);
  connect(m_ui->urlLineEdit, &QLineEdit::textChanged, this,
          &AddDictDialog::validate);
  connect(m_ui->encodingEdit, &QLineEdit::textChanged, this,
          &AddDictDialog::validate);
}

QMap<QString, QString> AddDictDialog::dictionary() {
  int idx = m_ui->typeComboBox->currentIndex();
  idx = idx < 0 ? 0 : idx;
  idx = idx > 1 ? 0 : idx;

  QMap<QString, QString> dict;

  dict["type"] = "file";
  dict["file"] = m_ui->urlLineEdit->text();
  dict["mode"] = mode_type[idx];
  dict["encoding"] = m_ui->encodingEdit->text();
  if (m_ui->completeYesRadio->isChecked()) {
    dict["complete"] = "true";
  } else {
    dict["complete"] = "false";
  }

  return dict;
}

void AddDictDialog::indexChanged() { validate(); }

void AddDictDialog::validate() {
  const auto index = m_ui->typeComboBox->currentIndex();
  bool valid = true;
  switch (index) {
  case DictType_Static:
  case DictType_User:
    if (m_ui->urlLineEdit->text().isEmpty() ||
        m_ui->encodingEdit->text().isEmpty()) {
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
    auto fcitxBasePath =
        StandardPaths::global().userDirectory(StandardPathsType::PkgData) /
        "cskk";
    fs::makePath(fcitxBasePath);
    QString fcitxConfigBasePath =
        QDir::cleanPath(QString::fromStdString(fcitxBasePath));

    if (path.isEmpty()) {
      auto baseDataPath =
          StandardPaths::global().userDirectory(StandardPathsType::Data) /
          "fcitx5-cskk";
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
  validate();
}

void AddDictDialog::setDictionary(QMap<QString, QString> &dict) {
  m_ui->urlLineEdit->setText(dict["file"]);

  for (size_t i = 0; i < FCITX_ARRAY_SIZE(mode_type); i++) {
    // It's OK to use stdstirng here. We only use latin-1.
    if (dict["mode"].toStdString() == mode_type[i]) {
      m_ui->typeComboBox->setCurrentIndex(i);
    }
  }
  m_ui->encodingEdit->setText(dict["encoding"]);
  if (dict["complete"] == "complete") {
    m_ui->completeYesRadio->setChecked(true);
  }
}

} // namespace fcitx
