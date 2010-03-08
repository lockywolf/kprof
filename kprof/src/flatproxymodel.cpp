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

#include "flatproxymodel.h"
#include "profilemodel.h"

namespace {
  
  template <typename Predicate>
  class FlatComparator {
    public:
      FlatComparator(QAbstractItemModel * m, int c)
          : model(m), column(c)  {}

      bool operator () (int lhs, int rhs) {
        QVariant l, r;
        l = model->data(model->index(lhs, column));
        r = model->data(model->index(rhs, column));

        return p(l, r);
      }

    private:
      QAbstractItemModel * model;
      int column;
      Predicate p;
  };
  
}

/*
 * FlatProxyModel
 */
FlatProxyModel::FlatProxyModel(QObject * parent)
    : QAbstractProxyModel(parent)
{
}

QModelIndex FlatProxyModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!sourceModel() || parent != QModelIndex())
    return QModelIndex();

  if (row < 0 || row >= mIndices.size() || column < 0 || column >= columnCount())
    return QModelIndex();

  return createIndex(row, column);
}

QModelIndex FlatProxyModel::parent(const QModelIndex &) const
{
  return QModelIndex();
}

int FlatProxyModel::rowCount(const QModelIndex &parent) const
{
  if (!sourceModel() || parent != QModelIndex())
    return 0;

  return sourceModel()->rowCount();
}

int FlatProxyModel::columnCount(const QModelIndex &parent) const
{
  if (!sourceModel() || parent != QModelIndex())
    return 0;

  return sourceModel()->columnCount();
}

QModelIndex FlatProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
  if (!sourceModel() || proxyIndex == QModelIndex() ||
      proxyIndex.row() < 0 || proxyIndex.row() >= mIndices.size())
    return QModelIndex();

  return sourceModel()->index(mIndices[proxyIndex.row()], proxyIndex.column());
}

QModelIndex FlatProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
  if (!sourceModel() || sourceIndex.parent() != QModelIndex())
    return QModelIndex();

  // assuming columns are correct
  for (int i = 0; i < mIndices.size(); ++i) {
    if (mIndices[i] == sourceIndex.row())
      return createIndex(i, sourceIndex.column());
  }

  return QModelIndex();
}

void FlatProxyModel::sort(int column, Qt::SortOrder order)
{
  if (column == -1)
    return;

  if (order == Qt::AscendingOrder)
    qSort(mIndices.begin(), mIndices.end(), FlatComparator<VariantLess>(sourceModel(), column));
  else
    qSort(mIndices.begin(), mIndices.end(), FlatComparator<VariantGreater>(sourceModel(), column));

  reset();
}

void FlatProxyModel::setSourceModel(QAbstractItemModel *model)
{
  if(sourceModel()) {
    disconnect(sourceModel(), SIGNAL(modelReset()), this, SLOT(updateIndices()));
  }
  
  QAbstractProxyModel::setSourceModel(model);
  
  if(sourceModel()) {
    connect(sourceModel(), SIGNAL(modelReset()), this, SLOT(updateIndices()));
  }
  
  updateIndices();
}

void FlatProxyModel::updateIndices()
{
  // initialize permutation vector
  mIndices.resize(sourceModel() ? sourceModel()->rowCount() : 0);

  for (int i = 0; i < mIndices.size(); ++i)
    mIndices[i] = i;
  
  reset();
}
