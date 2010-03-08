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

// TODO: remove
#include <QtDebug>

#include <QRegExp>
#include <QTextStream>
#include <QBrush>
#include <QSet>

#include "diffmodel.h"
#include "profilemodel.h"

namespace {
  
  const QVariant subtract(const QVariant & lhs, const QVariant & rhs)
  {
    if (lhs.type() != rhs.type()) {
      return QVariant();
    }
    
    switch(lhs.type()) {
    case QVariant::Double:
      qDebug() << lhs << "-" << rhs << "=" << lhs.toDouble() - rhs.toDouble();
      return lhs.toDouble() - rhs.toDouble();
    case QVariant::Int:
      return lhs.toInt() - rhs.toInt();
    case QVariant::LongLong:
      return lhs.toLongLong() - rhs.toLongLong();
    case QVariant::UInt:
      return lhs.toUInt() - rhs.toUInt();
    case QVariant::ULongLong:
      return lhs.toULongLong() - rhs.toULongLong();
    default:
      return QVariant();
    }
  }
  
  const QVariant divide(const QVariant & lhs, const QVariant & rhs)
  {
    if (lhs.type() != rhs.type()) {
      return QVariant();
    }
    
    if(lhs.type() != QVariant::Double &&
       lhs.type() != QVariant::Int && 
       lhs.type() != QVariant::LongLong && 
       lhs.type() != QVariant::UInt && 
       lhs.type() != QVariant::ULongLong) {
      return QVariant();
    }
    
    double l = lhs.toDouble(), r = rhs.toDouble();
    
    if(r == 0.0) {
      return QVariant();
    } else {
      return l / r;
    }
  }
  
}

DiffModel::DiffModel(QObject *parent) : ProfileModel(parent),
  mModelA(0), mModelB(0), mMode(OnlyA), mSingular(true)
{
}

void DiffModel::setMode(DisplayMode mode)
{
  DisplayMode newMode;
  
  if(!mModelB) {
    newMode = OnlyA;
    return;
  }
  
  if(!mModelA) {
    newMode = OnlyB;
    return;
  }
  
  newMode = mode;
  
  if(newMode != mMode) {
    mMode = newMode;
    
    updateIndices();
    reset();
  }
}

void DiffModel::setShowSingular(bool show)
{
  if(show != mSingular) {
    mSingular = show;

    updateIndices();
    reset();
  }
}

bool DiffModel::shortNames() const
{
  if (mModelA) {
    return mModelA->shortNames();
  } else if (mModelB) {
    return mModelB->shortNames();
  }
  return false;
}

void DiffModel::setShortNames(bool sh)
{
  if (mModelA) {
    mModelA->setShortNames(sh);
  }
  if (mModelB) {
    mModelB->setShortNames(sh);
  }
}

void DiffModel::setModelA(ProfileModel *model)
{
  if(model && mModelB && qstrcmp(mModelB->metaObject()->className(),
     model->metaObject()->className())) {
    qDebug("DiffModel::setModelA failed: Incompatible models (%s and %s)",
           model->metaObject()->className(), mModelB->metaObject()->className());
    return;
  }
  
  if (model) {
    model->setShortNames(shortNames());
  }
  
  mModelA = model;
 
  updateModel();
}

void DiffModel::setModelB(ProfileModel *model)
{
  if(model && mModelA && qstrcmp(mModelA->metaObject()->className(),
     model->metaObject()->className())) {
    qDebug("DiffModel::setModelB failed: Incompatible models (%s and %s)",
           mModelA->metaObject()->className(), model->metaObject()->className());
    return;
  }
  
  if (model) {
    model->setShortNames(shortNames());
  }
  
  mModelB = model;
  
  updateModel();
}

void DiffModel::setModels(ProfileModel *modelA, ProfileModel *modelB)
{
  if(modelA && modelB && qstrcmp(modelA->metaObject()->className(),
     modelB->metaObject()->className())) {
    qDebug("DiffModel::setModels failed: Incompatible models (%s and %s)",
           modelA->metaObject()->className(), modelB->metaObject()->className());
    return;
  }
  
  mModelA = modelA;
  mModelB = modelB;
  
  updateModel();
}

