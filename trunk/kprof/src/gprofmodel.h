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

#ifndef GPROFMODEL_H_
#define GPROFMODEL_H_

#include <QVector>
#include <QStringList>

#include "profilemodel.h"

class QTextStream;

class GProfModel : public ProfileModel
{
    Q_OBJECT

  public:
    enum DisplaySection {
      Name = 0,
      SelfPercentage,
      TotalPercentage,
      SelfTime,
      TotalTime,
      ChildrenTime,
      Calls,
      SelfPerCall,
      TimePerCall
    };

    struct ProfileInfo {
      QString name;             ///< full function/method prototype
      QStringList object;       ///< namespace specification
      QString method;           ///< name - object
      QList<int> callees;       ///< list of functions that this one calls
      QList<int> callers;       ///< list of functions calling this one

      QVariant selfPercentage;  ///< percentage of time spent in this function alone
      QVariant totalPercentage; ///< percentage of time spent in this function and it's children
      QVariant selfSeconds;     ///< total amount of time spent in this function alone
      QVariant totalSeconds;    ///< total amount of time spent in this function and it's children
      QVariant childrenSeconds; ///< total amount of time propagated to this function by it's children
      QVariant calls;           ///< number of times this function was called
      QVariant selfMsPerCall;   ///< function's own CPU usage PER CALL (average)
      QVariant totalMsPerCall;  ///< function's total CPU usage (incl. children) (average)
      bool recursive;

      // splits name into object, method and arguments
      void splitName(void);
      void addCaller(int caller);
      void addCallee(int callee);
    };

  public:
    GProfModel(QObject * parent);

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool loadFile(QTextStream &stream);

    bool shortNames() const {return mShort;}
    void setShortNames(bool sh);
    
  private:
    QVector<ProfileInfo> mProfile;
    bool mShort;
    
  private:
    int findFunction(const QString & name);
    QString prepareHtml(int index) const;
};

#endif
