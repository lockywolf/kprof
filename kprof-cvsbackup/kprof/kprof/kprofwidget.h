/*
 * kprofwidget.h
 *
 * $Id$
 *
 * Copyright (c) 2000 Florent Pillet <florent.pillet@wanadoo.fr>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.trolltech.com/
 *
 * Requires the K Desktop Environment 2.0 (KDE 2.0) libraries, available
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

#ifndef __KPROFWIDGET_H__
#define __KPROFWIDGET_H__

#include <qvector.h>
#include <qglobal.h>
#include <qstring.h>
#include <qlineedit.h>
#include <qprinter.h>
#include <qtextstream.h>

#include <klistview.h>
#include <ktabctl.h>
#include <kurl.h>
#include <kprocess.h>

#include "cprofileinfo.h"

class CProfileViewItem;

class KProfWidget : public QWidget
{
	Q_OBJECT

protected:
	KTabCtl*				mTabs;		// the tabbed control
	KListView*				mFlat;		// the flat profile list widget
	KListView*				mHier;		// the hierarchical profile widget
	KListView*				mObjs;		// the object profile widget
	QVector<CProfileInfo>	mProfile;	// profile information read from file
#ifndef QT_NO_PRINTER
	QPrinter				mPrinter;	// printer object
#endif
	int						mCurPage;	// id of the current page (used to know which to print)

	QString					mGProfStdout;	// stdout from gprof command
	QString					mGProfStderr;	// stderr from gprof command

	// this structure is used while parsing the call graph
	typedef struct
	{
		QString	name;
		long	line;
		bool	primary;
		bool	recursive;
	} SCallGraphEntry;

	enum								// states while parsing the gprof output
	{
		SEARCH_FLAT_PROFILE,
		PROCESS_FLAT_PROFILE,
		SEARCH_CALL_GRAPH,
		PROCESS_CALL_GRAPH
	};

public:
	enum colID
	{
		col_function = 0,
		col_recursive,
		col_count,
		col_total,
		col_totalPercent,
		col_self,
		col_totalPerCall,
		col_selfPerCall
	};

public:
	KProfWidget (QWidget *parent=NULL, const char *name=NULL);
	~KProfWidget ();

public slots:
	void tabSelected (int page);

	void settingsChanged ();
	void loadSettings ();
	void applySettings ();

	void openResultsFile ();
	void openRecentFile (const KURL& url);
	void doPrint ();

	void profileEntryRightClick (QListViewItem *listItem, const QPoint &p, int);
	void flatProfileFilterChanged (const QString &filter);
	void generateCallGraph ();

protected slots:
	void selectionChanged (QListViewItem *item);
	void gprofStdout (KProcess*, char *buffer, int buflen);
	void gprofStderr (KProcess*, char *buffer, int buflen);

signals:
	void addRecentFile (const KURL&);

private:
	void openFile (const QString &filename);
	void prepareProfileView (KListView *view, bool rootIsDecorated);
	void parseProfile (QTextStream &t);
	void processCallGraphBlock (const QVector<SCallGraphEntry> &data);

	CProfileInfo *locateProfileEntry (const QString& name);

	void fillFlatProfileList (const QString& filter);
	void fillHierProfileList ();
	void fillHierarchy (CProfileViewItem *item, CProfileInfo *parent, QArray<CProfileInfo *> &addedEntries, int &count);
	void fillObjsProfileList ();
	void fillObjsHierarchy (CProfileViewItem *parent, QString *className);

	void selectProfileItem (CProfileInfo *info);
	void selectItemInView (QListView *view, CProfileInfo *info, bool examineSubs);

	void generateDotCallGraph (bool currentSelectionOnly);
	void generateVCGCallGraph (bool currentSelectionOnly);
};

#endif
