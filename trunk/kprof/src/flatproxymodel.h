/*
 *  Copyright (c) 2010 Martin Moracek <mori_bundus@sourceforge.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef FLATPROXYMODEL_H_
#define FLATPROXYMODEL_H_

#include <QAbstractProxyModel>
#include <QVector>

class FlatProxyModel : public QAbstractProxyModel {
  Q_OBJECT

  public:
    FlatProxyModel(QObject *parent = 0);

    QModelIndex index(int row, int column,
      const QModelIndex &parent = QModelIndex()) const;

    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    void setSourceModel(QAbstractItemModel *model);

  private:
    QVector<int> mIndices;
    
  private slots:
    void updateIndices();
};

#endif
