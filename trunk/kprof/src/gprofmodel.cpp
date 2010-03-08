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

// #define TEST_GPROF

#include <QRegExp>
#include <QTextStream>

#include <KLocale>

#include "gprofmodel.h"

#ifdef TEST_GPROF
#  include <QtDebug>
#endif

/*
 * Utility functions
 */
namespace {
  
  bool seekToRegExp(QTextStream &stream, QRegExp &exp, long *line = 0)
  {
    while (!stream.atEnd()) {
      if (line)
        ++(*line);

      if (stream.readLine().contains(exp))
        break;
    }

    return !stream.atEnd();
  }
  
  QString normalizeName(QString name)
  {
    QRegExp numExp("\\s*\\[\\d+\\]\\s*$");
    QRegExp cycleExp("\\s*<cycle \\d+>\\s*$");
    
    int pos;
    if ((pos = name.indexOf(numExp)) != -1) {
      name.remove(pos, numExp.matchedLength());
    }
    
    if ((pos = name.indexOf(cycleExp)) != -1) {
      name.remove(pos, cycleExp.matchedLength());
    }
    
    if (name.startsWith("global constructors")) {
      return name.simplified();
    }

    enum PrevToken {
      Id,
      LeftBracket,
      RightBracket,
      NameSpec,
      Other
    };

    // set of used regexps
    QString idStr("[_a-zA-Z][_a-zA-Z0-9]*");
    QRegExp spaceExp("^\\s+");
    QRegExp idExp("^" + idStr);
    QRegExp lbracketExp("^(\\(|\\[|<)");
    QRegExp rshiftExp("^(>>|>)");
    QRegExp nameSpecExp("^::");
    QRegExp destructorExp(QString("^(%1)\\s*::\\s*~\\s*(%1)").arg(idStr));

    QString res;
    PrevToken prev = NameSpec;
    pos = 0;
    
    while (pos < name.length()) {
      if (spaceExp.indexIn(name, pos, QRegExp::CaretAtOffset) != -1) {
        pos += spaceExp.matchedLength();
      } else if (destructorExp.indexIn(name, pos, QRegExp::CaretAtOffset) != -1) {
        pos += destructorExp.matchedLength();
        if (prev != NameSpec && prev != LeftBracket) {
          res.append(' ');
        }
        res.append(QString("%1::~%2").arg(destructorExp.cap(1), destructorExp.cap(2)));
        prev = Id;
      } else if (idExp.indexIn(name, pos, QRegExp::CaretAtOffset) != -1) {
        pos += idExp.matchedLength();
        if (prev != NameSpec && prev != LeftBracket) {
          res.append(' ');
        }
        res.append(idExp.cap(0));
        prev = Id;
      } else if (nameSpecExp.indexIn(name, pos, QRegExp::CaretAtOffset) != -1) {
        pos += nameSpecExp.matchedLength();
        res.append("::");
        prev = NameSpec;
      } else if (lbracketExp.indexIn(name, pos, QRegExp::CaretAtOffset) != -1) {
        pos += lbracketExp.matchedLength();
        res.append(lbracketExp.cap(1));
        prev = LeftBracket;
      } else if (rshiftExp.indexIn(name, pos, QRegExp::CaretAtOffset) != -1) {
        pos += rshiftExp.matchedLength();
        if(prev == RightBracket) {
          res.append(' ');
        }
        res.append(rshiftExp.cap(1));
        prev = RightBracket;
      } else {
        QChar c = name[pos++];
        res.append(c);
        if(c == ',') {
          res.append(' ');
        }
        prev = Other;
      }
    }
    return res;
  }

  void seekBracket(const QString &string, int &pos, char stop)
  {
    if (pos >= string.length())
      return;
    
    while (pos < string.length() && string[pos++] != stop) {
      switch (string[pos - 1].toAscii()) {
        case '(':
          seekBracket(string, pos, ')');
          break;
        case '[':
          seekBracket(string, pos, ']');
          break;
        case '<':
          if(string[pos] != '<') {
            seekBracket(string, pos, '>');
          }
          break;
      }
    }
  }

