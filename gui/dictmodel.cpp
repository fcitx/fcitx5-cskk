/*
 * SPDX-FileCopyrightText: <text>2013~2022 CSSlayer <wengxt@gmail.com>, Naoaki
 * Iwakiri <naokiri@gmail.com></text>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "dictmodel.h"
#include <QDebug>
#include <QFile>
#include <QStringList>
#include <QTemporaryFile>
#include <QtGlobal>
#include <fcitx-utils/standardpaths.h>
#include <fcntl.h>
#include <iostream>

namespace fcitx {

const std::string config_path = "cskk/dictionary_list";

SkkDictModel::SkkDictModel(QObject *parent) : QAbstractListModel(parent) {}

void SkkDictModel::defaults() {
  auto path = StandardPaths::fcitxPath("pkgdatadir", config_path.c_str());
  QFile f(path);
  if (f.open(QIODevice::ReadOnly)) {
    load(f);
  }
}

void SkkDictModel::load() {
  auto file =
      StandardPaths::global().open(StandardPathsType::PkgData, config_path);
  if (file.fd() < 0) {
    return;
  }
  QFile f;
  if (!f.open(file.fd(), QIODevice::ReadOnly)) {
    return;
  }

  load(f);
  f.close();
}

void SkkDictModel::load(QFile &file) {
  beginResetModel();
  m_dicts.clear();

  QByteArray bytes;
  while (!(bytes = file.readLine()).isEmpty()) {
    QString line = QString::fromUtf8(bytes).trimmed();
    QMap<QString, QString> dict = parseLine(line);
    if (3 <= dict.size()) {
      m_dicts << dict;
    }
  }
  endResetModel();
}

bool SkkDictModel::save() {
  return StandardPaths::global().safeSave(
      StandardPathsType::PkgData, config_path, [this](int fd) {
        QFile tempFile;
        if (!tempFile.open(fd, QIODevice::WriteOnly)) {
          return false;
        }

        typedef QMap<QString, QString> DictType;

        Q_FOREACH (const DictType &dict, m_dicts) {
          QString line = serialize(dict);
          tempFile.write(line.toUtf8());
          tempFile.write("\n");
        }
        return true;
      });
}

int SkkDictModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return m_dicts.size();
}

bool SkkDictModel::removeRows(int row, int count, const QModelIndex &parent) {
  if (parent.isValid()) {
    return false;
  }

  if (count == 0 || row >= m_dicts.size() || row + count > m_dicts.size()) {
    return false;
  }

  beginRemoveRows(parent, row, row + count - 1);
  m_dicts.erase(m_dicts.begin() + row, m_dicts.begin() + row + count);
  endRemoveRows();

  return true;
}

QVariant SkkDictModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  if (index.row() >= m_dicts.size() || index.column() != 0) {
    return QVariant();
  }

  switch (role) {
  case Qt::DisplayRole:
    return m_dicts[index.row()]["file"];
  case Qt::EditRole:
    return serialize(m_dicts[index.row()]);
  }
  return QVariant();
}

bool SkkDictModel::setData(const QModelIndex &index, const QVariant &value,
                           int role) {
  if (role == Qt::EditRole) {
    QString dictString = value.toString();
    QMap<QString, QString> dict = parseLine(dictString);
    m_dicts[index.row()] = dict;
    dataChanged(index, index, {role});
    return true;
  }
  return false;
}

bool SkkDictModel::moveUp(const QModelIndex &currentIndex) {
  if (currentIndex.row() > 0 && currentIndex.row() < m_dicts.size()) {
    beginResetModel();
#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
    m_dicts.swap(currentIndex.row() - 1, currentIndex.row());
#else
    m_dicts.swapItemsAt(currentIndex.row() - 1, currentIndex.row());
#endif
    endResetModel();
    return true;
  }
  return false;
}

bool SkkDictModel::moveDown(const QModelIndex &currentIndex) {
  if (currentIndex.row() >= 0 && currentIndex.row() + 1 < m_dicts.size()) {
    beginResetModel();
#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
    m_dicts.swap(currentIndex.row() + 1, currentIndex.row());
#else
    m_dicts.swapItemsAt(currentIndex.row() + 1, currentIndex.row());
#endif
    endResetModel();
    return true;
  }

  return false;
}

void SkkDictModel::add(const QMap<QString, QString> &dict) {
  beginInsertRows(QModelIndex(), m_dicts.size(), m_dicts.size());
  m_dicts << dict;
  endInsertRows();
}

QMap<QString, QString> SkkDictModel::parseLine(const QString &line) {
  QStringList items = line.split(",");
  // No matter which type, it should has at least 3 keys.
  if (items.size() < 3) {
    return {};
  }

  QMap<QString, QString> dict;
  Q_FOREACH (const QString &item, items) {
    if (!item.contains('=')) {
      return {};
    }
    QString key = item.section('=', 0, 0);
    QString value = item.section('=', 1, -1);

    if (!SkkDictModel::m_knownKeys.contains(key)) {
      continue;
    }

    dict[key] = value;
  }

  // Inheritance from fcitx5-skk, encoding was optional.
  if (!dict.keys().contains("encoding")) {
    dict["encoding"] = "euc-jp";
  }

  if (dict.keys().contains("file") && dict.keys().contains("type") &&
      dict.keys().contains("mode") && dict.keys().contains("encoding")) {
    return dict;
  }
  return {};
}

// Serialize to QString of ',' separated k=v pair.
// This k=v pair string is to match the configuration reading function of old
// fcitx, and to be used as QVariant.
QString SkkDictModel::serialize(const QMap<QString, QString> &dict) {
  bool first = true;
  QString result = "";
  Q_FOREACH (const QString &key, dict.keys()) {
    if (first) {
      first = false;
    } else {
      result.append(",");
    }
    result.append(key.toUtf8());
    result.append("=");
    result.append(dict[key].toUtf8());
  }

  return result;
}

} // namespace fcitx