QModelIndex DiffModel::index
(int row, int column, const QModelIndex &parent) const
{
  if (parent != QModelIndex()) {
    return QModelIndex();
  }

  if (row < 0 || row >= mIndices.size() || column < 0 || column >= columnCount()) {
    return QModelIndex();
  }

  return createIndex(row, column);
}

int DiffModel::rowCount(const QModelIndex &) const
{
  return mIndices.size();
}

int DiffModel::columnCount(const QModelIndex &) const
{
  if(mModelA) {
    return mModelA->columnCount();
  } else if(mModelB) {
    return mModelB->columnCount();
  } else {
    return 0;
  }
}

QVariant DiffModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(mModelA) {
    return mModelA->headerData(section, orientation, role);
  } else if(mModelB) {
    return mModelB->headerData(section, orientation, role);
  } else {
    return 0;
  }
}

QVariant DiffModel::data(const QModelIndex &index, int role) const
{
  if (!mModelA && !mModelB) {
    return QVariant();
  }
  
  if (index.parent() != QModelIndex()) {
    return QVariant();
  }
  
  if (index.row() < 0 || index.row() >= mIndices.size()) {
    return QVariant();
  }
  
  QVariant res;
  const DiffEntry & entry = mDiffData[mIndices[index.row()]];
  
  QModelIndex indexA, indexB;
  if(mModelA) {
    indexA = mModelA->index(entry.rowA, index.column());
  }
  if(mModelB) {
    indexB = mModelB->index(entry.rowB, index.column());
  }
  
  if (mMode == OnlyA) {
    res = mModelA->data(indexA, role);
  } else if (mMode == OnlyB) {
    res = mModelB->data(indexB, role);
  } else if (role == Qt::ForegroundRole && (entry.rowA == -1 || entry.rowB == -1)) {
    if (entry.rowA == -1) {
      res = QBrush(Qt::darkBlue);
    } else {
      res = QBrush(Qt::darkMagenta);
    }
  } else if (index.column() > 0 && role == Qt::DisplayRole) {
    QVariant vA, vB;
    vA = mModelA->data(indexA, role);
    vB = mModelB->data(indexB, role);
   
    switch (mMode) {
    case ASubB:
      res = subtract(vA, vB);
      break;
    case BSubA:
      res = subtract(vB, vA);
      break;
    case ADivB:
      res = divide(vA, vB);
      break;
    case BDivA:
      res = divide(vB, vA);
      break;
    default:
      break;
    }
  } else if (index.column() == 0) {
    if (role == CallersRole || role == CalleesRole) {
      QSet<int> tmp;
      foreach(int i, mModelA->data(indexA, role).value<QList<int> >()) {
        tmp.insert(mMapA[i]);
      }
      foreach(int i, mModelB->data(indexB, role).value<QList<int> >()) {
        tmp.insert(mMapB[i]);
      }
      res = QVariant::fromValue(tmp.toList());
    } else  {
      if (entry.rowA != -1) {
        res = mModelA->data(indexA, role);
      } else {
        res = mModelB->data(indexB, role);
      }
    }
  }
  return res;
}

int DiffModel::findItem(const QString &name)
{
  for (int i = 0; i < mIndices.size(); ++i) {
    if (data(index(i, 0), FullNameRole).toString() == name) {
      return i;
    }
  }
  return -1;
}