  QString abbreviateTemplates(const QString &name)
  {
    QString res;
    
    QRegExp breakExp("<|operator\\s*(<|<<)");
    
    int pos = 0, mark = 0;
    
    while (pos < name.length()) {
      if ((pos = breakExp.indexIn(name, pos)) != -1) {
        if (breakExp.cap(0) == "<") {
          res.append(name.mid(mark, pos - mark));
          res.append("<...>");
          pos += breakExp.matchedLength();
          seekBracket(name, pos, '>');
          mark = pos;
        } else {
          pos += breakExp.matchedLength();
        }
      } else {
        break;
      }
    }
    if (mark < name.length()) {
      res.append(name.right(name.length() - mark));
    }
    return res;
  }
  
  QStringList abbreviateTemplates(const QStringList &names)
  {
    QStringList res;
    
    foreach(const QString &str, names) {
      res.append(abbreviateTemplates(str));
    } 
    return res;
  }

#ifdef TEST_GPROF
  struct GProfTester {
    GProfTester() {
      qDebug() << "Testing GProf output parsing routines...";
      
      test("bbox<int, (int)3>** std::_Allocate<bbox<int, (int)3>*>(unsigned, bbox<int, (int)3>**)");
      test("Carpet::SplitRegions_Automatic_Recursively(vect<bool, (int)3> const&, int, vect<double, (int)3>, bbox<int, (int)3> const&, vect<vect<bool, (int)2>, (int)3> const&, int const&, std::vector<bbox<int, (int)3>, std::allocator<bbox<int, (int)3> > >&, std::vector<vect<vect<bool, (int)2>, (int)3>, std::allocator<vect<vect<bool, (int)2>, (int)3> > >&, std::vector<int, std::allocator<int> >&)");
      test("CarpetRegrid::ManualGridpoints_OneLevel(_cGH const*, gh<(int)3> const&, int, int, vect<int, (int)3>, vect<int, (int)3>, vect<vect<bool, (int)2>, (int)3>, std::vector<bbox<int, (int)3>, std::allocator<bbox<int, (int)3> > >&, std::vector<vect<vect<bool, (int)2>, (int)3>, std::allocator<vect<vect<bool, (int)2>, (int)3> > >&)");
      test("std::locale::id::operator unsigned()");
      test("gh<(int)3>** std::_Uninit_copy<gh<(int)3>*, gh<(int)3>*>(gh<(int)3>**, gh<(int)3>**, gh<(int)3>**, std::allocator<gh<(int)3>*>&, std::_Scalar_ptr_iterator_tag)");
    }
    
    void test(const QString & str) {
      qDebug() << ">>TEST:" << str;
      qDebug() << "abbreviated:" << abbreviateTemplates(str);
      
      GProfModel::ProfileInfo info;
      info.name = str;
      info.splitName();
      qDebug() << "method:" << info.method;
      qDebug() << "obj:" << info.object;
    }
  };
  
  GProfTester test;
#endif
}

/*
 * ProfileInfo class
 */
void GProfModel::ProfileInfo::splitName(void)
{
  QStringList parts;
  int pos = 0, mark = 0;
  
  if (name.startsWith("global constructors")) {
    parts << name;
    pos = mark = name.length();
  }

  // user regexps
  QRegExp breakExp("operator\\s*(<|<<)?|\\s+|<[^<]|\\(|\\[");
  
  while (pos < name.length()) {
    if ((pos = breakExp.indexIn(name, pos)) != -1) {
      if (breakExp.cap(0).startsWith("operator")) {
        pos += breakExp.matchedLength();
      } else if (breakExp.cap(0).startsWith('<')) {
        pos += breakExp.matchedLength();
        seekBracket(name, pos, '>');
      } else if (breakExp.cap(0).startsWith('(')) {
        pos += breakExp.matchedLength();
        seekBracket(name, pos, ')');
      } else if (breakExp.cap(0).startsWith('[')) {
        pos += breakExp.matchedLength();
        seekBracket(name, pos, ']');
      } else {  // space
        parts.append(name.mid(mark, pos - mark));
        pos += breakExp.matchedLength();
        mark = pos;
      }
    } else {
      break;
    }
  }
  if (mark < name.length()) {
    parts.append(name.right(name.length() - mark));
  }

  // determine which part is the method name
  int meth = parts.size() - 1;
  while (meth >= 0) {
    if (parts[meth].startsWith("const") || parts[meth].startsWith("throw")) {
      --meth;
    } else {
      break;
    }
  }
  
  if (meth < 0) {
    qDebug("Unable to parse method name: %s!", name.toAscii().data());
    method = name;
    return;
  }

  // split method into objects
  QRegExp parseStop("(::|operator\\s*(<|<<)|<|\\(|\\[)");
  QString methodName;
  
  pos = mark = 0;
  
  while ((pos = parseStop.indexIn(parts[meth], pos)) != -1) {
    if (parseStop.cap(1) == "::") {
      object.append(parts[meth].mid(mark, pos - mark));
      pos += 2;
      mark = pos;
    } else if (parseStop.cap(1).startsWith("operator")) {
      pos += parseStop.matchedLength();
    } else {
      switch (parseStop.cap(1).at(0).toAscii()) {
      case '<':
        seekBracket(parts[meth], ++pos, '>');
        break;
      case '(':
        seekBracket(parts[meth], ++pos, ')');
        break;
      case '[':
        seekBracket(parts[meth], ++pos, ']');
        break;
      }
    }
  }
  methodName = parts[meth].mid(mark, parts[meth].length() - mark);

  // glue it together
  parts[meth] = methodName;
  method = parts.join(" ");
}

