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

#include <QStringList>

#include "objectproxymodel.h"
#include "profilemodel.h"

namespace {
  
  template <typename Predicate>
  class ObjectComparator {
    public:
      ObjectComparator(QAbstractItemModel * m, int c)
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

ObjectProxyModel::ObjectNode::~ObjectNode()
{
  for (int i = 0; i < children.size(); ++i) {
    delete children[i];
  }
}

ObjectProxyModel::ObjectProxyModel(QObject *parent)
    : QAbstractProxyModel(parent), mRoot(0)
{
}

ObjectProxyModel::~ObjectProxyModel()
{
  delete mRoot;
}

QModelIndex ObjectProxyModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!mRoot || column < 0 || column >= columnCount()) {
    return QModelIndex();
  }

  ObjectNode * ptr = reinterpret_cast<ObjectNode*>(parent.internalPointer());

  if (ptr) {
    if (parent.row() < 0 || parent.row() >= ptr->children.size()) {
      return QModelIndex();
    }

    ptr = ptr->children[parent.row()];
  } else {
    ptr = mRoot;
  }

  if (row < 0 || row >= (ptr->children.size() + ptr->indices.size())) {
    return QModelIndex();
  }
  return createIndex(row, column, ptr);
}

QModelIndex ObjectProxyModel::parent(const QModelIndex &child) const
{
  ObjectNode * ptr = reinterpret_cast<ObjectNode*>(child.internalPointer());

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

int ObjectProxyModel::rowCount(const QModelIndex &parent) const
{
  if (!mRoot)
    return 0;

  if (parent == QModelIndex())
    return mRoot->children.size() + mRoot->indices.size();

  ObjectNode * ptr = reinterpret_cast<ObjectNode*>(parent.internalPointer());

  if (parent.row() < 0 || parent.row() >= ptr->children.size()) {
    return 0;
  }

  ptr = ptr->children[parent.row()];

  return ptr->children.size() + ptr->indices.size();
}

int ObjectProxyModel::columnCount(const QModelIndex &) const
{
  if (!sourceModel())
    return 0;

  return sourceModel()->columnCount();
}

QVariant ObjectProxyModel::headerData
(int section, Qt::Orientation orientation, int role) const
{
  if (sourceModel()) {
    return sourceModel()->headerData(section, orientation, role);
  }
  return QVariant();
}

QVariant ObjectProxyModel::data(const QModelIndex &index, int role) const
{
  if (!mRoot || index == QModelIndex() || index.row() < 0)
    return QVariant();

  ObjectNode * ptr = reinterpret_cast<ObjectNode*>(index.internalPointer());

  // node
  if (index.row() < ptr->children.size()) {
    if (index.column() == 0) {
      ptr = ptr->children[index.row()];
      if (ptr->row != -1) {
        if (role == Qt::ToolTipRole) {
          role = ProfileModel::FullNamespaceRole;
        } else if (role == Qt::DisplayRole) {
          role = ProfileModel::NamespaceRole;
        } else {
          return QVariant();
        }
        return sourceModel()->data(sourceModel()->index(ptr->row, 0), role).toStringList().at(ptr->position);
      }
    }
  // leaf
  } else {
    int row = ptr->indices[index.row() - ptr->children.size()];
    QModelIndex srcIndex = sourceModel()->index(row, index.column());

    // exception for shortened name
    if (role == Qt::DisplayRole && index.column() == 0) {
      return sourceModel()->data(srcIndex, ProfileModel::MethodRole);
    } else {
      return sourceModel()->data(srcIndex, role);
    }
  }
  return QVariant();
}

Qt::ItemFlags ObjectProxyModel::flags(const QModelIndex &index) const
{
  if (!mRoot || index == QModelIndex() || index.row() < 0)
    return 0;

  ObjectNode * ptr = reinterpret_cast<ObjectNode*>(index.internalPointer());

  // node
  if (ptr && index.row() >= 0 && index.row() < ptr->children.size() + ptr->indices.size()) {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }
  return 0;
}

QModelIndex ObjectProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
  if (!mRoot || proxyIndex == QModelIndex())
    return QModelIndex();
  
  ObjectNode * ptr = reinterpret_cast<ObjectNode*>(proxyIndex.internalPointer());
  
  if (!ptr) {
    return QModelIndex();
  }
  
  int idx = proxyIndex.row() - ptr->children.size();
  
  if (idx >= 0 && idx < ptr->indices.size()) {
    return sourceModel()->index(ptr->indices[idx], proxyIndex.column());
  }
  return QModelIndex();
}

QModelIndex ObjectProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
  if (!mRoot) {
    return QModelIndex();
  }
  return findSourceIndex(mRoot, sourceIndex);
}

void ObjectProxyModel::sort(int column, Qt::SortOrder order)
{
  if(!mRoot) {
    return;
  }
  
  sortNode(mRoot, column, order);
  reset();
}

void ObjectProxyModel::setSourceModel(QAbstractItemModel *model)
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

void ObjectProxyModel::updateNodes(void )
{
  delete mRoot;
  mRoot = 0;

  if (!sourceModel()) {
    reset();
    return;
  }

  mRoot = new ObjectNode();
  mRoot->row = -1;
  mRoot->position = -1;
  mRoot->parent = 0;

  for (int i = 0; i < sourceModel()->rowCount(); ++i) {
    QString name = sourceModel()->data(sourceModel()->index(i, 0),
                                       ProfileModel::FullNameRole).toString();
    QStringList nsp = sourceModel()->data(sourceModel()->index(i, 0),
                                          ProfileModel::FullNamespaceRole).toStringList();
    
    addMethod(mRoot, nsp, i, 0);
  }
  
  reset();
}

// recursively add given method
void ObjectProxyModel::addMethod
(ObjectNode *node, const QStringList &prefix, int row, int pos)
{
  Q_ASSERT(node);

  if (prefix.isEmpty()) {
    node->indices.append(row);
  } else {
    for (int i = 0; i < node->children.size(); ++i) {
      if (node->children[i]->name == prefix.first()) {
        addMethod(node->children[i], prefix.mid(1), row, pos + 1);
        return;
      }
    }

    // create a new node
    ObjectNode * nd = new ObjectNode();

    nd->name = prefix.first();
    nd->row = row;
    nd->position = pos;
    nd->parent = node;

    node->children.append(nd);

    addMethod(nd, prefix.mid(1), row, pos + 1);
  }
}

QModelIndex ObjectProxyModel::findSourceIndex(ObjectNode * node, const QModelIndex & index) const
{
  Q_ASSERT(node);
  
  int offset = node->indices.indexOf(index.row());
  if (offset != -1) {
    return createIndex(node->children.size() + offset, index.column(), node);
  } else {
    foreach (ObjectNode * nd, node->children) {
      QModelIndex res = findSourceIndex(nd, index);
      
      if (res.isValid()) {
        return res;
      }
    }
  }
  return QModelIndex();
}

void ObjectProxyModel::sortNode(ObjectNode *node, int column, Qt::SortOrder order)
{ 
  foreach(ObjectNode * nd, node->children) {
    sortNode(nd, column, order);
  }

  if (order == Qt::AscendingOrder)
    qSort(node->indices.begin(), node->indices.end(), ObjectComparator<VariantLess>(sourceModel(), column));
  else
    qSort(node->indices.begin(), node->indices.end(), ObjectComparator<VariantGreater>(sourceModel(), column));
}
