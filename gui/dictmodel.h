/*
 * SPDX-FileCopyrightText: 2013~2022 CSSlayer <wengxt@gmail.com>, Naoaki Iwakiri
 * <naokiri@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef DICTMODEL_H
#define DICTMODEL_H
#include <QAbstractItemModel>
#include <QFile>
#include <QSet>

namespace fcitx {

class DictModel {};

class SkkDictModel : public QAbstractListModel {
  Q_OBJECT
public:
  explicit SkkDictModel(QObject *parent = 0);
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole) override;

  bool removeRows(int row, int count,
                  const QModelIndex &parent = QModelIndex()) override;

  void load();
  void load(QFile &file);
  void defaults();
  bool save();
  void add(const QMap<QString, QString> &dict);
  bool moveDown(const QModelIndex &currentIndex);
  bool moveUp(const QModelIndex &currentIndex);

  static QMap<QString, QString> parseLine(const QString &line);
  static QString serialize(const QMap<QString, QString> &dict);
  static QSet<QString> m_knownKeys;

private:
  QList<QMap<QString, QString>> m_dicts;
};

QSet<QString> SkkDictModel::m_knownKeys =
    QSet<QString>({"file", "type", "mode", "encoding"});

} // namespace fcitx

#endif // DICTMODEL_H