void GProfModel::ProfileInfo::addCaller(int caller)
{
  if (!callers.contains(caller)) {
    callers.append(caller);
  }
}

void GProfModel::ProfileInfo::addCallee(int callee)
{
  if (!callees.contains(callee)) {
    callees.append(callee);
  }
}

/*
 * GProfModel class
 */
GProfModel::GProfModel(QObject * parent) : ProfileModel(parent), mShort(true)
{
}

QModelIndex GProfModel::index
(int row, int column, const QModelIndex &parent) const
{
  if (parent != QModelIndex())
    return QModelIndex();

  if (row < 0 || row >= mProfile.size() || column < 0 || column >= columnCount())
    return QModelIndex();

  return createIndex(row, column);
}


int GProfModel::rowCount(const QModelIndex &parent) const
{
  if (parent != QModelIndex())
    return 0;

  return mProfile.size();
}

int GProfModel::columnCount(const QModelIndex &parent) const
{
  if (parent != QModelIndex())
    return 0;

  return 9;
}

QVariant GProfModel::headerData
(int section, Qt::Orientation orientation, int role) const
{
  if (orientation != Qt::Horizontal)
    return QVariant();

  if (role == Qt::DisplayRole) {
    switch (section) {
    case Name:
      return i18n("Function Name");
    case SelfPercentage:
      return i18n("Self [%]");
    case TotalPercentage:
      return i18n("Total [%]");
    case SelfTime:
      return i18n("Self [s]");
    case TotalTime:
      return i18n("Total [s]");
    case ChildrenTime:
      return i18n("Children [s]");
    case Calls:
      return i18n("Calls");
    case SelfPerCall:
      return i18n("Self [ms/call]");
    case TimePerCall:
      return i18n("Total [ms/call]");
    }
  } else if (role == Qt::WhatsThisRole) {
    switch (section) {
    case Name:
      return i18n("The name of the function.");
    case SelfPercentage:
      return i18n("The percentage of the total running time of the program "
                  "used by this function.");
    case TotalPercentage:
      return i18n("This is the percentage of the `total' time that was spent "
                  "in this function and its children.  Note that due to "
                  "different viewpoints, functions excluded by options, etc, "
                  "these numbers will NOT add up to 100%.");
    case SelfTime:
      return i18n("The number of seconds accounted for by this function alone.");
    case TotalTime:
      return i18n("This is the total amount of time spent in this function.");
    case ChildrenTime:
      return i18n("This is the total amount of time propagated into this "
                  "function by its children.");
    case Calls:
      return i18n("The number of times this function was invoked.");
    case SelfPerCall:
      return i18n("The average number of milliseconds spent in this function per call.");
    case TimePerCall:
      return i18n("The average number of milliseconds spent "
                  "in this function and its descendents per call.");
    }
  }
  return QVariant();
}

