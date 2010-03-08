/*
 * kprofwidget.cpp
 *
 * $Id: kprofwidget.cpp,v 1.56 2004/07/03 06:03:50 bab Exp $
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

#include <QtDebug>

#include <QTabBar>
#include <QWhatsThis>
#include <QTextBrowser>

#include <KLocale>

#include "kprofwidget.h"
#include "profilemodel.h"
#include "diffmodel.h"
#include "flatproxymodel.h"
#include "objectproxymodel.h"
#include "hierarchicproxymodel.h"
#include "profileproxyview.h"

KProfWidget::KProfWidget(QWidget *parent) : KTabWidget(parent),
  mDiffModel(0), mSrcModelA(0), mSrcModelB(0)
{
  mDiffModel = new DiffModel(this);
    
  // create flat profile list
  mFlat = new ProfileProxyView(this);
  Q_CHECK_PTR(mFlat);
  mFlat->setObjectName("flatProfile");
  mFlat->setProxyModel(new FlatProxyModel());
  mFlat->setSourceModel(mDiffModel);
  connect(mFlat, SIGNAL(showDetails(int)), SLOT(onShowDetails(int)));

  // create object profile list
  mObject = new ProfileProxyView(this);
  Q_CHECK_PTR(mObject);
  mObject->setObjectName("objectProfile");
  mObject->setProxyModel(new ObjectProxyModel());
  mObject->setSourceModel(mDiffModel);
  connect(mObject, SIGNAL(showDetails(int)), SLOT(onShowDetails(int)));
  
  // create hierarchical profile list
  mHierarchic = new ProfileProxyView(this);
  Q_CHECK_PTR(mHierarchic);
  mHierarchic->setObjectName("hierarchicalProfile");
  mHierarchic->setProxyModel(new HierarchicProxyModel());
  mHierarchic->setSourceModel(mDiffModel);
  connect(mHierarchic, SIGNAL(showDetails(int)), SLOT(onShowDetails(int)));

  // add the view to the tab control  
  addTab(mFlat, i18n("&Flat Profile"));
  addTab(mObject, i18n("O&bject Profile"));
  addTab(mHierarchic, i18n("&Hierarchical Profile"));

  // enable close button and disable closing of the first three tabs
  setTabsClosable(true);
  setAutomaticResizeTabs(true);
  
  for(int i = 0; i < 3; ++i) {
    QWidget * btn;
    btn = tabBar()->tabButton(i, QTabBar::LeftSide);
    if(btn) {
      btn->setEnabled(false);
    }
    
    btn = tabBar()->tabButton(i, QTabBar::RightSide);
    if(btn) {
      btn->setEnabled(false);
    }
  }
  
  connect(this, SIGNAL(tabCloseRequested(int)), SLOT(onTabCloseRequested(int)));
  
  // add some help on items
  mFlat->setWhatsThis(i18n(
    "This is the <i>flat view</i>.\n\nIt displays all functions and method "
    "'flat'. Click on a column header to sort the list on this column (click "
    "a second time to reverse the sort order)."));
  mHierarchic->setWhatsThis(i18n(
    "This is the <i>hierarchical view</i>.\n\nIt displays each function/method "
    "like a tree to let you see the other functions that it calls. For that "
    "reason, each function may appear several times in the list."));
  mObject->setWhatsThis(i18n(
    "This is the <i>object view</i>.\n\n"
    "It displays C++ methods grouped by object name."));
}

KProfWidget::~KProfWidget()
{
}

const QList<QTreeView*> KProfWidget::views() const
{
  QList<QTreeView*> res;
  res << mFlat << mHierarchic << mObject;
  return res;
}

void KProfWidget::setModelA(ProfileModel * model)
{
  if(model) {
    model->setParent(this);
  }
  mDiffModel->setModelA(model);
  
  // remove old model
  delete mSrcModelA;
  mSrcModelA = model;
  
  updateView();
}

void KProfWidget::setModelB(ProfileModel * model)
{
  if(model) {
    model->setParent(this);
  }
  mDiffModel->setModelB(model);
 
  // remove old model
  delete mSrcModelB;
  mSrcModelB = model;

  updateView();
}

void KProfWidget::setModels(ProfileModel * modelA, ProfileModel * modelB)
{
  if(modelA) {
    modelA->setParent(this);
  }
  
  if(modelB) {
    modelB->setParent(this);
  }
  
  mDiffModel->setModels(modelA, modelB);
  
  delete mSrcModelA;
  mSrcModelA = modelA;
  
  delete mSrcModelB;
  mSrcModelB = modelB;
    
  updateView();
}

bool KProfWidget::shortNames() const
{
  return mDiffModel->shortNames();
}

bool KProfWidget::singularEntries() const
{
  return mDiffModel->showSingular();
}

DiffModel::DisplayMode KProfWidget::displayMode() const
{
  return mDiffModel->mode();
}

void KProfWidget::setShortNames(bool sh)
{
  bool resize = sh != mDiffModel->shortNames();
  
  mDiffModel->setShortNames(sh);
  
  if (resize) {
    mFlat->resizeColumnToContents(0);
    mObject->resizeColumnToContents(0);
    mHierarchic->resizeColumnToContents(0);
  }
}

void KProfWidget::setSingularEntries(bool sng)
{
  bool resize = sng != mDiffModel->showSingular();
  
  mDiffModel->setShowSingular(sng);
  
  if (resize) {
    mFlat->resizeColumnToContents(0);
    mObject->resizeColumnToContents(0);
    mHierarchic->resizeColumnToContents(0);
  }
}

void KProfWidget::setDisplayMode(DiffModel::DisplayMode mode)
{
  mDiffModel->setMode(mode);
}

void KProfWidget::setFilter(const QString &filter)
{
  mFlat->setFilter(filter);
  mObject->setFilter(filter);
  mHierarchic->setFilter(filter);
}

void KProfWidget::updateView()
{
  // resize columns
  for (int i = 0; i < mDiffModel->columnCount(); ++i) {
    mFlat->resizeColumnToContents(i);
    mObject->resizeColumnToContents(i);
    mHierarchic->resizeColumnToContents(i);
  }
}

void KProfWidget::showDetails(const QString &name)
{
  for (int i = 0; i < count(); ++i) {
    QTextBrowser * browser = qobject_cast<QTextBrowser*>(widget(i));
    
    // just switch to that tab
    if (browser && browser->documentTitle() == name) {
      setCurrentIndex(i);
      return;
    }
  }
  
  int index = mDiffModel->findItem(name);
  
  if(index == -1) {
    return;
  }
  
  // otherwise create the tab
  QTextBrowser * browser = new QTextBrowser(this);
  browser->setOpenLinks(false);
  browser->setHtml(mDiffModel->htmlProfile(index));
  
  connect(browser, SIGNAL(anchorClicked(QUrl)), SLOT(onBrowserAnchorClicked(QUrl)));
  
  QString tabName = mDiffModel->data(mDiffModel->index(index, 0),
                                  ProfileModel::ShortMethodRole).toString();
  tabName.replace("&", "&&");
  addTab(browser, tabName);
  setCurrentWidget(browser);
  
  // HACK: doesn't seem to work otherwise
  setAutomaticResizeTabs(false);
  setAutomaticResizeTabs(true);
}

void KProfWidget::onTabCloseRequested(int index)
{
  QTextBrowser * browser = qobject_cast<QTextBrowser*>(widget(index));
  
  if (browser) {
    removePage(browser);
    delete browser;
  }
}

void KProfWidget::onShowDetails(int index)
{
  if (index < 0 || index >= mDiffModel->rowCount())
    return;
  
  QString name = mDiffModel->data(mDiffModel->index(index, 0),
                                  ProfileModel::FullNameRole).toString();
  
  showDetails(name);
}

void KProfWidget::onBrowserAnchorClicked(const QUrl &url)
{
  if (url.scheme() == "whatsthis") {
    QWhatsThis::showText(QCursor::pos(), url.path());
  } else {
    showDetails(url.path());
  }
}

//void KProfWidget::selectionChanged (QTreeWidgetItem *item)
//{
// 	QTreeWidget *view = item->listView();
// 	ProfileInfo *info = ((CProfileViewItem *)item)->getProfile ();
// 	if (view != mFlat)
// 	{
// 		disconnect (mFlat, SIGNAL (selectionChanged (QTreeWidgetItem *)),
// 			 this, SLOT (selectionChanged (QTreeWidgetItem *)));
// 		selectItemInView (mFlat, info, false);
// 		connect (mFlat, SIGNAL (selectionChanged (QTreeWidgetItem *)),
// 				 this, SLOT (selectionChanged (QTreeWidgetItem *)));
// 	}
// 	if (view != mHier)
// 	{
// 		disconnect (mHier, SIGNAL (selectionChanged (QTreeWidgetItem *)),
// 			 this, SLOT (selectionChanged (QTreeWidgetItem *)));
// 		selectItemInView (mHier, info, false);
// 		connect (mHier, SIGNAL (selectionChanged (QTreeWidgetItem *)),
// 				 this, SLOT (selectionChanged (QTreeWidgetItem *)));
// 	}
// 	if (view != mObjs)
// 	{
// 		disconnect (mObjs, SIGNAL (selectionChanged (QTreeWidgetItem *)),
// 			 this, SLOT (selectionChanged (QTreeWidgetItem *)));
// 		selectItemInView (mObjs, info, true);
// 		connect (mObjs, SIGNAL (selectionChanged (QTreeWidgetItem *)),
// 				 this, SLOT (selectionChanged (QTreeWidgetItem *)));
// 	}
//}

//void KProfWidget::doPrint()
//{
// 	KListView *view =	mCurPage == 0 ? mFlat :
// 						mCurPage == 1 ? mHier :
// 										mObjs;
// 
// 	QString s;
// 	s = "<HTML><HEAD><META http-equiv=\"content-type\" content=\"text/html\" charset=\"iso-8859-1\"></HEAD>";
// 	s += "<BODY bgcolor=\"#FFFFFF\">";
// 	s += "<TABLE border=\"0\" cellspacing=\"2\" cellpadding=\"1\">";
// 
// 	QTreeWidgetItem *item = view->firstChild ();
// 	int cols = view->columns();
// 
// 	s += "<BR><BR><THEAD><TR>";		// two BRs to alleviate for margin problems
// 	for (int i=0; i<cols; i++)
// 		if (i==0)
// 			s += "<TH align=\"left\"><B>" + view->columnText(i) + "</B></TH>";
// 		else
// 			s += "<TH align=\"right\"><B>" + view->columnText(i) + "</B></TH>";
// 	s += "</TR></THEAD><TBODY>";
// 
// 	while (item) {
// 		CProfileViewItem *pitem = (CProfileViewItem *) item;
// 		QString sitem = "<TR valign=\"top\">";
// 		for (int i=0; i<cols; i++)
// 			if (i==0)
// 				sitem += "<TD align=\"left\">" + pitem->text(i) + "</TD>";
// 			else
// 				sitem += "<TD align=\"right\">" + pitem->text(i) + "</TD>";
// 		sitem += "</TR>";
// 		s += sitem;
// 		item = item->nextSibling();
// 	}
// 	s += "</TBODY></TABLE></BODY></HTML>";
// 
// 	KHTMLPart *part = new KHTMLPart ();
// 	part->begin();
// 	part->write(s);
// 	part->end();
// 	part->view()->print();
// 	delete part;
//}

//void KProfWidget::generateCallGraph ()
//{
	// Display the call-graph format selection dialog and generate a
	// call graph
// 	CCallGraph dialog (this, "Call-Graph Format", true);
// 	if (dialog.exec ())
// 	{
// 		bool currentSelectionOnly = dialog.mSelectedFunction->isChecked ();
// 
// 		QTreeWidgetItem *selectedItem = NULL;
// 		if (currentSelectionOnly)
// 		{
// 			selectedItem = (mCurPage == 0 ? mFlat->selectedItem() :
// 							mCurPage == 1 ? mHier->selectedItem() :
// 							mObjs->selectedItem());
// 			if (selectedItem == NULL)
// 			{
// 				KMessageBox::sorry (this, i18n ("To export the current selection's call-graph,\nyou must select an item in the profile view."), i18n ("Selection Empty"));
// 				return;
// 			}
// 		}
// 
// 	if (dialog.mSaveFile->isChecked() )
// 	{
// 		QString dotfile = KFileDialog::getSaveFileName (
// 			QString::null,
// 			dialog.mGraphViz->isChecked () ? i18n("*.dot|GraphViz files") : i18n("*.vcg|VCG files"),
// 			this,
// 			i18n ("Save call-graph as..."));
// 	
// 			if (dotfile.isEmpty ())
// 			{
// 				return;
// 			}
// 
// 			QFile file (dotfile);
// 			if (!file.open (IO_WriteOnly | IO_Truncate | IO_Translate))
// 			{
// 				KMessageBox::error (this, i18n ("File could not be opened for writing."), i18n ("File Error"));
// 				return;
// 			}
// 
// 			if (currentSelectionOnly)
// 			{
// 				for (uint i=0; i < mProfile.count(); i++)
// 					mProfile[i]->output = false;
// 
// 				ProfileInfo *info = ((CProfileViewItem *) selectedItem)->getProfile ();
// 				if (info == NULL)
// 				{	
// 					// probably a parent item in the object profile view;
// 					// in this case, mark all objects of the same class for output
// 					QTreeWidgetItem *childItem = selectedItem->firstChild ();
// 					if (childItem)
// 						info = ((CProfileViewItem *) childItem)->getProfile ();
// 
// 					if (info == NULL)
// 					{
// 						KMessageBox::error (this, i18n ("Internal Error"), i18n ("Could not find any function or class to export."));
// 						return;
// 					}
// 
// 					QString className = info->object;
// 					for (uint i = 0; i < mProfile.count(); i++)
// 					{
// 						if (mProfile[i]->output==false && mProfile[i]->object==info->object)
// 							markForOutput (mProfile[i]);
// 					}
// 				}
// 				else
// 					markForOutput (info);
// 			}
// 
// 			// graph generation
// 			if (dialog.mGraphViz->isChecked ())
// 				DotCallGraph dotCallGraph(file, currentSelectionOnly, false, mProfile, processName, mConfigure->highColour());
// 			else
// 				VCGCallGraph vcgCallGraph(file, currentSelectionOnly, mProfile);
// 
// 			file.close ();
// 		}
// 		else
// 		{
// 			//Generate a temporary file
// 			QFile file;
// 
// 			if (dialog.mGraphViz->isChecked ())
// 			{
// 				file.setName(".graphViz_temp");
// 			}
// 			else
// 			{
// 				file.setName(".vcg_temp");
// 			}
// 
// 			file.open(IO_ReadWrite);
// 			if (!file.exists())
// 			{
// 				KMessageBox::error (this, i18n ("Internal Error"), i18n ("Could not open a temporary file for writing."));
// 				return;
// 			}
// 
// 			// graph generation
// 			if (dialog.mGraphViz->isChecked ())
// 			{
// 				DotCallGraph dotCallGraph(file, currentSelectionOnly, true, mProfile, processName, mConfigure->highColour());
// 			}
// 			else
// 			{
// 				VCGCallGraph vcgCallGraph(file, currentSelectionOnly, mProfile);
// 			}
// 
// 			file.close ();
// 
// 			KProcess graphApplication;
// 			KProcess displayApplication;
// 
// 			QString graphApplicationName = "";
// 
// 			if (dialog.mGraphViz->isChecked ())
// 			{
// 				graphApplicationName = "dot";
// 				graphApplication << "dot" << file.name() << "-Timap" ;
// 				displayApplication << "dot" << file.name() << "-Tjpg" << "-o" << ".graphViz.jpg";
// 
// 				connect (&graphApplication, SIGNAL (receivedStdout (KProcess*, char*, int)), this, SLOT (graphVizStdout (KProcess*, char*, int)));
// 				connect (&graphApplication, SIGNAL (receivedStderr (KProcess*, char*, int)), this, SLOT (graphVizStderr (KProcess*, char*, int)));
// 
// 				graphApplication.start(KProcess::Block, KProcess::AllOutput);
// 				displayApplication.start();
// 
// 				QFile mapFile;
// 				QTextStream text (&mGraphVizStdout, IO_ReadOnly);
// 				
// 				mapFile.setName("./.kprof.html");
// 				mapFile.open(IO_ReadWrite);
// 				ClientSideMap(text, mapFile, processName);
// 			}
// 			else
// 			{
// 				graphApplicationName = "xvcg";
// 				graphApplication << "xvcg" << file.name() ;
// 				graphApplication.start();
// 			}
// 		
// 			if (!graphApplication.normalExit () || graphApplication.exitStatus ())
// 			{
// 				QString text = graphApplicationName + i18n (" could not display the data.\n");
// 				if (graphApplication.normalExit () && graphApplication.exitStatus ())
// 				{
// 					QString s;
// 					s.sprintf (i18n ("Error %d was returned.\n"), graphApplication.exitStatus ());
// 					text += s;
// 				}
// 				KMessageBox::error (this, text, i18n ("Exited with error(s)"));
// 				return;
// 			}
// 		}
// 	}
// }

// void KProfWidget::markForOutput(ProfileInfo * p)
// {
//   // if true, we already passed this item; avoid entering a recursive loop
//   if(p->output)
//     return;
// 
//   p->output = true;
//   for(int i = 0; i < p->called.count(); i++)
//     markForOutput(p->called[i]);
// }


//This method takes the call graph and turns it into a client side image map
//in the html viewer
// void KProfWidget::prepareHtmlPart(KHTMLPart* part)
// {
	//Generate a temporary file
	//QFile file;
// 	KProfFile* file = new KProfFile();
// 
// 	file->setName(processName + "graphViz_temp");
// 	if (file->exists())
// 	{
// 		file->remove();
// 	}
// 
// 	file->open(IO_ReadWrite);
// 
// 	DotCallGraph dotCallGraph(*file, true, true, mProfile, processName, mConfigure->highColour());
// 	file->close ();
// 
// 	KProcess graphApplication;
// 	KProcess displayApplication;
// 
// 	graphApplication << "dot" << file->name() << "-Timap" ;
// 	QString fileName(processName + "graphViz.jpg");
// 	displayApplication << "dot" << file->name() << "-Tjpg" << "-o" << fileName;
// 
// 	connect (&graphApplication, SIGNAL (receivedStdout (KProcess*, char*, int)), this, SLOT (graphVizStdout (KProcess*, char*, int)));
// 	connect (&graphApplication, SIGNAL (receivedStderr (KProcess*, char*, int)), this, SLOT (graphVizStderr (KProcess*, char*, int)));
// 
// 	connect (&displayApplication, SIGNAL (receivedStdout (KProcess*, char*, int)), this, SLOT (graphVizDispStdout (KProcess*, char*, int)));
// 	connect (&displayApplication, SIGNAL (receivedStderr (KProcess*, char*, int)), this, SLOT (graphVizDispStderr (KProcess*, char*, int)));
// 
// 	graphApplication.start(KProcess::Block, KProcess::AllOutput);
// 	displayApplication.start(KProcess::Block, KProcess::AllOutput);
// 
// 	if (!graphApplication.normalExit () || graphApplication.exitStatus ())
// 	{
// 		QString text = i18n ("dot could not display the data (1).\n");
// 		if (graphApplication.normalExit () && graphApplication.exitStatus ())
// 		{
// 			QString s;
// 			s.sprintf (i18n ("Error %d was returned.\n"), graphApplication.exitStatus ());
// 			text += s;
// 			text += mGraphVizDispStderr;
// 		}
// 		KMessageBox::error (this, text, i18n ("Exited with error(s)"));
// 		return;
// 	}
// 	if (!displayApplication.normalExit () || displayApplication.exitStatus ())
// 	{
// 		QString text = i18n (" dot could not display the data (2).\n");
// 		if (displayApplication.normalExit () && displayApplication.exitStatus ())
// 		{
// 			QString s;
// 			s.sprintf (i18n ("Error %d was returned.\n"), displayApplication.exitStatus ());
// 			text += s;
// 		}
// 		KMessageBox::error (this, text, i18n ("Exited with error(s)"));
// 		return;
// 	}
// 	file->close();
// 	//delete file;
// 	file = 0;
// 
// 	QFile mapFile;
// 	QTextStream text (&mGraphVizStdout, IO_ReadOnly);
// 	mapFile.setName(processName + "kprof.html");
// 
// 	if(mapFile.exists())
// 	{
// 		mapFile.remove();
// 	}
// 
// 	mapFile.open(IO_ReadWrite);
// 	ClientSideMap(text, mapFile, processName);
// 
// 	mapFile.close();
// 	part->openURL(KUrl("file://" + processName + "kprof.html"));
// }
