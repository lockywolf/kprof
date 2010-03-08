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

#ifndef DIFFMODEL_H_
#define DIFFMODEL_H_

#include <QVector>
#include <QHash>

#include "profilemodel.h"

class DiffModel : public ProfileModel
{
  Q_OBJECT

  public:
    enum DisplayMode {
      OnlyA,
      OnlyB,
      ASubB,
      BSubA,
      ADivB,
      BDivA
    };

  public:
    DiffModel(QObject *parent);
    
    DisplayMode mode() const {return mMode;}
    void setMode(DisplayMode mode);
    
    bool shortNames() const;
    void setShortNames(bool sh);
    
    bool showSingular() const {return mSingular;}
    void setShowSingular(bool show);
    
    void setModelA(ProfileModel *model);
    void setModelB(ProfileModel *model);
    void setModels(ProfileModel *modelA, ProfileModel *modelB);
    
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &) const {return QModelIndex();}

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    
    int findItem(const QString &name);
    
    QString htmlProfile(int item) const;
    
  private:
    struct DiffEntry {
      int rowA;
      int rowB;
      
      DiffEntry() : rowA(-1), rowB(-1) {}
    };
    
  private:
    ProfileModel * mModelA;
    ProfileModel * mModelB;
    
    DisplayMode mMode;
    bool mSingular;
    
    QVector<DiffEntry> mDiffData;
    QVector<int> mIndices;
    QHash<int, int> mMapA; // maps indices from A to Diff
    QHash<int, int> mMapB; // maps indices from B to Diff
    
  private:
    QString dumpCallTable(const QList<int> &items, bool &note) const;
    
    void updateModel();
    void updateIndices();
};

#endif