QVariant GProfModel::data(const QModelIndex &index, int role) const
{
  if (index.row() < 0 || index.row() >= mProfile.size())
    return QVariant();
  
  if (index.column() == 0) {
    if (mShort && (role == NameRole || role == MethodRole || role == NamespaceRole)) {
      if (role == NameRole) {
        role = ShortNameRole;
      } else if (role == MethodRole) {
        role = ShortMethodRole;
      } else {
        role = ShortNamespaceRole;
      }
    }
    
    switch (role) {
    case NameRole:
    case FullNameRole:
    case Qt::ToolTipRole:
      return mProfile[index.row()].name;
    case NamespaceRole:
    case FullNamespaceRole:
      return mProfile[index.row()].object;
    case MethodRole:
    case FullMethodRole:
      return mProfile[index.row()].method;
    case ShortNameRole:
      return abbreviateTemplates(mProfile[index.row()].name);
    case ShortNamespaceRole:
      return abbreviateTemplates(mProfile[index.row()].object);
    case ShortMethodRole:
      return abbreviateTemplates(mProfile[index.row()].method);
    case CallersRole:
      return QVariant::fromValue(mProfile[index.row()].callers);
    case CalleesRole:
      return QVariant::fromValue(mProfile[index.row()].callees);
    }
  } else if (role == Qt::DisplayRole) {
    switch (index.column()) {
    case SelfPercentage:
      return mProfile[index.row()].selfPercentage;
    case TotalPercentage:
      return mProfile[index.row()].totalPercentage;
    case SelfTime:
      return mProfile[index.row()].selfSeconds;
    case TotalTime:
      return mProfile[index.row()].totalSeconds;
    case ChildrenTime:
      return mProfile[index.row()].childrenSeconds;
    case Calls:
      return mProfile[index.row()].calls;
    case SelfPerCall:
      return mProfile[index.row()].selfMsPerCall;
    case TimePerCall:
      return mProfile[index.row()].totalMsPerCall;
    }
  }
  return QVariant();
}