QString DiffModel::htmlProfile(int item) const
{
  QString res, tmp;
  
  res += "<html>";
  tmp = data(index(item, 0), ProfileModel::FullNameRole).toString();
  tmp.replace('<', "&lt;");
  tmp.replace('>', "&gt;");
  res += "<head><title>" + tmp + "</title></head>";
  res += "<body>";
  res += "<h3><u>" + tmp + "</u></h3>";
  
  // call graph image
 
 
  // flat profile table
  QString flat;
  for (int i = 1; i < columnCount(); ++i) {
    QString rowStr;
    QVariant var;
    
    int row;
    if ((row = mDiffData[mIndices[item]].rowA) != -1) {
      var = mModelA->data(mModelA->index(row, i));
      
      if (var.isValid()) {
        rowStr += "<td>" + var.toString() + "</td>";
      }
    }
    
    if ((row = mDiffData[mIndices[item]].rowB) != -1) {
      var = mModelB->data(mModelB->index(row, i));
      
      if (var.isValid()) {
        if (mModelA && rowStr.isEmpty()) {
          rowStr += "<td></td>";
        }
        rowStr += "<td>" + var.toString() + "</td>";
      }
    }
    
    if (!rowStr.isEmpty()) {
      flat += "<tr><td><a href=\"whatsthis:" + headerData(i, Qt::Horizontal, Qt::WhatsThisRole).toString();
      flat += "\">" + headerData(i, Qt::Horizontal).toString() + "</a></td>";
      flat += rowStr + "</tr>";
    }
  }
  if (!flat.isEmpty()) {
    res += "<h4>Flat Profile:</h4>";
    res += "<table>";
  
    if (mModelA && mModelB) {
      res += "<tr><td></td>";
      if (mDiffData[mIndices[item]].rowA != -1) {
        res += "<td><b>A</b></td>";
      }
      if (mDiffData[mIndices[item]].rowB != -1) {
        res += "<td><b>B</b></td>";
      }
      res += "</tr>";
    }
    
    res += flat;
    res += "</table>";
  }
  
  // call graph table
  QList<int> indices = data(index(item, 0), ProfileModel::CallersRole).value<QList<int> >();
  bool onlyNote = false;
  
  if (!indices.isEmpty()) {
    res += "<h4>Called by:</h4>";
    res += dumpCallTable(indices, onlyNote);
  }
  
  indices = data(index(item, 0), ProfileModel::CalleesRole).value<QList<int> >();
  
  if (!indices.isEmpty()) {
    res += "<h4>Calls:</h4>";
    res += dumpCallTable(indices, onlyNote);
  }
  
  if (onlyNote) {
    res += "<p><sup>*</sup> Only in model A.<br>";
    res += "<sup>**</sup> Only in model B.</p>";
  }
  
  res += "</body></html>";
  return res;
}

QString DiffModel::dumpCallTable(const QList<int> &items, bool &note) const
{
  QString res, tmp;
  
  res += "<table>";
  foreach(int idx, items) {
    tmp = data(index(idx, 0), ProfileModel::FullNameRole).toString();
    tmp.replace('<', "&lt;");
    tmp.replace('>', "&gt;");
    
    res += "<tr>";
    res += "<td><a href=\"method:" + tmp + "\">" + tmp + "</a>";
    
    if (mDiffData[idx].rowA == -1 && mModelA) {
      res += " <sup>**</sup>";
      note = true;
    }
    if (mDiffData[idx].rowB == -1 && mModelB) {
      res += " <sup>*</sup>";
      note = true;
    }
    res += "</td></tr>";
  }
  res += "</table>";
  
  return res;
}

void DiffModel::updateModel()
{
  mDiffData.clear();
  mMapA.clear();
  mMapB.clear();
  
  if (mModelA) {
    for (int i = 0; i < mModelA->rowCount(); ++i) {
      DiffEntry item;
      item.rowA = i;
      mDiffData.append(item);
      mMapA.insert(i, i);
    }
  }
  
  if (mModelB) {
    for (int i = 0; i < mModelB->rowCount(); ++i) {
      int j = 0;
      for (; j < mDiffData.size(); ++j) {
        DiffEntry & entry = mDiffData[j];
        if (entry.rowA != -1 &&
            mModelA->data(mModelA->index(entry.rowA, 0), ProfileModel::FullNameRole) ==
            mModelB->data(mModelB->index(i, 0), ProfileModel::FullNameRole)) {
          entry.rowB = i;
          mMapB.insert(i, j);
          break;
        }
      }
      if (j == mDiffData.size()) {
        DiffEntry item;
        item.rowB = i;
        mDiffData.append(item);
        mMapB.insert(i, j);
      }
    }
  }
  
  updateIndices();
  reset();
}

void DiffModel::updateIndices()
{
  mIndices.resize(0);
  for (int i = 0; i < mDiffData.size(); ++i) {
    switch (mMode) {
    case OnlyA:
      if (mDiffData[i].rowA != -1) {
        mIndices.append(i);
      }
      break;
    case OnlyB:
      if (mDiffData[i].rowB != -1) {
        mIndices.append(i);
      }
      break;
    default:
      if ((mSingular || (mDiffData[i].rowA != -1 && mDiffData[i].rowB != -1))) {
        mIndices.append(i);
      }
      break;
    }
  }
}
