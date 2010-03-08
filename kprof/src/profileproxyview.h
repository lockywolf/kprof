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

#ifndef PROFILEPROXYVIEW_H_
#define PROFILEPROXYVIEW_H_

#include <QTreeView>

class QMenu;
class QAction;
class QRegExp;

class QAbstractProxyModel;
class QAbstractItemModel;

class ProfileProxyView : public QTreeView {
  Q_OBJECT
  
  public:
    ProfileProxyView(QWidget *parent);
    
    void setProxyModel(QAbstractProxyModel *model);
    void setSourceModel(QAbstractItemModel *model);
    
    void setFilter(const QString &filter);
    
  signals:
    void showDetails(int index);
        
  private:
    QAbstractProxyModel * mModel;
    QMenu * mItemMenu;
    QMenu * mCallersMenu;
    QMenu * mCalleesMenu;
    QAction * mDetailsAction;
    
  private:
    bool filterLevel(const QModelIndex &parent, const QRegExp &filter);
    
  private slots:
    void onActivated(const QModelIndex &item);
    void onCustomContextMenuRequested(const QPoint &point);
    void onItemTriggered(QAction *action);
    void onDetailsTriggered();
};

#endif
