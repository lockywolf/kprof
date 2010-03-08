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

#include "hierarchicproxymodel.h"
#include "profilemodel.h"

template <typename Predicate>
class HierarchicProxyModel::NodeComparator {
  public:
    NodeComparator(QAbstractItemModel * m, int c)
        : model(m), column(c)  {}

    bool operator () (const HierarchyNode *lhs, const HierarchyNode *rhs) {
      QVariant l, r;
      l = model->data(model->index(lhs->row, column));
      r = model->data(model->index(rhs->row, column));

      return p(l, r);
    }

  private:
    QAbstractItemModel * model;
    int column;
    Predicate p;
};

HierarchicProxyModel::HierarchicProxyModel(QObject * parent)
  : QAbstractProxyModel(parent), mRoot(0)
{
}

QModelIndex HierarchicProxyModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!mRoot || column < 0 || column >= columnCount()) {
    return QModelIndex();
  }

  HierarchyNode * ptr = reinterpret_cast<HierarchyNode*>(parent.internalPointer());

  if (ptr) {
    if (parent.row() < 0 || parent.row() >= ptr->children.size()) {
      return QModelIndex();
    }

    ptr = ptr->children[parent.row()];
  } else {
    ptr = mRoot;
  }

  if (row < 0 || row >= ptr->children.size()) {
    return QModelIndex();
  }
  return createIndex(row, column, ptr);
}

QModelIndex HierarchicProxyModel::parent(const QModelIndex &child) const
{
  HierarchyNode * ptr = reinterpret_cast<HierarchyNode*>(child.internalPointer());

  if (ptr == 0 || ptr->parent == 0) {
    return QModelIndex();
  }

  for (int row = 0; row < ptr->parent->children.size(); ++row) {
    if (ptr->parent->children[row] == ptr) {
      return createIndex(row, 0, ptr->parent);
    }
  }
  return QModelIndex();
}

int HierarchicProxyModel::rowCount(const QModelIndex &parent) const
{
  if (!mRoot)
    return 0;

  if (parent == QModelIndex())
    return mRoot->children.size();

  HierarchyNode * ptr = reinterpret_cast<HierarchyNode*>(parent.internalPointer());

  if (parent.row() < 0 || parent.row() >= ptr->children.size()) {
    return 0;
  }

  ptr = ptr->children[parent.row()];

  return ptr->children.size();
}

int HierarchicProxyModel::columnCount(const QModelIndex &) const
{
  if (!sourceModel())
    return 0;

  return sourceModel()->columnCount();
}

QVariant HierarchicProxyModel::data(const QModelIndex &index, int role) const
{
  if (!mRoot || index == QModelIndex() || index.row() < 0)
    return QVariant();

  HierarchyNode * ptr = reinterpret_cast<HierarchyNode*>(index.internalPointer());

  if(ptr && index.row() < ptr->children.size()) {
    QModelIndex srcIndex = sourceModel()->index(ptr->children[index.row()]->row, index.column());
    return sourceModel()->data(srcIndex, role);
  }
  return QVariant();
}

QModelIndex HierarchicProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
  if (!mRoot || proxyIndex == QModelIndex() || proxyIndex.row() < 0)
    return QModelIndex();

  HierarchyNode * ptr = reinterpret_cast<HierarchyNode*>(proxyIndex.internalPointer());

  if(ptr && proxyIndex.row() < ptr->children.size()) {
    return sourceModel()->index(ptr->children[proxyIndex.row()]->row, proxyIndex.column());
  }
  return QModelIndex();
}

QModelIndex HierarchicProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
  if (!mRoot)
    return QModelIndex();

  for (int i = 0; i < mRoot->children.size(); ++i) {
    if (mRoot->children[i]->row == sourceIndex.row()) {
      return createIndex(i, sourceIndex.column(), mRoot);
    }
  }
  return QModelIndex();
}


void HierarchicProxyModel::sort(int column, Qt::SortOrder order)
{
  if(!mRoot) {
    return;
  }
  
  sortNode(mRoot, column, order);
  reset();
}

void HierarchicProxyModel::setSourceModel(QAbstractItemModel *model)
{
  if(sourceModel()) {
    disconnect(sourceModel(), SIGNAL(modelReset()), this, SLOT(updateNodes()));
  }
  
  QAbstractProxyModel::setSourceModel(model);
  
  if(sourceModel()) {
    connect(sourceModel(), SIGNAL(modelReset()), this, SLOT(updateNodes()));
  }
  
  updateNodes();
}

void HierarchicProxyModel::updateNodes()
{
  delete mRoot;
  mRoot = 0;

  if (!sourceModel()) {
    reset();
    return;
  }

  mRoot = new HierarchyNode();
  mRoot->row = -1;
  mRoot->parent = 0;

  for (int i = 0; i < sourceModel()->rowCount(); ++i) {
    addMethod(mRoot, i);
  }
  
  reset();
}

void HierarchicProxyModel::addMethod(HierarchyNode *parent, int row)
{
  HierarchyNode * nd = parent;
  while(nd) {
    // check for recursion
    if(nd->row == row) {
      return;
    }
    nd = nd->parent;
  }
  
  // create a new node
  nd = new HierarchyNode();

  nd->row = row;
  nd->parent = parent;

  QList<int> callees = sourceModel()->data(sourceModel()->index(row, 0),
                                           ProfileModel::CalleesRole).value<QList<int> >();
  foreach(int i, callees) {
    addMethod(nd, i);
  }
  
  parent->children.append(nd);
}

void HierarchicProxyModel::sortNode(HierarchyNode *node, int column, Qt::SortOrder order)
{
  foreach(HierarchyNode * nd, node->children) {
    sortNode(nd, column, order);
  }
  
  if (order == Qt::AscendingOrder)
    qSort(node->children.begin(), node->children.end(), NodeComparator<VariantLess>(sourceModel(), column));
  else
    qSort(node->children.begin(), node->children.end(), NodeComparator<VariantGreater>(sourceModel(), column));
}
