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

// TODO: trr
#include <QtDebug>

#include <QMenu>
#include <QAbstractProxyModel>

#include <KLocale>
#include <KAction>

#include "profileproxyview.h"
#include "profilemodel.h"

ProfileProxyView::ProfileProxyView(QWidget *parent)
  : QTreeView(parent), mModel(0)
{
  setSortingEnabled(true);
  sortByColumn(-1, Qt::AscendingOrder);
 
  setContextMenuPolicy(Qt::CustomContextMenu);

  mItemMenu = new QMenu(this);
  mCallersMenu = new QMenu(i18n("Called by"), this);
  mCalleesMenu = new QMenu(i18n("Calls"), this);
  mDetailsAction = new KAction(i18n("Show &Details"), this);
  
  mItemMenu->addMenu(mCallersMenu);
  mItemMenu->addMenu(mCalleesMenu);
  
  KAction * action = new KAction("", this);
  action->setSeparator(true);
  
  mItemMenu->addAction(action);
  mItemMenu->addAction(mDetailsAction);
  
  connect(mCallersMenu, SIGNAL(triggered(QAction*)), SLOT(onItemTriggered(QAction*)));
  connect(mCalleesMenu, SIGNAL(triggered(QAction*)), SLOT(onItemTriggered(QAction*)));
  connect(mDetailsAction, SIGNAL(triggered(bool)), SLOT(onDetailsTriggered()));
  
  connect(this, SIGNAL(activated(QModelIndex)), SLOT(onActivated(QModelIndex)));
  connect(this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(onCustomContextMenuRequested(QPoint)));

//   connect (mFlat, SIGNAL (selectionChanged (QTreeWidgetItem *)),
//         this, SLOT (selectionChanged (QTreeWidgetItem *)));
}

void ProfileProxyView::setProxyModel(QAbstractProxyModel *model)
{
  if (mModel) {
    delete mModel;
  }
  
  mModel = model;
  setModel(model);
  
  if (mModel) {
    mModel->setParent(this);
  }
}

void ProfileProxyView::setSourceModel(QAbstractItemModel *model)
{
  if (mModel) {
    mModel->setSourceModel(model);
  }
}

void ProfileProxyView::setFilter(const QString &filter)
{
  if (mModel) {
    filterLevel(QModelIndex(), QRegExp(filter));
  }
}

bool ProfileProxyView::filterLevel(const QModelIndex &parent, const QRegExp &filter)
{
  bool allHidden = true;
  for (int i = 0; i < mModel->rowCount(parent); ++i) {
    QModelIndex index = mModel->index(i, 0, parent);

    // is it meta-item?
    if (mModel->mapToSource(index) == QModelIndex()) {
      bool hide = filterLevel(index, filter);
      
      setRowHidden(i, parent, hide);
      allHidden = allHidden && hide;
    } else {
      if (mModel->data(index, ProfileModel::FullNameRole).toString().contains(filter)) {
        setRowHidden(i, parent, false);
        allHidden = false;
      } else {
        bool hide = filterLevel(index, filter);
        
        setRowHidden(i, parent, hide);
        allHidden = allHidden && hide;
      }
    }
  }
  return allHidden;
}

void ProfileProxyView::onActivated(const QModelIndex &item)
{
  Q_ASSERT(mModel);
  
  emit showDetails(mModel->mapToSource(item).row());
}

void ProfileProxyView::onCustomContextMenuRequested(const QPoint &point)
{
  QModelIndex index = indexAt(point);
  
  if (index != QModelIndex()) {
    mCallersMenu->clear();
    mCalleesMenu->clear();
    
    const QAbstractItemModel * srcModel = mModel->sourceModel();
    QModelIndex srcIndex = mModel->mapToSource(index);
    
    if (srcIndex == QModelIndex())
      return;
    
    QList<int> list;
    list = srcModel->data(srcModel->index(srcIndex.row(), 0), ProfileModel::CallersRole).value<QList<int> >();
    foreach(int idx, list) {
      KAction * act = new KAction(srcModel->data(srcModel->index(idx, 0), ProfileModel::NameRole).toString(), mCallersMenu);
      act->setData(idx);
      mCallersMenu->addAction(act);
    }
    
    list = srcModel->data(srcModel->index(srcIndex.row(), 0), ProfileModel::CalleesRole).value<QList<int> >();
    foreach(int idx, list) {
      KAction * act = new KAction(srcModel->data(srcModel->index(idx, 0), ProfileModel::NameRole).toString(), mCalleesMenu);
      act->setData(idx);
      mCalleesMenu->addAction(act);
    }
    
    mCallersMenu->setEnabled(!mCallersMenu->actions().isEmpty());
    mCalleesMenu->setEnabled(!mCalleesMenu->actions().isEmpty());
    
    mDetailsAction->setData(srcIndex.row());
    
    mItemMenu->popup(mapToGlobal(point));
  }
}

void ProfileProxyView::onItemTriggered(QAction *action)
{
  if (mModel) {
    int row = action->data().toInt();
    const QAbstractItemModel * srcModel = mModel->sourceModel();
    
    if (!srcModel) {
      return;
    }
    
    QModelIndex index = mModel->mapFromSource(srcModel->index(row, 0));
    
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
  }
}

void ProfileProxyView::onDetailsTriggered()
{
  emit showDetails(mDetailsAction->data().toInt());
}
