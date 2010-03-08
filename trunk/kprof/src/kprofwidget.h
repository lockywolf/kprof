/*
 * kprofwidget.h
 *
 * $Id: kprofwidget.h,v 1.35 2004/06/19 05:18:55 bab Exp $
 *
 * Copyright (c) 2000-2001 Florent Pillet <fpillet@users.sourceforge.net>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.trolltech.com/
 *
 * Requires the K Desktop Environment 2.0 (KDE 2.0) libraries or later,
 * available at no cost at http://www.kde.org/
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

#ifndef KPROFWIDGET_H_
#define KPROFWIDGET_H_

// TODO: pri nacteni druheho modelu refreshnout zalozky, nebo zavrit

#include <KTabWidget>

class QTreeView;
class QUrl;

#include "diffmodel.h"

class ProfileProxyView;

class KProfWidget : public KTabWidget
{
  Q_OBJECT

  public:
    KProfWidget(QWidget *parent = 0);
    ~KProfWidget();

  public:
    const QList<QTreeView*> views() const;
    
    const ProfileModel * modelA() const {return mSrcModelA;}
    const ProfileModel * modelB() const {return mSrcModelB;}
    
    void setModelA(ProfileModel * model);
    void setModelB(ProfileModel * model);
    void setModels(ProfileModel * modelA, ProfileModel * modelB);
    
    bool shortNames() const;
    bool singularEntries() const;
    DiffModel::DisplayMode displayMode() const;
    
  public slots:
    void setShortNames(bool sh);
    void setSingularEntries(bool sng);
    void setDisplayMode(DiffModel::DisplayMode mode);
    
    void setFilter(const QString &filter);
    
  protected:
    ProfileProxyView * mFlat;
    ProfileProxyView * mObject;
    ProfileProxyView * mHierarchic;
    
    DiffModel * mDiffModel;
    ProfileModel * mSrcModelA;
    ProfileModel * mSrcModelB;
    
  private:
    void updateView();
    void showDetails(const QString &name);
    
  private slots:
    void onTabCloseRequested(int index);
    void onShowDetails(int index);
    void onBrowserAnchorClicked(const QUrl &url);
    
// public slots:
//  void settingsChanged ();
//  void loadSettings ();
//  void applySettings ();
//
//  void doPrint ();
//
// //  void profileEntryRightClick (QTreeWidgetItem *listItem, const QPoint &p, int);
//  void flatProfileFilterChanged (const QString &filter);
//  void generateCallGraph ();
//
//  void toggleTemplateAbbrev ();
//  void selectListFont ();
//  void configure();
//
// protected slots:
// //  void selectionChanged (QTreeWidgetItem *item);
// //  void openURLRequestDelayed( const KUrl &, const KParts::OpenUrlArguments& args1, const KParts::BrowserArguments& args2);
//
// private:
//  void openFile (const QString &filename, ProfilerEnumeration format, bool compare = false);
//  void prepareProfileView (QTreeWidget * view, bool rootIsDecorated);
//  void postProcessProfile (bool compare);
//  void prepareHtmlPart(KHTMLPart* part);
//
//  void customizeColumns (QTreeWidget *view, int profiler);
//
//  void fillFlatProfileList ();
//  void fillHierProfileList ();
// // NOTE: used to be QArray<ProfileInfo>
//  void fillHierarchy (CProfileViewItem *item, ProfileInfo *parent, QVector<ProfileInfo *> &addedEntries, int &count);
//  void fillObjsProfileList ();
//
//  void selectProfileItem (ProfileInfo *info);
//  void selectItemInView (QTreeWidget *view, ProfileInfo *info, bool examineSubs);
//
//  void markForOutput (ProfileInfo *info);
//
//  QString removeTemplates (const QString& name);
//
//  QString processName;
//  KHTMLPart* mCallTreeHtmlPart;
//  KHTMLPart* mMethodHtmlPart;
// //  CConfigure* mConfigure;
};

#endif
