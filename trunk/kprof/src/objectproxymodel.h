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

#ifndef OBJECTPROXYMODEL_H_
#define OBJECTPROXYMODEL_H_

#include <QVector>
#include <QAbstractProxyModel>

class ObjectProxyModel : public QAbstractProxyModel {
  Q_OBJECT

  public:
    ObjectProxyModel(QObject *parent = 0);
    ~ObjectProxyModel();

    QModelIndex index(int row, int column,
      const QModelIndex &parent = QModelIndex()) const;

    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
    
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    void setSourceModel(QAbstractItemModel *model);
    
  private:
    struct ObjectNode {
      QString name;  // full name, for internal use only
      int row;
      int position;  // namespace position

      // store pointers in case we need persistent model indices in the future
      QVector<ObjectNode*> children;
      ObjectNode * parent;

      QVector<int> indices;

      ~ObjectNode();
    };

  private:
    ObjectNode * mRoot;

  private slots:
    void updateNodes(void);

  private:
    void addMethod(ObjectNode *parent, const QStringList &prefix, int row, int pos);
    QModelIndex findSourceIndex(ObjectNode * node, const QModelIndex & index) const;
    void sortNode(ObjectNode *node, int column, Qt::SortOrder order);
};

#endif
