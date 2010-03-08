/*
 * kprof.cpp
 *
 * $Id: kprof.cpp,v 1.19 2004/06/19 05:18:54 bab Exp $
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

#include <QHBoxLayout>
#include <QLabel>
#include <QTextStream>

#include <KCmdLineArgs>
#include <KToolBar>
#include <KConfig>
#include <KConfigGroup>
#include <KLocale>
#include <KGlobal>
#include <KAction>
#include <KActionCollection>
#include <KStandardAction>
#include <KRecentFilesAction>
#include <KFileDialog>
#include <KMessageBox>
#include <KProcess>
#include <KIO/NetAccess>
#include <KLineEdit>

#include "kproftoplevel.h"
#include "kprofwidget.h"

#include "gprofmodel.h"

KProfTopLevel::KProfTopLevel(QWidget *parent) : KXmlGuiWindow(parent)
{
  mKProfWidget = new KProfWidget(this);

  setCentralWidget(mKProfWidget);
  
  setupActions();

  setStandardToolBarMenuEnabled(true);
  setupGUI(Default, "kprofui.rc");

  setupToolBars();

  // load the recent files list
  KSharedConfigPtr config = KGlobal::config();
  mRecent->loadEntries(config->group("Recent"));

  loadSettings();
  saveSettings();

  openCommandLineFiles();
}

KProfTopLevel::~KProfTopLevel()
{
}

bool KProfTopLevel::queryExit()
{
  KSharedConfigPtr config = KGlobal::config();

  mRecent->saveEntries(config->group("Recent"));
  saveSettings();
  return true;
}

void KProfTopLevel::openFile()
{
  KUrl url = KFileDialog::getOpenUrl(mCurrentDir, QString(), this,
                                     i18n("Select profiling results..."));

  if (url.isEmpty())
    return;

  // if user tried to open gmon.out, have him select the executable instead
  if(url.fileName() == "gmon.out") {
    KMessageBox::error(this,
                       i18n("File 'gmon.out' is the result of the execution of an application "
                            "with gprof(1) profiling turned on. You can not open it as such: "
                            "either open the application itself or open a text results file "
                            "generated with 'gprof -b application-name'"),
                       i18n("Opening gmon.out not allowed"));
    return;
  }
  
  QString tmpFile;
  bool ok;

  if (KIO::NetAccess::download(url, tmpFile, this)) {
    if (QFileInfo(tmpFile).isExecutable()) {
      if (url.protocol() != "file") {
        KMessageBox::error(this, i18n("Cannot open remote executables."));
        return;
      }
      
      ok = gprofApplication(tmpFile, false);
    } else {
      ok = loadFile(tmpFile, false);
    }
    // store filename in recent files and change the window title
    mRecent->addUrl(url);
    mCurrentDir = url;

    setCaption(url.fileName());
    mCompareFile->setEnabled(true);

    KIO::NetAccess::removeTempFile(tmpFile);
  } else {
    KMessageBox::error(this,
                       KIO::NetAccess::lastErrorString());
  }
}

void KProfTopLevel::openRecentFile(KUrl url)
{
  if(url.isEmpty() || url.fileName() == "gmon.out") {
    return;
  }
  
  QString tmpFile;
  bool ok;
  
  if (KIO::NetAccess::download(url, tmpFile, this)) {
    if (QFileInfo(tmpFile).isExecutable()) {
      if (url.protocol() != "file") {
        KMessageBox::error(this, i18n("Cannot open remote executables."));
        return;
      }
      
      ok = gprofApplication(tmpFile, false);
    } else {
      ok = loadFile(tmpFile, false);
    }
    mRecent->addUrl(url);
    mCurrentDir = url;
      
    setCaption(url.fileName());
    mCompareFile->setEnabled(true);

    KIO::NetAccess::removeTempFile(tmpFile);
  } else {
    KMessageBox::error(this,
                       KIO::NetAccess::lastErrorString());
  }
}

void KProfTopLevel::compareFile()
{
  KUrl url = KFileDialog::getOpenUrl(mCurrentDir, QString(), this,
                                     i18n("Select profiling results..."));
  
  if (url.isEmpty())
    return;

  if(url.fileName() == "gmon.out") {
    KMessageBox::error(this,
                       i18n("File 'gmon.out' is the result of the execution of an application "
                            "with gprof(1) profiling turned on. You can not open it as such: "
                            "either open the application itself or open a text results file "
                            "generated with 'gprof -b application-name'"),
                       i18n("Opening gmon.out not allowed"));
    return;
  }
  
  QString tmpFile;

  if (KIO::NetAccess::download(url, tmpFile, this)) {
    if (QFileInfo(tmpFile).isExecutable()) {
      if (url.protocol() != "file") {
        KMessageBox::error(this, i18n("Cannot open remote executables."));
        return;
      }
      
      gprofApplication(tmpFile, true);
    } else {
      loadFile(tmpFile, true);
    }
    mRecent->addUrl(url);
    mCurrentDir = url;
    
    KIO::NetAccess::removeTempFile(tmpFile);
  } else {
    KMessageBox::error(this,
                       KIO::NetAccess::lastErrorString());
  }
}

void KProfTopLevel::loadSettings()
{
  KSharedConfigPtr config = KGlobal::config();

  KConfigGroup group = config->group("KProfiler");
  int w = group.readEntry("Width", QVariant(width())).toInt();
  int h = group.readEntry("Height", QVariant(height())).toInt();
  resize(w, h);
  
  mCurrentDir = group.readEntry("Directory", KUrl());
}

void KProfTopLevel::saveSettings()
{
  KSharedConfigPtr config = KGlobal::config();

  KConfigGroup group = config->group("KProfiler");
  group.writeEntry("Width", width());
  group.writeEntry("Height", height());
  
  group.writeEntry("Directory", mCurrentDir);

  config->sync();
}

void KProfTopLevel::openCommandLineFiles()
{
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QString fileName1, fileName2;

  if (args->isSet("f")) {
    fileName1 = args->getOption("f");
  }
  
  if (args->isSet("c")) {
    fileName2 = args->getOption("c");
  }

  args->clear();

  if(!fileName1.isEmpty()) {
    loadFile(fileName1, false);
    
    if(fileName2.isEmpty()) {
      loadFile(fileName2, true);
    }
  }
}

bool KProfTopLevel::loadFile(const QString &fileName, bool compare)
{
  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;
  
  QTextStream stream(&file);
  return loadFile(stream, compare);
}

bool KProfTopLevel::loadFile(QTextStream &stream, bool compare)
{
  GProfModel * model = new GProfModel(this);

  if (!model->loadFile(stream)) {
    KMessageBox::error(this, i18n("Error loading profile results."));
    delete model;
    return false;
  }

  // if model is ok, replace it
  if(compare) {
    mKProfWidget->setModelB(model);
  } else {
    mKProfWidget->setModelA(model);
  }
  
  updateActions();
  return true;
}

bool KProfTopLevel::gprofApplication(const QString &fileName, bool compare)
{
  QFileInfo finfo(fileName);

  if(!finfo.isExecutable()) {
    return false;
  }

  // prepare the "gmon.out" filename
  QString outfile = finfo.absolutePath() + "/gmon.out";

  if (!QFileInfo(outfile).exists()) {
    KMessageBox::error(this, i18n("Can't find any profiling output file "
                                  "(gmon.out)"), i18n("File not found"));
    return false;
  }

  KProcess profileGenerator;

  profileGenerator.setEnv("LC_ALL", "C");
  QStringList commands;

  // GNU gprof analysis of gmon.out file
  // exec "gprof -b filename gmon.out"
  commands << "gprof" << "-b" << fileName << outfile;

  profileGenerator << commands;
  profileGenerator.execute();

  QByteArray profStdout = profileGenerator.readAllStandardOutput();
  QByteArray profStderr = profileGenerator.readAllStandardError();

  if (!profileGenerator.execute() || profileGenerator.exitCode()) {
    QString details;
    details += i18n("Command:\n") + commands.join(" ") + "\n";

    if (profileGenerator.exitCode()) {
      details += i18n("Received error code: %1\n").arg(
                    QString::number(profileGenerator.exitCode()));
    } else {
      if (profileGenerator.error() == QProcess::FailedToStart) {
        details += i18n(
                      "\nThe process failed to start. Either the invoked "
                      "program is missing, or you may have insufficient "
                      "permissions to invoke the program.\n\n");
      } else {
        details += i18n("The process crashed!\n\n");
      }
    }

    if (!profStderr.isEmpty()) {
      details += i18n("Error message(s):\n");
      details += profStderr;
    }

    if (!profStdout.isEmpty()) {
      details += i18n("Dump of standard output:\n");
      details += profStdout;
    }

    KMessageBox::detailedError(this,
                                i18n("Could not generate the profile data."),
                                details, i18n("Error running profiler"));
    return false;
  }

  QTextStream stream(&profStdout, QIODevice::ReadOnly);
  return loadFile(stream, compare);
}

void KProfTopLevel::setupActions()
{
  KStandardAction::open(this, SLOT(openFile()), actionCollection());

  mRecent = KStandardAction::openRecent(this, SLOT(openRecentFile(KUrl)),
                                        actionCollection());

  mCompareFile = new KAction(i18n("Compare With..."), this);
  mCompareFile->setEnabled(false);
  connect(mCompareFile, SIGNAL(triggered()), SLOT(compareFile()));
  actionCollection()->addAction("compareFile", mCompareFile);
  
  KStandardAction::print(this, SLOT(doPrint()), actionCollection());
  KStandardAction::printPreview(this, SLOT(previewPrint()),
                                actionCollection());
  KStandardAction::quit(this, SLOT(close()), actionCollection());

  KStandardAction::preferences(this, SLOT(configure()), actionCollection());
  
  mModeOnlyA = new KAction(i18n("Only A"), this);
  mModeOnlyA->setCheckable(true);
  mModeOnlyA->setShortcut(i18n("Ctrl+1"));
  connect(mModeOnlyA, SIGNAL(triggered(bool)), SLOT(onOnlyATriggered()));
  actionCollection()->addAction("onlyA", mModeOnlyA);
  
  mModeOnlyB = new KAction(i18n("Only B"), this);
  mModeOnlyB->setCheckable(true);
  mModeOnlyB->setShortcut(i18n("Ctrl+2"));
  connect(mModeOnlyB, SIGNAL(triggered(bool)), SLOT(onOnlyBTriggered()));
  actionCollection()->addAction("onlyB", mModeOnlyB);
  
  mModeSubAB = new KAction(i18n("A - B"), this);
  mModeSubAB->setCheckable(true);
  mModeSubAB->setShortcut(i18n("Ctrl+3"));
  connect(mModeSubAB, SIGNAL(triggered(bool)), SLOT(onSubABTriggered()));
  actionCollection()->addAction("subAB", mModeSubAB);
  
  mModeSubBA = new KAction(i18n("B - A"), this);
  mModeSubBA->setCheckable(true);
  mModeSubBA->setShortcut(i18n("Ctrl+4"));
  connect(mModeSubBA, SIGNAL(triggered(bool)), SLOT(onSubBATriggered()));
  actionCollection()->addAction("subBA", mModeSubBA);
  
  mModeDivAB = new KAction(i18n("A / B"), this);
  mModeDivAB->setCheckable(true);
  mModeDivAB->setShortcut(i18n("Ctrl+5"));
  connect(mModeDivAB, SIGNAL(triggered(bool)), SLOT(onDivABTriggered()));
  actionCollection()->addAction("divAB", mModeDivAB);
  
  mModeDivBA = new KAction(i18n("B / A"), this);
  mModeDivBA->setCheckable(true);
  mModeDivBA->setShortcut(i18n("Ctrl+6"));
  connect(mModeDivBA, SIGNAL(triggered(bool)), SLOT(onDivBATriggered()));
  actionCollection()->addAction("divBA", mModeDivBA);
  
  QActionGroup * viewGroup = new QActionGroup(this);
  viewGroup->addAction(mModeOnlyA);
  viewGroup->addAction(mModeOnlyB);
  viewGroup->addAction(mModeSubAB);
  viewGroup->addAction(mModeSubBA);
  viewGroup->addAction(mModeDivAB);
  viewGroup->addAction(mModeDivBA);
  viewGroup->setExclusive(true);
  
  mShortNames = new KAction(i18n("Short Names"), this);
  mShortNames->setCheckable(true);
  connect(mShortNames, SIGNAL(triggered(bool)), mKProfWidget, SLOT(setShortNames(bool)));
  actionCollection()->addAction("shortNames", mShortNames);
  
  mSingularEntries = new KAction(i18n("Singular Entries"), this);
  mSingularEntries->setCheckable(true);
  connect(mSingularEntries, SIGNAL(triggered(bool)), mKProfWidget, SLOT(setSingularEntries(bool)));
  actionCollection()->addAction("singular", mSingularEntries);
  
  updateActions();
}

void KProfTopLevel::setupToolBars()
{
  KToolBar * filterBar = toolBar("filterBar");
  filterBar->setWindowTitle(i18n("Filter ToolBar"));

  QWidget * wdg = new QWidget(this);
  QHBoxLayout * hbox = new QHBoxLayout(wdg);
 
  KLineEdit * filter = new KLineEdit;
  connect(filter, SIGNAL(textChanged(QString)), mKProfWidget, SLOT(setFilter(QString)));
  
  QLabel * lbl = new QLabel(i18n("Filter:"), wdg);
  lbl->setBuddy(filter);

  hbox->addWidget(lbl);
  hbox->addWidget(filter);
  filterBar->addWidget(wdg);

  // add some help on items
  filter->setWhatsThis(
    i18n("Type text in this field to filter the display "
         "and only show the functions/methods whose name match the text."));
}

void KProfTopLevel::updateActions()
{
  bool hasModel = mKProfWidget->modelA() || mKProfWidget->modelB();
  bool hasBoth = mKProfWidget->modelA() && mKProfWidget->modelB();
  
  mModeOnlyA->setEnabled(mKProfWidget->modelA());
  mModeOnlyB->setEnabled(mKProfWidget->modelB());
  mModeSubAB->setEnabled(hasBoth);
  mModeSubBA->setEnabled(hasBoth);
  mModeDivAB->setEnabled(hasBoth);
  mModeDivBA->setEnabled(hasBoth);

  switch (mKProfWidget->displayMode()) {
  case DiffModel::OnlyA:
    mModeOnlyA->setChecked(true);
    break;
  case DiffModel::OnlyB:
    mModeOnlyB->setChecked(true);
    break;
  case DiffModel::ASubB:
    mModeSubAB->setChecked(true);
    break;
  case DiffModel::BSubA:
    mModeSubBA->setChecked(true);
    break;
  case DiffModel::ADivB:
    mModeDivAB->setChecked(true);
    break;
  case DiffModel::BDivA:
    mModeDivBA->setChecked(true);
    break;
  }
  
  mShortNames->setEnabled(hasModel);
  mShortNames->setChecked(mKProfWidget->shortNames());
  
  mSingularEntries->setEnabled(hasModel);
  mSingularEntries->setChecked(mKProfWidget->singularEntries());
}

void KProfTopLevel::onOnlyATriggered()
{
  mKProfWidget->setDisplayMode(DiffModel::OnlyA);
}

void KProfTopLevel::onOnlyBTriggered()
{
  mKProfWidget->setDisplayMode(DiffModel::OnlyB);
}

void KProfTopLevel::onSubABTriggered()
{
  mKProfWidget->setDisplayMode(DiffModel::ASubB);
}

void KProfTopLevel::onSubBATriggered()
{
  mKProfWidget->setDisplayMode(DiffModel::BSubA);
}

void KProfTopLevel::onDivABTriggered()
{
  mKProfWidget->setDisplayMode(DiffModel::ADivB);
}

void KProfTopLevel::onDivBATriggered()
{
  mKProfWidget->setDisplayMode(DiffModel::BDivA);
}
