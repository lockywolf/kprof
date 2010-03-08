/*
 * kprof.h
 *
 * $Id: kprof.h,v 1.12 2002/10/29 21:50:45 cdesmond Exp $
 *
 * Copyright (c) 2000 Florent Pillet <florent.pillet@wanadoo.fr>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.trolltech.com/
 *
 * Requires the K Desktop Environment 3.0 (KDE 3.0) libraries, available
 * at no cost at http://www.kde.org/
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

#ifndef KPROFTOPLEVEL_H_
#define KPROFTOPLEVEL_H_

#include <KXmlGuiWindow>
#include <KUrl>

class QTextStream;

class KAction;
class KRecentFilesAction;
class KProfWidget;

// TODO: configuration
//         -whether to save and where

class KProfTopLevel : public KXmlGuiWindow
{
  Q_OBJECT

  public:
    KProfTopLevel(QWidget *parent = 0);
    ~KProfTopLevel();
    
  protected:
    KProfWidget * mKProfWidget;
    
  protected:
    void loadSettings();
    void saveSettings();
    
  protected slots:
    virtual bool queryExit();

    void openFile();
    void openRecentFile(KUrl url);
    void compareFile();
    
  private:
    KUrl mCurrentDir;

    KAction * mCompareFile;
    
    KAction * mModeOnlyA;
    KAction * mModeOnlyB;
    KAction * mModeSubAB;
    KAction * mModeSubBA;
    KAction * mModeDivAB;
    KAction * mModeDivBA;
    
    KAction * mShortNames;
    KAction * mSingularEntries;
    
//     KAction * mRunApplication;
//     KAction * mDisplayTreeMapAction;
    KRecentFilesAction * mRecent;

  private:
    void openCommandLineFiles();
    
    bool loadFile(const QString & fileName, bool compare);
    bool loadFile(QTextStream & stream, bool compare);
    bool gprofApplication(const QString &fileName, bool compare);

    void setupActions();
    void setupToolBars();
    
    void updateActions();
    
  private slots:
    void onOnlyATriggered();
    void onOnlyBTriggered();
    void onSubABTriggered();
    void onSubBATriggered();
    void onDivABTriggered();
    void onDivBATriggered();
};

#endif
