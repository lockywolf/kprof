/*
 * kprofwidget.h
 *
 * $Id$
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

#ifndef __KPROFWIDGET_H__
#define __KPROFWIDGET_H__

#include <qvector.h>
#include <qglobal.h>
#include <qstring.h>
#include <qlineedit.h>
#include <qprinter.h>
#include <qtextstream.h>
#include <qdir.h>

#include <klistview.h>
#include <ktabctl.h>
#include <kurl.h>
#include <kprocess.h>

#include "cprofileinfo.h"

class QFont;
class QFile;
class CProfileViewItem;

class KProfWidget : public QWidget
{
	Q_OBJECT

public:
	static QFont*			sListFont;	// font used to draw list entries
	static int				sLastFileFormat;	// format of the last opened file
	static bool				sDiffMode;	// true if performing a diff. Used by CProfileViewItem

protected:
	KTabCtl*				mTabs;		// the tabbed control
	KListView*				mFlat;		// the flat profile list widget
	KListView*				mHier;		// the hierarchical profile widget
	KListView*				mObjs;		// the object profile widget

	QVector<CProfileInfo>	mProfile;	// profile information read from file
	QVector<CProfileInfo>	mPreviousProfile;	// when comparing, keep previous profile information here
	QVector<QString>		mClasses;	// list of distinct class names found in the profile information

	int						mCurPage;		// id of the current page (used to know which to print)
	QString					mGProfStdout;	// stdout from gprof command
	QString					mGProfStderr;	// stderr from gprof command
	QString					mFlatFilter;	// filter string for flat profile view
	QFont					mListFont;		// font used to draw the text
	bool					mAbbrevTemplates; // if true, templates are "abbreviates" (i.e. become <...>)
	QDir					mCurDir;		// current directory

	typedef struct						// structure is used while parsing the call graph for gprof and Function Check
	{
		QString	name;
		long	line;
		bool	primary;
		bool	recursive;
	} SCallGraphEntry;

	typedef struct					 	// structure holding call-graph data for PalmOS Emulator results
	{
		int		index;
		int		parent;
	} SPoseCallGraph;

	enum								// states while parsing the gprof output
	{
		ANALYZING,
		PROCESS_CYCLES,					// used for Function Check parsing
		SEARCH_FLAT_PROFILE,			// for GPROF (to remove later)
		PROCESS_FLAT_PROFILE,
		PROCESS_MIN_MAX_TIME,			// used for Function Check parsing
		SEARCH_CALL_GRAPH,				// for GPROF (to remove later)
		PROCESS_CALL_GRAPH,
		DISCARD_CALL_GRAPH_ENTRY
	};


public:
	enum								// text results file format that we support
	{
		FORMAT_GPROF,					// GNU gprof
		FORMAT_FNCCHECK,				// Function Check
		FORMAT_POSE						// PalmOS Emulator
	};

	enum colID							// column IDs
	{
		col_function = 0,
		col_recursive,
		col_count,
		col_total,
		col_totalPercent,
		col_self,
		col_totalMsPerCall,				// last column common to all formats

		// gprof specific columns
		col_selfMsPerCall = col_totalMsPerCall + 1,

		// Function Check specific columns
		col_minMsPerCall = col_totalMsPerCall + 1,
		col_maxMsPerCall,

		// POSE specific columns
		col_selfCycles = col_totalMsPerCall + 1,
		col_cumCycles
	};
	
	enum diffColID						// diff mode column IDs
	{
		diff_col_function = 0,
		diff_col_recursive,
		diff_col_status,
		diff_col_count,
		diff_col_new_count,
		diff_col_total,
		diff_col_new_total,
		diff_col_totalPercent,
		diff_col_new_totalPercent,
		diff_col_self,
		diff_col_new_self,
		diff_col_totalMsPerCall,
		diff_col_new_totalMsPerCall,	// last column common to all formats

		// gprof specific columns
		diff_col_selfMsPerCall,
		diff_col_new_selfMsPerCall,

		// Function Check specific columns
		diff_col_minMsPerCall = diff_col_new_totalMsPerCall + 1,
		diff_col_new_minMsPerCall,
		diff_col_maxMsPerCall,
		diff_col_new_maxMsPerCall,

		// POSE specific columns
		diff_col_selfCycles = diff_col_new_totalMsPerCall + 1,
		diff_col_new_selfCycles,
		diff_col_cumCycles,
		diff_col_new_cumCycles
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
	void compareFile ();
	void openRecentFile (const KURL& url);
	void doPrint ();

	void profileEntryRightClick (QListViewItem *listItem, const QPoint &p, int);
	void flatProfileFilterChanged (const QString &filter);
	void generateCallGraph ();

	void toggleTemplateAbbrev ();
	void selectListFont ();

protected slots:
	void selectionChanged (QListViewItem *item);
	void gprofStdout (KProcess*, char *buffer, int buflen);
	void gprofStderr (KProcess*, char *buffer, int buflen);

signals:
	void addRecentFile (const KURL&);

private:
	void openFile (const QString &filename, int format, bool compare = false);
	void prepareProfileView (KListView *view, bool rootIsDecorated);
	void parseProfile_pose (QTextStream &t);
	bool parseProfile_fnccheck (QTextStream &t);
	void parseProfile_gprof (QTextStream &t);
	void processCallGraphBlock (const QVector<SCallGraphEntry> &data);
	void postProcessProfile (bool compare);

	void customizeColumns (KListView *view, int profiler);
	
	CProfileInfo *locateProfileEntry (const QString& name);

	void fillFlatProfileList ();
	void fillHierProfileList ();
	void fillHierarchy (CProfileViewItem *item, CProfileInfo *parent, QArray<CProfileInfo *> &addedEntries, int &count);
	void fillObjsProfileList ();

	void selectProfileItem (CProfileInfo *info);
	void selectItemInView (QListView *view, CProfileInfo *info, bool examineSubs);

	void markForOutput (CProfileInfo *info);
	void generateDotCallGraph (QFile& file, bool currentSelectionOnly);
	void generateVCGCallGraph (QFile& file, bool currentSelectionOnly);

	QString getClassName (const QString& name);
	QString removeTemplates (const QString& name);
};

#endif