bool GProfModel::loadFile(QTextStream &stream)
{
  const QString floatPat("(\\d+\\.\\d+)");
  const QString intPat("(\\d+)");

  // section expressions
  QRegExp startFlatProfExp("^\\s*time\\s+seconds\\s+seconds(\\s+)"
                           "calls(\\s+)(ms|s)/call(\\s+)(ms|s)/call\\s+name\\s*$");
  QRegExp startCallGraphExp("^\\s*index\\s+%\\s+time\\s+"
                            "self\\s+children\\s+called\\s+name\\s*$");

  // find flat profile section
  if (!seekToRegExp(stream, startFlatProfExp))
    return false;
  
  float selfMsMod = startFlatProfExp.cap(3) == "ms" ? 1.0f : 1000.0f;
  float totalMsMod = startFlatProfExp.cap(5) == "ms" ? 1.0f : 1000.0f;

  int callsPadding = startFlatProfExp.cap(1).length() + 5;
  int selfPadding = startFlatProfExp.cap(2).length() + 6 + (selfMsMod == 1.0f ? 1 : 0);
  int totalPadding = startFlatProfExp.cap(4).length() + 6 + (totalMsMod == 1.0f ? 1 : 0);
  
  // flat profile regexp
  QRegExp flatProfExp(QString(
    "^\\s*%1\\s+%2\\s+%3\\s{1,%4}%5?\\s{1,%6}%7?\\s{1,%8}%9?\\s+(\\w.*)\\s*$").arg(
    floatPat, floatPat, floatPat,
    QString::number(callsPadding), intPat,
    QString::number(selfPadding), floatPat,
    QString::number(totalPadding), floatPat));
  mProfile.resize(0);

  // parse flat profile
  while (!stream.atEnd()) {
    if (!stream.readLine().contains(flatProfExp))
      break;
    
    ProfileInfo prof;
    prof.name = normalizeName(flatProfExp.cap(7));
    prof.selfPercentage = flatProfExp.cap(1).toDouble();
    prof.selfSeconds = flatProfExp.cap(3).toDouble();
    prof.totalSeconds = prof.selfSeconds;
    
    if(!flatProfExp.cap(4).isEmpty()) {
      prof.calls = flatProfExp.cap(4).toULongLong();
    }
    if(!flatProfExp.cap(5).isEmpty()) {
      prof.selfMsPerCall = flatProfExp.cap(5).toDouble() * selfMsMod;
    }
    if(!flatProfExp.cap(6).isEmpty()) {
      prof.totalMsPerCall = flatProfExp.cap(6).toDouble() * totalMsMod;
    }
    prof.recursive = false;
    prof.splitName();
    mProfile.append(prof);
  }

  // find call graph section
  if (!seekToRegExp(stream, startCallGraphExp))
    return false;

  // call graph regexps
  QRegExp spontaneousExp("^\\s*<spontaneous>\\s*$");
  QRegExp primaryExp(QString(
    "^\\s*\\[\\d+\\]\\s+%1\\s+%2\\s+%3\\s+(%4(\\+%6)?)?\\s+(\\w.*)$").arg(
    floatPat, floatPat, floatPat, intPat, intPat));
  QRegExp cgEntryExp(QString(
    "^\\s*%1?\\s+%2?\\s+%3(/%4)?\\s+(\\w.*)$").arg(
    floatPat, floatPat, intPat, intPat));
  QRegExp cycleExp("<cycle \\d+ as a whole>");
  QRegExp dashExp("^-+$");
  
  enum ParseState {
    ParseStart,
    ParseParents,
    ParsePrimary,
    ParseChildren,
    ParseStop
  };
  
  ParseState state = ParseStart;
  QList<int> parents;
  int primary = 0;
  QString line;
  
  while (!stream.atEnd() && state != ParseStop) {
    line = stream.readLine();
    
    switch (state) {
    case ParseStart:
      parents.clear();
      
      if (line.contains(cgEntryExp)) {
        int fnc = findFunction(normalizeName(cgEntryExp.cap(6)));
        parents.append(fnc);
        state = ParseParents;
      } else if (line.contains(spontaneousExp)) {
        state = ParsePrimary;
      } else if (line.contains(cycleExp)) {   // skip "whole cycle" entries
        if(!seekToRegExp(stream, dashExp)) {
          state = ParseStop;
        }
      } else {
        state = ParseStop;
      }
      break;
    case ParseParents:
      if (line.contains(cgEntryExp)) {
        int fnc = findFunction(normalizeName(cgEntryExp.cap(6)));
        parents.append(fnc);
      } else if (line.contains(primaryExp)) {
        primary = findFunction(normalizeName(primaryExp.cap(8)));
        mProfile[primary].callers = parents;
        foreach (int caller, parents) {
          mProfile[caller].addCallee(primary);
        }
        mProfile[primary].totalPercentage = primaryExp.cap(1).toDouble();
        mProfile[primary].selfSeconds = primaryExp.cap(2).toDouble();
        mProfile[primary].childrenSeconds = primaryExp.cap(3).toDouble();
        mProfile[primary].totalSeconds = mProfile[primary].selfSeconds.toDouble()
          + mProfile[primary].childrenSeconds.toDouble();
        state = ParseChildren;
      } else {
        state = ParseStop;
      }
      break;
    case ParsePrimary:
      if (line.contains(primaryExp)) {
        primary = findFunction(normalizeName(primaryExp.cap(8)));
        mProfile[primary].callers = parents;
        foreach (int caller, parents) {
          mProfile[caller].addCallee(primary);
        }
        mProfile[primary].totalPercentage = primaryExp.cap(1).toDouble();
        mProfile[primary].selfSeconds = primaryExp.cap(2).toDouble();
        mProfile[primary].childrenSeconds = primaryExp.cap(3).toDouble();
        mProfile[primary].totalSeconds = mProfile[primary].selfSeconds.toDouble()
          + mProfile[primary].childrenSeconds.toDouble();
        state = ParseChildren;
      } else {
        state = ParseStop;
      }
      break;
    case ParseChildren:
      if (line.contains(cgEntryExp)) {
        int fnc = findFunction(normalizeName(cgEntryExp.cap(6)));
        mProfile[primary].addCallee(fnc);
        if(fnc == primary) {
          mProfile[primary].recursive = true;
        }
      } else if (line.contains(dashExp)) {
        state = ParseStart;
      } else {
        state = ParseStop;
      }
      break;
    default:
      break;
    }
  }
  reset();

  return true;
}

int GProfModel::findFunction(const QString & name)
{
  for(int i = 0; i < mProfile.size(); ++i) {
    if(mProfile[i].name == name) {
      return i;
    }
  }
  
  ProfileInfo info;
  info.name = name;
  info.recursive = false;
  info.splitName();
  
  mProfile.append(info);
  return mProfile.size() - 1;
}

void GProfModel::setShortNames(bool sh)
{
  if (mShort != sh) {
    mShort = sh;
    
    reset();
  }
}
